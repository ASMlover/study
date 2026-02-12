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

test("messages after /switch are written into the switched target session", async (t) => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-switch-session-"));
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
  child.stdin.write("/new beta\n");
  child.stdin.write("/switch #1\n");
  child.stdin.write("persist to alpha\n");
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
    assert.match(stdout, /Switched to session #1: alpha/);

    const databasePath = path.join(cwd, ".minicli", "minicli.db");
    const connection = openSQLite(databasePath);
    const repositories = createRepositories(connection);
    const sessions = repositories.sessions.listSessions();
    const alpha = sessions.find((item) => item.title === "alpha");
    const beta = sessions.find((item) => item.title === "beta");
    assert.ok(alpha);
    assert.ok(beta);

    const alphaMessages = repositories.messages.listMessagesBySession(alpha.id);
    const betaMessages = repositories.messages.listMessagesBySession(beta.id);
    connection.close();

    assert.deepEqual(
      alphaMessages.map((item) => [item.role, item.content]),
      [
        ["user", "persist to alpha"],
        ["assistant", "mock(glm-4): persist to alpha"]
      ]
    );
    assert.deepEqual(betaMessages, []);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
