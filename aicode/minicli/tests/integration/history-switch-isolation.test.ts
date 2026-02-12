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

test("history output is isolated after switching sessions", async (t) => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-history-switch-"));
  const homeDir = path.join(tmpRoot, "home");
  const cwd = path.join(tmpRoot, "project");
  fs.mkdirSync(homeDir, { recursive: true });
  fs.mkdirSync(cwd, { recursive: true });

  const child = await spawnRepl(t, cwd, homeDir);
  if (child === null) {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
    return;
  }

  let stdout = "";
  let stderr = "";
  child.stdout.on("data", (chunk: Buffer) => {
    stdout += chunk.toString("utf8");
  });
  child.stderr.on("data", (chunk: Buffer) => {
    stderr += chunk.toString("utf8");
  });

  child.stdin.write("/new alpha\n");
  child.stdin.write("alpha note\n");
  child.stdin.write("/new beta\n");
  child.stdin.write("beta note\n");
  child.stdin.write("/switch #1\n");
  child.stdin.write("/history\n");
  child.stdin.write("/exit\n");
  child.stdin.end();

  let blocked = false;
  const exitCode = await new Promise<number>((resolve, reject) => {
    child.on("error", (error: NodeJS.ErrnoException) => {
      if (error.code === "EPERM") {
        t.skip("subprocess spawning is blocked in this environment");
        blocked = true;
        resolve(0);
        return;
      }
      reject(error);
    });
    child.on("close", (code) => resolve(code ?? -1));
  });

  try {
    if (blocked) {
      return;
    }

    assert.equal(exitCode, 0);
    assert.equal(stderr, "");
    assert.match(stdout, /\[1\] user: alpha note/);
    assert.match(stdout, /\[2\] assistant: mock\(glm-4\): alpha note/);
    assert.doesNotMatch(stdout, /\[\d+\] user: beta note/);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
