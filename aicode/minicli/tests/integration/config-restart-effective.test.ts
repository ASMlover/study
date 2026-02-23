import test, { TestContext } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { spawn } from "node:child_process";

async function spawnRepl(
  t: TestContext,
  cwd: string,
  homeDir: string
) {
  const scriptPath = path.resolve("build/src/index.js");
  try {
    return spawn(process.execPath, [scriptPath], {
      cwd,
      env: {
        ...process.env,
        HOME: homeDir,
        USERPROFILE: homeDir
      },
      stdio: ["pipe", "pipe", "pipe"]
    });
  } catch (error) {
    const e = error as NodeJS.ErrnoException;
    if (e.code === "EPERM") {
      t.skip("subprocess spawning is blocked in this environment");
      return null;
    }
    throw error;
  }
}

async function runReplScript(
  t: TestContext,
  cwd: string,
  homeDir: string,
  scriptLines: readonly string[]
): Promise<{ stdout: string; stderr: string; code: number; blocked: boolean }> {
  const child = await spawnRepl(t, cwd, homeDir);
  if (child === null) {
    return { stdout: "", stderr: "", code: 0, blocked: true };
  }

  let stdout = "";
  let stderr = "";
  child.stdout.on("data", (chunk: Buffer) => {
    stdout += chunk.toString("utf8");
  });
  child.stderr.on("data", (chunk: Buffer) => {
    stderr += chunk.toString("utf8");
  });

  for (const line of scriptLines) {
    child.stdin.write(`${line}\n`);
  }
  child.stdin.end();

  let blocked = false;
  const code = await new Promise<number>((resolve, reject) => {
    child.on("error", (error: NodeJS.ErrnoException) => {
      if (error.code === "EPERM") {
        t.skip("subprocess spawning is blocked in this environment");
        blocked = true;
        resolve(0);
        return;
      }
      reject(error);
    });
    child.on("close", (exitCode) => resolve(exitCode ?? -1));
  });

  return { stdout, stderr, code, blocked };
}

test("/config set persists and is effective after restart", async (t) => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-config-restart-"));
  const homeDir = path.join(tmpRoot, "home");
  const cwd = path.join(tmpRoot, "project");
  fs.mkdirSync(homeDir, { recursive: true });
  fs.mkdirSync(cwd, { recursive: true });

  try {
    const first = await runReplScript(t, cwd, homeDir, [
      "/config set timeoutMs 45000",
      "/exit"
    ]);
    if (first.blocked) {
      return;
    }
    assert.equal(first.code, 0);
    assert.equal(first.stderr, "");
    assert.match(first.stdout, /\[config\] updated timeoutMs=45000/);

    const second = await runReplScript(t, cwd, homeDir, [
      "/config get timeoutMs",
      "/exit"
    ]);
    assert.equal(second.code, 0);
    assert.equal(second.stderr, "");
    assert.match(second.stdout, /timeoutMs=45000/);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
