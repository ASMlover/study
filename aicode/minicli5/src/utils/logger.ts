import { appendFileSync, mkdirSync, existsSync } from "node:fs";
import { join } from "node:path";

let logDir: string | null = null;

export function initLogger(projectRoot: string): void {
  logDir = join(projectRoot, ".minicli5", "logs");
  if (!existsSync(logDir)) {
    mkdirSync(logDir, { recursive: true });
  }
}

export function log(level: "info" | "warn" | "error" | "debug", message: string, data?: unknown): void {
  if (!logDir) return;
  const ts = new Date().toISOString();
  const line = JSON.stringify({ ts, level, message, data }) + "\n";
  const file = join(logDir, `${ts.slice(0, 10)}.jsonl`);
  try {
    appendFileSync(file, line, "utf-8");
  } catch {
    // silent
  }
}
