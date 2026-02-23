import fs from "node:fs";
import os from "node:os";
import path from "node:path";

export type MiniCliConfig = Record<string, unknown>;
export type ModelName = string;

export interface RuntimeConfig {
  model: ModelName;
  timeoutMs: number;
  apiKey?: string;
  runConfirmationTimeoutMs?: number;
  requestTokenBudget?: number;
}

export type ConfigIssueCode =
  | "missing_required"
  | "type_mismatch"
  | "unknown_field"
  | "parse_error";

export interface ConfigIssue {
  code: ConfigIssueCode;
  path: string;
  message: string;
}

export interface LoadedConfig {
  globalPath: string;
  projectPath: string;
  globalConfig: MiniCliConfig;
  projectConfig: MiniCliConfig;
  mergedConfig: MiniCliConfig;
}

export interface LoadedRuntimeConfig {
  globalPath: string;
  projectPath: string;
  config: RuntimeConfig;
  issues: ConfigIssue[];
}

type ReadFileSync = typeof fs.readFileSync;
type WriteFileSync = typeof fs.writeFileSync;
type MkdirSync = typeof fs.mkdirSync;

export function resolveGlobalConfigPath(homeDir: string): string {
  return path.join(homeDir, ".minicli", "config.json");
}

export function resolveProjectConfigPath(cwd: string): string {
  return path.join(cwd, ".minicli", "config.json");
}

function stripJsonComments(content: string): string {
  let output = "";
  let inString = false;
  let escaping = false;
  let inLineComment = false;
  let inBlockComment = false;

  for (let i = 0; i < content.length; i += 1) {
    const current = content[i];
    const next = content[i + 1];
    const previous = i > 0 ? content[i - 1] : "\n";

    if (inLineComment) {
      if (current === "\n") {
        inLineComment = false;
        output += current;
      }
      continue;
    }

    if (inBlockComment) {
      if (current === "*" && next === "/") {
        inBlockComment = false;
        i += 1;
      }
      continue;
    }

    if (inString) {
      output += current;
      if (escaping) {
        escaping = false;
      } else if (current === "\\") {
        escaping = true;
      } else if (current === "\"") {
        inString = false;
      }
      continue;
    }

    if (current === "\"") {
      inString = true;
      output += current;
      continue;
    }

    if (current === "/" && next === "/") {
      inLineComment = true;
      i += 1;
      continue;
    }

    if (current === "/" && next === "*") {
      inBlockComment = true;
      i += 1;
      continue;
    }

    if (current === "#") {
      const atLineStart = previous === "\n" || previous === "\r";
      if (atLineStart) {
        inLineComment = true;
        continue;
      }
    }

    output += current;
  }

  return output;
}

export function parseConfig(content: string): MiniCliConfig {
  const stripped = stripJsonComments(content).trim();
  if (stripped.length === 0) {
    return {};
  }
  return JSON.parse(stripped) as MiniCliConfig;
}

export function loadConfigFile(
  filePath: string,
  readFileSync: ReadFileSync = fs.readFileSync
): MiniCliConfig {
  try {
    const content = readFileSync(filePath, "utf8");
    return parseConfig(content);
  } catch (error) {
    const e = error as NodeJS.ErrnoException;
    if (e.code === "ENOENT") {
      return {};
    }
    throw error;
  }
}

export function mergeConfig(
  globalConfig: MiniCliConfig,
  projectConfig: MiniCliConfig
): MiniCliConfig {
  return {
    ...globalConfig,
    ...projectConfig
  };
}

export function loadMergedConfig(params?: {
  cwd?: string;
  homeDir?: string;
  readFileSync?: ReadFileSync;
}): LoadedConfig {
  const cwd = params?.cwd ?? process.cwd();
  const homeDir = params?.homeDir ?? os.homedir();
  const readFileSync = params?.readFileSync ?? fs.readFileSync;
  const globalPath = resolveGlobalConfigPath(homeDir);
  const projectPath = resolveProjectConfigPath(cwd);
  const globalConfig = loadConfigFile(globalPath, readFileSync);
  const projectConfig = loadConfigFile(projectPath, readFileSync);

  return {
    globalPath,
    projectPath,
    globalConfig,
    projectConfig,
    mergedConfig: mergeConfig(globalConfig, projectConfig)
  };
}

export type ManagedConfigKey =
  | "model"
  | "timeoutMs"
  | "apiKey"
  | "runConfirmationTimeoutMs"
  | "requestTokenBudget";

export type MutableManagedConfigKey = Exclude<ManagedConfigKey, "apiKey">;

const ALLOWED_KEYS = new Set<ManagedConfigKey>([
  "model",
  "timeoutMs",
  "apiKey",
  "runConfirmationTimeoutMs",
  "requestTokenBudget"
]);
const DEFAULT_RUNTIME_CONFIG: RuntimeConfig = {
  model: "glm-4",
  timeoutMs: 30000
};

export function isSupportedModelName(model: string): model is ModelName {
  return model.trim().length > 0;
}

export function maskSecret(secret: string): string {
  if (secret.length <= 4) {
    return "*".repeat(secret.length);
  }
  return `${"*".repeat(secret.length - 4)}${secret.slice(-4)}`;
}

export function validateAndNormalizeConfig(
  config: MiniCliConfig,
  options?: { requireModel?: boolean }
): {
  config: RuntimeConfig;
  issues: ConfigIssue[];
} {
  const issues: ConfigIssue[] = [];
  const normalized: RuntimeConfig = { ...DEFAULT_RUNTIME_CONFIG };
  const requireModel = options?.requireModel ?? true;

  for (const key of Object.keys(config)) {
    if (!ALLOWED_KEYS.has(key as ManagedConfigKey)) {
      issues.push({
        code: "unknown_field",
        path: key,
        message: `Unknown field "${key}" will be ignored.`
      });
    }
  }

  const rawModel = config.model;
  if (rawModel === undefined) {
    if (requireModel) {
      issues.push({
        code: "missing_required",
        path: "model",
        message: "Required field \"model\" is missing; default will be used."
      });
    }
  } else if (typeof rawModel !== "string") {
    issues.push({
      code: "type_mismatch",
      path: "model",
      message: `Field "model" must be string, got ${typeof rawModel}.`
    });
  } else if (rawModel.trim().length === 0) {
    issues.push({
      code: "type_mismatch",
      path: "model",
      message: "Field \"model\" cannot be empty."
    });
  } else {
    normalized.model = rawModel.trim();
  }

  const rawTimeout = config.timeoutMs;
  if (rawTimeout !== undefined) {
    if (typeof rawTimeout !== "number" || Number.isNaN(rawTimeout)) {
      issues.push({
        code: "type_mismatch",
        path: "timeoutMs",
        message: `Field "timeoutMs" must be number, got ${typeof rawTimeout}.`
      });
    } else {
      normalized.timeoutMs = rawTimeout;
    }
  }

  const rawApiKey = config.apiKey;
  if (rawApiKey !== undefined) {
    if (typeof rawApiKey !== "string") {
      issues.push({
        code: "type_mismatch",
        path: "apiKey",
        message: `Field "apiKey" must be string, got ${typeof rawApiKey}.`
      });
    } else if (rawApiKey.trim().length === 0) {
      issues.push({
        code: "type_mismatch",
        path: "apiKey",
        message: "Field \"apiKey\" cannot be empty."
      });
    } else {
      normalized.apiKey = rawApiKey;
    }
  }

  const rawRunConfirmationTimeout = config.runConfirmationTimeoutMs;
  if (rawRunConfirmationTimeout !== undefined) {
    if (
      typeof rawRunConfirmationTimeout !== "number" ||
      Number.isNaN(rawRunConfirmationTimeout) ||
      rawRunConfirmationTimeout <= 0
    ) {
      issues.push({
        code: "type_mismatch",
        path: "runConfirmationTimeoutMs",
        message:
          'Field "runConfirmationTimeoutMs" must be a positive number.'
      });
    } else {
      normalized.runConfirmationTimeoutMs = rawRunConfirmationTimeout;
    }
  }

  const rawRequestTokenBudget = config.requestTokenBudget;
  if (rawRequestTokenBudget !== undefined) {
    if (
      typeof rawRequestTokenBudget !== "number" ||
      Number.isNaN(rawRequestTokenBudget) ||
      rawRequestTokenBudget <= 0
    ) {
      issues.push({
        code: "type_mismatch",
        path: "requestTokenBudget",
        message: 'Field "requestTokenBudget" must be a positive number.'
      });
    } else {
      normalized.requestTokenBudget = rawRequestTokenBudget;
    }
  }

  return { config: normalized, issues };
}

export function formatConfigIssue(issue: ConfigIssue): string {
  return `[config:${issue.code}] ${issue.path}: ${issue.message}`;
}

export function loadRuntimeConfig(params?: {
  cwd?: string;
  homeDir?: string;
  readFileSync?: ReadFileSync;
}): LoadedRuntimeConfig {
  const cwd = params?.cwd ?? process.cwd();
  const homeDir = params?.homeDir ?? os.homedir();
  const readFileSync = params?.readFileSync ?? fs.readFileSync;
  const globalPath = resolveGlobalConfigPath(homeDir);
  const projectPath = resolveProjectConfigPath(cwd);
  const issues: ConfigIssue[] = [];

  const readFileOrEmpty = (filePath: string): MiniCliConfig => {
    try {
      const content = readFileSync(filePath, "utf8");
      return parseConfig(content);
    } catch (error) {
      const e = error as NodeJS.ErrnoException;
      if (e.code === "ENOENT") {
        return {};
      }
      issues.push({
        code: "parse_error",
        path: filePath,
        message: e.message
      });
      return {};
    }
  };

  const globalConfig = readFileOrEmpty(globalPath);
  const projectConfig = readFileOrEmpty(projectPath);
  const merged = mergeConfig(globalConfig, projectConfig);
  const validated = validateAndNormalizeConfig(merged, {
    requireModel: Object.keys(merged).length > 0
  });

  return {
    globalPath,
    projectPath,
    config: validated.config,
    issues: [...issues, ...validated.issues]
  };
}

interface WriteConfigOptions {
  writeFileSync?: WriteFileSync;
  mkdirSync?: MkdirSync;
  readFileSync?: ReadFileSync;
}

export function writeConfigFile(
  filePath: string,
  config: MiniCliConfig,
  options?: WriteConfigOptions
): void {
  const writeFileSync = options?.writeFileSync ?? fs.writeFileSync;
  const mkdirSync = options?.mkdirSync ?? fs.mkdirSync;
  mkdirSync(path.dirname(filePath), { recursive: true });
  writeFileSync(filePath, `${JSON.stringify(config, null, 2)}\n`, "utf8");
}

export function saveApiKeyToGlobalConfig(
  globalConfigPath: string,
  apiKey: string,
  options?: WriteConfigOptions
): void {
  const normalized = apiKey.trim();
  if (normalized.length === 0) {
    throw new Error("API key cannot be empty.");
  }

  const readFileSync = options?.readFileSync ?? fs.readFileSync;
  const current = loadConfigFile(globalConfigPath, readFileSync);
  writeConfigFile(
    globalConfigPath,
    {
      ...current,
      apiKey: normalized
    },
    options
  );
}

export function saveModelToProjectConfig(
  projectConfigPath: string,
  model: string,
  options?: WriteConfigOptions
): ModelName {
  const normalized = model.trim();
  if (!isSupportedModelName(normalized)) {
    throw new Error("Model name cannot be empty.");
  }

  const readFileSync = options?.readFileSync ?? fs.readFileSync;
  const current = loadConfigFile(projectConfigPath, readFileSync);
  writeConfigFile(
    projectConfigPath,
    {
      ...current,
      model: normalized
    },
    options
  );

  return normalized;
}

export function saveManagedValueToProjectConfig(
  projectConfigPath: string,
  key: MutableManagedConfigKey,
  value: string | number,
  options?: WriteConfigOptions
): void {
  const readFileSync = options?.readFileSync ?? fs.readFileSync;
  const current = loadConfigFile(projectConfigPath, readFileSync);
  writeConfigFile(
    projectConfigPath,
    {
      ...current,
      [key]: value
    },
    options
  );
}

export function resetManagedValueInProjectConfig(
  projectConfigPath: string,
  key: MutableManagedConfigKey,
  options?: WriteConfigOptions
): void {
  const readFileSync = options?.readFileSync ?? fs.readFileSync;
  const current = loadConfigFile(projectConfigPath, readFileSync);
  if (!(key in current)) {
    return;
  }
  const next: MiniCliConfig = { ...current };
  delete next[key];
  writeConfigFile(projectConfigPath, next, options);
}
