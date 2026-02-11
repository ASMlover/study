import fs from "node:fs";
import os from "node:os";
import path from "node:path";

export type MiniCliConfig = Record<string, unknown>;
export type ModelName = "glm-4" | "glm-4-air" | "mock-mini";

export interface RuntimeConfig {
  model: ModelName;
  timeoutMs: number;
}

export type ConfigIssueCode =
  | "missing_required"
  | "type_mismatch"
  | "invalid_enum"
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

const ALLOWED_MODELS: readonly ModelName[] = ["glm-4", "glm-4-air", "mock-mini"];
const ALLOWED_KEYS = new Set(["model", "timeoutMs"]);
const DEFAULT_RUNTIME_CONFIG: RuntimeConfig = {
  model: "glm-4",
  timeoutMs: 30000
};

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
    if (!ALLOWED_KEYS.has(key)) {
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
  } else if (!ALLOWED_MODELS.includes(rawModel as ModelName)) {
    issues.push({
      code: "invalid_enum",
      path: "model",
      message: `Field "model" must be one of: ${ALLOWED_MODELS.join(", ")}.`
    });
  } else {
    normalized.model = rawModel as ModelName;
  }

  const rawTimeout = config.timeoutMs;
  if (rawTimeout === undefined) {
    return { config: normalized, issues };
  }

  if (typeof rawTimeout !== "number" || Number.isNaN(rawTimeout)) {
    issues.push({
      code: "type_mismatch",
      path: "timeoutMs",
      message: `Field "timeoutMs" must be number, got ${typeof rawTimeout}.`
    });
    return { config: normalized, issues };
  }

  normalized.timeoutMs = rawTimeout;
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
