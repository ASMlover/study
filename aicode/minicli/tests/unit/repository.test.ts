import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { DatabaseConnection, initializeDatabase } from "../../src/db";
import { createRepositories, MessageRole } from "../../src/repository";

function openSQLite(databasePath: string): DatabaseConnection {
  const sqliteModule = require("node:sqlite") as {
    DatabaseSync: new (dbPath: string) => DatabaseConnection;
  };
  return new sqliteModule.DatabaseSync(databasePath);
}

function createTestStorage(): {
  tmpRoot: string;
  connection: DatabaseConnection;
  cleanup: () => void;
} {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-repo-unit-"));
  const init = initializeDatabase({ cwd: tmpRoot });
  const connection = openSQLite(init.databasePath);
  return {
    tmpRoot,
    connection,
    cleanup: () => {
      connection.close();
      fs.rmSync(tmpRoot, { recursive: true, force: true });
    }
  };
}

test("SessionRepository creates new session", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const session = repositories.sessions.createSession("My Session");
    assert.equal(session.id > 0, true);
    assert.equal(session.title, "My Session");
  } finally {
    storage.cleanup();
  }
});

test("MessageRepository writes message for existing session", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const session = repositories.sessions.createSession("Chat");
    const message = repositories.messages.createMessage({
      sessionId: session.id,
      role: "user",
      content: "hello"
    });
    assert.equal(message.id > 0, true);
    assert.equal(message.sessionId, session.id);
    assert.equal(message.role, "user");
    assert.equal(message.content, "hello");
  } finally {
    storage.cleanup();
  }
});

test("SessionRepository list is sorted by updated_at desc then id desc", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const older = repositories.sessions.createSession("older");
    const newer = repositories.sessions.createSession("newer");
    storage.connection
      .prepare("UPDATE sessions SET updated_at = ? WHERE id = ?")
      .run("2026-01-01 00:00:00", older.id);
    storage.connection
      .prepare("UPDATE sessions SET updated_at = ? WHERE id = ?")
      .run("2026-01-02 00:00:00", newer.id);

    const sessions = repositories.sessions.listSessions();
    assert.deepEqual(
      sessions.map((x) => x.id),
      [newer.id, older.id]
    );
  } finally {
    storage.cleanup();
  }
});

test("MessageRepository list supports pagination boundaries", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const session = repositories.sessions.createSession("pager");
    repositories.messages.createMessage({
      sessionId: session.id,
      role: "user",
      content: "m1"
    });
    repositories.messages.createMessage({
      sessionId: session.id,
      role: "assistant",
      content: "m2"
    });

    const first = repositories.messages.listMessagesBySession(session.id, {
      limit: 1,
      offset: 0
    });
    assert.equal(first.length, 1);
    assert.equal(first[0].content, "m1");

    const none = repositories.messages.listMessagesBySession(session.id, {
      limit: 1,
      offset: 10
    });
    assert.deepEqual(none, []);

    const zero = repositories.messages.listMessagesBySession(session.id, {
      limit: 0,
      offset: 0
    });
    assert.deepEqual(zero, []);
  } finally {
    storage.cleanup();
  }
});

test("repositories return empty results when no rows", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const sessions = repositories.sessions.listSessions();
    assert.deepEqual(sessions, []);
    const messages = repositories.messages.listMessagesBySession(999);
    assert.deepEqual(messages, []);
  } finally {
    storage.cleanup();
  }
});

test("MessageRepository transactional write rolls back on failure", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const session = repositories.sessions.createSession("tx");
    assert.throws(
      () =>
        repositories.messages.createMessagesInTransaction(session.id, [
          {
            role: "user",
            content: "ok"
          },
          {
            role: "invalid-role" as MessageRole,
            content: "bad"
          }
        ]),
      /CHECK constraint failed/
    );

    const rows = repositories.messages.listMessagesBySession(session.id);
    assert.deepEqual(rows, []);
  } finally {
    storage.cleanup();
  }
});

test("CommandHistoryRepository records command usage and aggregates frequency", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    repositories.commandHistory.recordCommand({
      command: "/switch",
      cwd: storage.tmpRoot
    });
    repositories.commandHistory.recordCommand({
      command: "/switch",
      cwd: storage.tmpRoot
    });
    repositories.commandHistory.recordCommand({
      command: "/sessions",
      cwd: storage.tmpRoot
    });

    const frequencies = repositories.commandHistory.listUsageFrequency();
    assert.equal(frequencies["/switch"], 2);
    assert.equal(frequencies["/sessions"], 1);
  } finally {
    storage.cleanup();
  }
});

test("CommandHistoryRepository persists usage across reopened connections", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-history-reopen-"));
  const init = initializeDatabase({ cwd: tmpRoot });
  const first = openSQLite(init.databasePath);
  try {
    const repositories = createRepositories(first);
    repositories.commandHistory.recordCommand({
      command: "/switch",
      cwd: tmpRoot
    });
  } finally {
    first.close();
  }

  const second = openSQLite(init.databasePath);
  try {
    const repositories = createRepositories(second);
    const frequencies = repositories.commandHistory.listUsageFrequency();
    assert.equal(frequencies["/switch"], 1);
  } finally {
    second.close();
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("RunAuditRepository stores audit rows with approval status and result fields", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const created = repositories.runAudit.recordAudit({
      command: "pwd | wc -c",
      riskLevel: "medium",
      approvalStatus: "approved",
      executed: true,
      exitCode: 0,
      stdout: "42",
      stderr: ""
    });

    assert.equal(created.id > 0, true);
    assert.equal(created.command, "pwd | wc -c");
    assert.equal(created.riskLevel, "medium");
    assert.equal(created.approvalStatus, "approved");
    assert.equal(created.executed, true);
    assert.equal(created.exitCode, 0);
    assert.equal(created.stdout, "42");
    assert.equal(created.stderr, "");
    assert.match(created.createdAt, /^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$/);
  } finally {
    storage.cleanup();
  }
});

test("RunAuditRepository supports approval status filtering", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    repositories.runAudit.recordAudit({
      command: "pwd | wc -c",
      riskLevel: "medium",
      approvalStatus: "rejected",
      executed: false
    });
    repositories.runAudit.recordAudit({
      command: "rm -rf .",
      riskLevel: "high",
      approvalStatus: "timeout",
      executed: false
    });
    repositories.runAudit.recordAudit({
      command: "pwd",
      riskLevel: "low",
      approvalStatus: "not_required",
      executed: true,
      exitCode: 0
    });

    const onlyRejected = repositories.runAudit.listAudits({
      approvalStatus: "rejected"
    });
    assert.equal(onlyRejected.length, 1);
    assert.equal(onlyRejected[0].approvalStatus, "rejected");
    assert.equal(onlyRejected[0].command, "pwd | wc -c");
  } finally {
    storage.cleanup();
  }
});

test("RunAuditRepository supports pagination with recent-first order", () => {
  const storage = createTestStorage();
  try {
    const repositories = createRepositories(storage.connection);
    const first = repositories.runAudit.recordAudit({
      command: "first",
      riskLevel: "low",
      approvalStatus: "not_required",
      executed: true,
      exitCode: 0
    });
    const second = repositories.runAudit.recordAudit({
      command: "second",
      riskLevel: "medium",
      approvalStatus: "rejected",
      executed: false
    });
    const third = repositories.runAudit.recordAudit({
      command: "third",
      riskLevel: "high",
      approvalStatus: "timeout",
      executed: false
    });

    storage.connection
      .prepare("UPDATE run_audit SET created_at = ? WHERE id = ?")
      .run("2026-01-01 00:00:00", first.id);
    storage.connection
      .prepare("UPDATE run_audit SET created_at = ? WHERE id = ?")
      .run("2026-01-02 00:00:00", second.id);
    storage.connection
      .prepare("UPDATE run_audit SET created_at = ? WHERE id = ?")
      .run("2026-01-03 00:00:00", third.id);

    const paged = repositories.runAudit.listAudits({
      limit: 1,
      offset: 1
    });
    assert.equal(paged.length, 1);
    assert.equal(paged[0].command, "second");
  } finally {
    storage.cleanup();
  }
});
