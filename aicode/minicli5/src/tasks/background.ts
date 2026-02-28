import { writeFileSync, readFileSync, existsSync, mkdirSync } from "node:fs";
import { join } from "node:path";
import { execSync } from "node:child_process";
import { log } from "../utils/logger.js";

interface BackgroundTask {
  id: string;
  command: string;
  status: "running" | "completed" | "failed";
  outputFile: string;
  startedAt: number;
  completedAt?: number;
}

const backgroundTasks = new Map<string, BackgroundTask>();

function getTempDir(projectRoot: string): string {
  const dir = join(projectRoot, ".minicli5", "tmp");
  if (!existsSync(dir)) mkdirSync(dir, { recursive: true });
  return dir;
}

export function startBackgroundTask(
  projectRoot: string,
  command: string,
): string {
  const id = Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
  const outputFile = join(getTempDir(projectRoot), `bg-${id}.txt`);

  const task: BackgroundTask = {
    id,
    command,
    status: "running",
    outputFile,
    startedAt: Date.now(),
  };

  backgroundTasks.set(id, task);

  // Run async
  (async () => {
    try {
      const output = execSync(command, {
        cwd: projectRoot,
        timeout: 300000, // 5 min max
        encoding: "utf-8",
        maxBuffer: 5 * 1024 * 1024,
      });
      writeFileSync(outputFile, output, "utf-8");
      task.status = "completed";
      task.completedAt = Date.now();
      log("info", "Background task completed", { id, command });
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : String(err);
      writeFileSync(outputFile, `ERROR: ${msg}`, "utf-8");
      task.status = "failed";
      task.completedAt = Date.now();
      log("error", "Background task failed", { id, command, error: msg });
    }
  })();

  return id;
}

export function getBackgroundTaskStatus(id: string): BackgroundTask | undefined {
  return backgroundTasks.get(id);
}

export function getBackgroundTaskOutput(id: string): string | null {
  const task = backgroundTasks.get(id);
  if (!task) return null;
  if (!existsSync(task.outputFile)) return null;
  return readFileSync(task.outputFile, "utf-8");
}

export function listBackgroundTasks(): BackgroundTask[] {
  return [...backgroundTasks.values()];
}
