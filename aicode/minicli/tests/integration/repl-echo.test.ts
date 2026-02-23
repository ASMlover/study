import test, { TestContext } from "node:test";
import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import path from "node:path";

async function spawnRepl(t: TestContext) {
  const scriptPath = path.resolve("build/src/index.js");
  try {
    return spawn(process.execPath, [scriptPath], {
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

test("repl calls mock provider and prints response in subprocess mode", async (t) => {
  const child = await spawnRepl(t);
  if (child === null) {
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

  child.stdin.write("hello repl\n");
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

  if (blocked) {
    return;
  }

  assert.equal(exitCode, 0);
  assert.equal(stderr, "");
  assert.match(stdout, /mock\(glm-4\): hello repl/);
  assert.match(stdout, /EOF received\. Bye\./);
});

test("repl supports /help to /exit flow", async (t) => {
  const child = await spawnRepl(t);
  if (child === null) {
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

  child.stdin.write("/help\n");
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

  if (blocked) {
    return;
  }

  assert.equal(exitCode, 0);
  assert.equal(stderr, "");
  assert.match(stdout, /Available commands:/);
  assert.match(stdout, /\/help\s+\[perm:public\]\s+Show this help message/);
  assert.match(stdout, /\/exit\s+\[perm:public\]\s+Exit MiniCLI/);
  assert.match(stdout, /Bye\./);
});
