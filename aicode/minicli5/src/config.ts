import { readFileSync, writeFileSync, mkdirSync, existsSync } from "node:fs";
import { join } from "node:path";
import type { AppConfig } from "./types.js";

const DEFAULT_CONFIG: AppConfig = {
  provider: "glm",
  model: "glm-5",
  api_key: "",
  api_base: "https://open.bigmodel.cn/api/paas/v4",
  max_tokens: 4096,
  temperature: 0.7,
  max_retries: 3,
  stream: true,
  max_tool_rounds: 10,
  context_max_tokens: 128000,
  compact_threshold: 0.85,
};

function getConfigDir(projectRoot: string): string {
  return join(projectRoot, ".minicli5");
}

function getConfigPath(projectRoot: string): string {
  return join(getConfigDir(projectRoot), "config.json");
}

export function ensureConfigDir(projectRoot: string): void {
  const dir = getConfigDir(projectRoot);
  if (!existsSync(dir)) {
    mkdirSync(dir, { recursive: true });
  }
  const sessionsDir = join(dir, "sessions");
  if (!existsSync(sessionsDir)) {
    mkdirSync(sessionsDir, { recursive: true });
  }
}

export function loadConfig(projectRoot: string): AppConfig {
  const configPath = getConfigPath(projectRoot);

  if (!existsSync(configPath)) {
    ensureConfigDir(projectRoot);
    writeFileSync(configPath, JSON.stringify(DEFAULT_CONFIG, null, 2), "utf-8");
    return { ...DEFAULT_CONFIG };
  }

  const raw = readFileSync(configPath, "utf-8");
  const parsed = JSON.parse(raw) as Partial<AppConfig>;
  return { ...DEFAULT_CONFIG, ...parsed };
}

export function saveConfig(projectRoot: string, config: AppConfig): void {
  ensureConfigDir(projectRoot);
  const configPath = getConfigPath(projectRoot);
  writeFileSync(configPath, JSON.stringify(config, null, 2), "utf-8");
}
