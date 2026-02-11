import fs from "node:fs";
import os from "node:os";
import path from "node:path";

export type MiniCliConfig = Record<string, unknown>;

export interface LoadedConfig {
  globalPath: string;
  projectPath: string;
  globalConfig: MiniCliConfig;
  projectConfig: MiniCliConfig;
  mergedConfig: MiniCliConfig;
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
