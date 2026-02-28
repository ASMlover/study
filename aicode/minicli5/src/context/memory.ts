import { readFileSync, writeFileSync, existsSync, mkdirSync } from "node:fs";
import { join, dirname } from "node:path";

function memoryPath(projectRoot: string): string {
  return join(projectRoot, ".minicli5", "memory.md");
}

export function loadMemory(projectRoot: string): string {
  const path = memoryPath(projectRoot);
  if (!existsSync(path)) return "";
  try {
    return readFileSync(path, "utf-8");
  } catch {
    return "";
  }
}

export function saveMemory(projectRoot: string, content: string): void {
  const path = memoryPath(projectRoot);
  const dir = dirname(path);
  if (!existsSync(dir)) mkdirSync(dir, { recursive: true });
  writeFileSync(path, content, "utf-8");
}

export function appendMemory(projectRoot: string, entry: string): void {
  const current = loadMemory(projectRoot);
  const timestamp = new Date().toISOString().slice(0, 10);
  const newContent = current
    ? `${current}\n\n## ${timestamp}\n${entry}`
    : `# MiniCLI5 Memory\n\n## ${timestamp}\n${entry}`;
  saveMemory(projectRoot, newContent);
}
