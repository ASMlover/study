import fs from "node:fs";
import path from "node:path";
import { detectAnsiTheme } from "./ansi";

export type SafeMode = "strict" | "balanced";
export type CompletionMode = "prefix" | "contextual";
export type ThemeMode = "dark" | "light";
export type MotionMode = "full" | "minimal";

export interface RuntimeConfig {
  api_key: string;
  base_url: string;
  model: "glm-5";
  stream: boolean;
  timeout_ms: number;
  max_retries: number;
  temperature: number;
  max_tokens: number;
  agent_max_rounds: number;
  safe_mode: SafeMode;
  theme: ThemeMode;
  motion: MotionMode;
  allowed_paths: string[];
  shell_allowlist: string[];
  slash_completion_mode: CompletionMode;
}

export const DEFAULT_CONFIG: RuntimeConfig = {
  api_key: "",
  base_url: "https://open.bigmodel.cn/api/paas/v4",
  model: "glm-5",
  stream: true,
  timeout_ms: 120_000,
  max_retries: 3,
  temperature: 0.7,
  max_tokens: 4096,
  agent_max_rounds: 6,
  safe_mode: "strict",
  theme: detectAnsiTheme(),
  motion: "full",
  allowed_paths: ["."],
  shell_allowlist: ["pwd", "ls", "dir", "cat", "type", "rg"],
  slash_completion_mode: "contextual"
};

export interface LoadedConfig {
  config: RuntimeConfig;
  configPath: string;
  stateDir: string;
}

export function stateRoot(projectRoot: string): string {
  return path.join(projectRoot, ".minicli4");
}

function configPathFor(projectRoot: string): string {
  return path.join(stateRoot(projectRoot), "config.json");
}

function ensureState(projectRoot: string): void {
  const root = stateRoot(projectRoot);
  fs.mkdirSync(path.join(root, "sessions"), { recursive: true });
  fs.mkdirSync(path.join(root, "logs"), { recursive: true });
  fs.mkdirSync(path.join(root, "cache"), { recursive: true });
}

function coerceConfig(raw: Partial<RuntimeConfig>): RuntimeConfig {
  const apiKeyFromEnv = process.env.GLM_API_KEY;
  const themeFromEnv = detectAnsiTheme();
  const merged: RuntimeConfig = {
    ...DEFAULT_CONFIG,
    ...raw,
    model: "glm-5",
    api_key: apiKeyFromEnv && apiKeyFromEnv.trim().length > 0 ? apiKeyFromEnv : (raw.api_key ?? ""),
    theme: raw.theme === "light" || raw.theme === "dark" ? raw.theme : themeFromEnv,
    motion: raw.motion === "minimal" ? "minimal" : "full"
  };
  return merged;
}

export function loadConfig(projectRoot: string = process.cwd()): LoadedConfig {
  ensureState(projectRoot);
  const configPath = configPathFor(projectRoot);
  if (!fs.existsSync(configPath)) {
    fs.writeFileSync(configPath, JSON.stringify(DEFAULT_CONFIG, null, 2), "utf8");
  }
  let parsed: Partial<RuntimeConfig> = {};
  try {
    parsed = JSON.parse(fs.readFileSync(configPath, "utf8")) as Partial<RuntimeConfig>;
  } catch {
    parsed = {};
  }
  const config = coerceConfig(parsed);
  if (config.model !== "glm-5") {
    config.model = "glm-5";
  }
  return {
    config,
    configPath,
    stateDir: stateRoot(projectRoot)
  };
}

export function saveConfig(projectRoot: string, config: RuntimeConfig): void {
  const normalized = { ...config, model: "glm-5" as const };
  ensureState(projectRoot);
  fs.writeFileSync(configPathFor(projectRoot), JSON.stringify(normalized, null, 2), "utf8");
}
