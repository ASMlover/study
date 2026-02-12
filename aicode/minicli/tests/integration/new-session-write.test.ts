import test, { TestContext } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { spawn } from "node:child_process";
import { DatabaseConnection } from "../../src/db";
import { createRepositories } from "../../src/repository";

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

function openSQLite(databasePath: string): DatabaseConnection {
  const sqliteModule = require("node:sqlite") as {
    DatabaseSync: new (dbPath: string) => DatabaseConnection;
  };
  return new sqliteModule.DatabaseSync(databasePath);
}

test("messages after /new are written into the new session", async (t) => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-new-session-"));
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

  child.stdin.write("/new sprint-alpha\n");
  child.stdin.write("persist this\n");
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
    assert.match(stdout, /Switched to session #\d+: sprint-alpha/);

    const databasePath = path.join(cwd, ".minicli", "minicli.db");
    const connection = openSQLite(databasePath);
    const repositories = createRepositories(connection);
    const sessions = repositories.sessions.listSessions();
    assert.equal(sessions.length, 1);
    assert.equal(sessions[0].title, "sprint-alpha");

    const messages = repositories.messages.listMessagesBySession(sessions[0].id);
    connection.close();

    assert.deepEqual(
      messages.map((x) => [x.role, x.content]),
      [
        ["user", "persist this"],
        ["assistant", "mock(glm-4): persist this"]
      ]
    );
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
