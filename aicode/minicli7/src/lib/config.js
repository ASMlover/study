import fs from "node:fs/promises";
import path from "node:path";

const DEFAULT_CONFIG = {
  model: "glm-5",
  apiBaseUrl: "https://open.bigmodel.cn/api/paas/v4/chat/completions",
  apiKeyEnv: "GLM_API_KEY",
  maxContextTokens: 12000,
  compactKeepRecentMessages: 12,
  autoCompact: true,
  permission: {
    allowReadWithoutPrompt: false,
  },
};

export async function loadConfig(cwd) {
  const configDir = path.join(cwd, ".minicli");
  const configPath = path.join(configDir, "config.json");

  let fileConfig = {};
  const warnings = [];
  try {
    const raw = await fs.readFile(configPath, "utf8");
    const clean = raw.replace(/^\uFEFF/, "");
    fileConfig = JSON.parse(clean);
  } catch (err) {
    // Use defaults if local config is absent or invalid.
    warnings.push(`Failed to parse config: ${err.message}`);
  }

  const config = {
    ...DEFAULT_CONFIG,
    ...fileConfig,
    permission: {
      ...DEFAULT_CONFIG.permission,
      ...(fileConfig.permission ?? {}),
    },
  };

  const isValidEnvName = (name) => /^[A-Za-z_][A-Za-z0-9_]*$/.test(name || "");
  const envApiKey = process.env[config.apiKeyEnv] || process.env.GLM_API_KEY || "";
  const fileApiKey = typeof fileConfig.apiKey === "string" ? fileConfig.apiKey.trim() : "";
  const apiKeyEnvLooksLikeRawKey =
    typeof fileConfig.apiKeyEnv === "string" &&
    fileConfig.apiKeyEnv.length > 0 &&
    !isValidEnvName(fileConfig.apiKeyEnv);

  if (apiKeyEnvLooksLikeRawKey) {
    warnings.push(
      "config.apiKeyEnv looks like a raw API key. Use config.apiKey for direct key, or set apiKeyEnv to an env var name like GLM_API_KEY."
    );
  }

  config.apiKey = fileApiKey || envApiKey || (apiKeyEnvLooksLikeRawKey ? fileConfig.apiKeyEnv.trim() : "");
  config.warnings = warnings;
  config.configPath = configPath;

  return config;
}

export async function ensureProjectConfigTemplate(cwd) {
  const configDir = path.join(cwd, ".minicli");
  const configPath = path.join(configDir, "config.json");

  await fs.mkdir(configDir, { recursive: true });
  try {
    await fs.access(configPath);
  } catch {
    const template = {
      model: "glm-5",
      apiBaseUrl: "https://open.bigmodel.cn/api/paas/v4/chat/completions",
      apiKeyEnv: "GLM_API_KEY",
      maxContextTokens: 12000,
      compactKeepRecentMessages: 12,
      autoCompact: true,
      permission: {
        allowReadWithoutPrompt: false,
      },
    };
    await fs.writeFile(configPath, JSON.stringify(template, null, 2), "utf8");
  }

  return configPath;
}
