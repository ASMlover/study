import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { parse, stringify } from 'yaml';

export interface MiniCLIConfig {
  provider: {
    type: 'glm5';
    apiKey: string;
    baseUrl: string;
    model: string;
  };
  agent: {
    maxIterations: number;
    timeout: number;
  };
  tools: {
    permissions: Record<string, 'auto' | 'confirm' | 'deny'>;
  };
  context: {
    maxTokens: number;
    compactThreshold: number;
    preserveRecent: number;
  };
  ui: {
    theme: 'dark' | 'light';
    spinner: string;
    showTokens: boolean;
  };
}

const DEFAULT_CONFIG: MiniCLIConfig = {
  provider: {
    type: 'glm5',
    apiKey: '',
    baseUrl: 'https://open.bigmodel.cn/api/paas/v4',
    model: 'glm-4-plus',
  },
  agent: {
    maxIterations: 50,
    timeout: 300000,
  },
  tools: {
    permissions: {
      bash: 'confirm',
      read: 'auto',
      write: 'confirm',
      edit: 'confirm',
      glob: 'auto',
      grep: 'auto',
    },
  },
  context: {
    maxTokens: 128000,
    compactThreshold: 100000,
    preserveRecent: 10,
  },
  ui: {
    theme: 'dark',
    spinner: 'dots',
    showTokens: true,
  },
};

export class ConfigManager {
  private config: MiniCLIConfig;
  private configPath: string | null = null;
  private watchers: Set<() => void> = new Set();

  constructor() {
    this.config = { ...DEFAULT_CONFIG };
    this.deepFreeze(this.config);
  }

  async load(configPath?: string): Promise<MiniCLIConfig> {
    const paths = this.getConfigPaths(configPath);

    for (const p of paths) {
      if (fs.existsSync(p)) {
        this.configPath = p;
        break;
      }
    }

    if (this.configPath && fs.existsSync(this.configPath)) {
      const content = await fs.promises.readFile(this.configPath, 'utf-8');
      const userConfig = this.parseYaml(content);
      this.config = this.mergeConfig(DEFAULT_CONFIG, userConfig);
    } else {
      this.config = { ...DEFAULT_CONFIG };
    }

    this.resolveEnvVarsInConfig();
    return this.config;
  }

  get(): MiniCLIConfig {
    return this.config;
  }

  set(key: string, value: unknown): void {
    const keys = key.split('.');
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let obj: any = this.config;

    for (let i = 0; i < keys.length - 1; i++) {
      if (!obj[keys[i]]) {
        obj[keys[i]] = {};
      }
      obj = obj[keys[i]];
    }

    obj[keys[keys.length - 1]] = value;
    this.notifyWatchers();
  }

  getConfigPath(): string | null {
    return this.configPath;
  }

  onChange(callback: () => void): () => void {
    this.watchers.add(callback);
    return () => this.watchers.delete(callback);
  }

  resolveEnvVars(str: string): string {
    return str.replace(/\$\{([^}]+)\}/g, (_, name) => {
      const value = process.env[name];
      if (value === undefined) {
        return '';
      }
      return value;
    });
  }

  async reload(): Promise<MiniCLIConfig> {
    return this.load(this.configPath || undefined);
  }

  async save(configPath?: string): Promise<void> {
    const targetPath = configPath || this.configPath || this.getDefaultUserConfigPath();
    const dir = path.dirname(targetPath);

    if (!fs.existsSync(dir)) {
      await fs.promises.mkdir(dir, { recursive: true });
    }

    const content = stringify(this.config, { lineWidth: 0 });
    await fs.promises.writeFile(targetPath, content, 'utf-8');

    if (!this.configPath) {
      this.configPath = targetPath;
    }
  }

  private getConfigPaths(customPath?: string): string[] {
    const paths: string[] = [];

    if (customPath) {
      paths.push(customPath);
    }

    paths.push(this.getDefaultUserConfigPath());
    paths.push(path.join(process.cwd(), '.minicli', 'config.yaml'));

    return paths;
  }

  private getDefaultUserConfigPath(): string {
    return path.join(os.homedir(), '.minicli', 'config.yaml');
  }

  private parseYaml(content: string): Partial<MiniCLIConfig> {
    try {
      return parse(content) || {};
    } catch {
      return {};
    }
  }

  private mergeConfig(
    defaultConfig: MiniCLIConfig,
    userConfig: Partial<MiniCLIConfig>
  ): MiniCLIConfig {
    const result = { ...defaultConfig };

    if (userConfig.provider) {
      result.provider = { ...defaultConfig.provider, ...userConfig.provider };
    }

    if (userConfig.agent) {
      result.agent = { ...defaultConfig.agent, ...userConfig.agent };
    }

    if (userConfig.tools) {
      result.tools = {
        permissions: {
          ...defaultConfig.tools.permissions,
          ...(userConfig.tools.permissions || {}),
        },
      };
    }

    if (userConfig.context) {
      result.context = { ...defaultConfig.context, ...userConfig.context };
    }

    if (userConfig.ui) {
      result.ui = { ...defaultConfig.ui, ...userConfig.ui };
    }

    return result;
  }

  private resolveEnvVarsInConfig(): void {
    this.config = JSON.parse(JSON.stringify(this.config));

    if (this.config.provider.apiKey) {
      this.config.provider.apiKey = this.resolveEnvVars(this.config.provider.apiKey);
    }
    if (this.config.provider.baseUrl) {
      this.config.provider.baseUrl = this.resolveEnvVars(this.config.provider.baseUrl);
    }
  }

  private deepFreeze<T>(obj: T): T {
    Object.freeze(obj);
    Object.getOwnPropertyNames(obj).forEach((prop) => {
      const value = (obj as Record<string, unknown>)[prop];
      if (value && typeof value === 'object' && !Object.isFrozen(value)) {
        this.deepFreeze(value);
      }
    });
    return obj;
  }

  private notifyWatchers(): void {
    this.watchers.forEach((callback) => callback());
  }
}

export const configManager = new ConfigManager();
