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

test("login then ask question returns assistant reply", async (t) => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-login-chat-"));
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

  child.stdin.write("/login sk-87654321\n");
  child.stdin.write("hello after login\n");
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
    if (!blocked) {
      assert.equal(exitCode, 0);
      assert.equal(stderr, "");
      assert.match(stdout, /API key saved: \*+4321/);
      assert.match(stdout, /mock\(glm-4\): hello after login/);
      assert.match(stdout, /Bye\./);
    }
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
