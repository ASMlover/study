export const CLI_NAME = "minicli";
const DEFAULT_APP_VERSION = "0.1.0";

export function resolveAppVersion(
  env: NodeJS.ProcessEnv = process.env,
  fallbackVersion = DEFAULT_APP_VERSION
): string {
  const injected = env.MINICLI_APP_VERSION;
  if (typeof injected === "string" && injected.trim().length > 0) {
    return injected.trim();
  }
  return fallbackVersion;
}

export const APP_VERSION = resolveAppVersion();

export function formatVersionOutput(): string {
  return `${CLI_NAME} ${APP_VERSION}`;
}
