import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { DatabaseConnection, initializeDatabase } from "../../src/db";
import { createRepositories } from "../../src/repository";

function openSQLite(databasePath: string): DatabaseConnection {
  const sqliteModule = require("node:sqlite") as {
    DatabaseSync: new (dbPath: string) => DatabaseConnection;
  };
  return new sqliteModule.DatabaseSync(databasePath);
}

test("repository data is consistent after write and read", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-repo-int-"));
  try {
    const init = initializeDatabase({ cwd: tmpRoot });

    const writeConnection = openSQLite(init.databasePath);
    const writeRepos = createRepositories(writeConnection);
    const session = writeRepos.sessions.createSession("integration");
    writeRepos.messages.createMessage({
      sessionId: session.id,
      role: "user",
      content: "hello"
    });
    writeRepos.messages.createMessage({
      sessionId: session.id,
      role: "assistant",
      content: "world"
    });
    writeConnection.close();

    const readConnection = openSQLite(init.databasePath);
    const readRepos = createRepositories(readConnection);
    const sessions = readRepos.sessions.listSessions();
    const messages = readRepos.messages.listMessagesBySession(session.id);
    readConnection.close();

    assert.equal(sessions.length, 1);
    assert.equal(sessions[0].title, "integration");
    assert.equal(messages.length, 2);
    assert.deepEqual(
      messages.map((x) => [x.role, x.content]),
      [
        ["user", "hello"],
        ["assistant", "world"]
      ]
    );
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
