import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import {
  DatabaseConnection,
  initializeDatabase,
  resolveDatabasePath
} from "../../src/db";

function openSQLite(databasePath: string): DatabaseConnection {
  const sqliteModule = require("node:sqlite") as {
    DatabaseSync: new (dbPath: string) => DatabaseConnection;
  };
  return new sqliteModule.DatabaseSync(databasePath);
}

test("resolveDatabasePath points to project .minicli sqlite file", () => {
  const resolved = resolveDatabasePath(path.join("C:", "workspace", "project"));
  assert.equal(
    resolved,
    path.join("C:", "workspace", "project", ".minicli", "minicli.db")
  );
});

test("initializeDatabase creates required tables and indexes", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-db-unit-"));
  try {
    const result = initializeDatabase({ cwd: tmpRoot });
    assert.equal(result.migrationVersion, 1);
    assert.equal(fs.existsSync(result.databasePath), true);

    const connection = openSQLite(result.databasePath);
    try {
      const hasTable = (name: string): boolean =>
        connection.prepare(
          "SELECT name FROM sqlite_master WHERE type = 'table' AND name = ?"
        ).get(name) !== undefined;
      const hasIndex = (name: string): boolean =>
        connection.prepare(
          "SELECT name FROM sqlite_master WHERE type = 'index' AND name = ?"
        ).get(name) !== undefined;

      assert.equal(hasTable("sessions"), true);
      assert.equal(hasTable("messages"), true);
      assert.equal(hasTable("command_history"), true);
      assert.equal(hasTable("run_audit"), true);
      assert.equal(hasIndex("idx_messages_session_id_created_at"), true);
      assert.equal(hasIndex("idx_command_history_created_at"), true);
      assert.equal(hasIndex("idx_run_audit_created_at"), true);
    } finally {
      connection.close();
    }
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("initializeDatabase migration is idempotent", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-db-idem-"));
  try {
    const first = initializeDatabase({ cwd: tmpRoot });
    const second = initializeDatabase({ cwd: tmpRoot });
    assert.equal(first.databasePath, second.databasePath);

    const connection = openSQLite(first.databasePath);
    try {
      const row = connection.prepare(
        "SELECT COUNT(*) AS count FROM schema_migrations WHERE version = 1"
      ).get() as { count: number };
      assert.equal(row.count, 1);
    } finally {
      connection.close();
    }
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("initializeDatabase surfaces connection failure", () => {
  assert.throws(
    () =>
      initializeDatabase({
        cwd: "CWD",
        openDatabase: () => {
          throw new Error("SQLITE_CANTOPEN: unable to open database file");
        }
      }),
    /SQLITE_CANTOPEN/
  );
});

test("initializeDatabase surfaces permission error", () => {
  assert.throws(
    () =>
      initializeDatabase({
        cwd: "CWD",
        openDatabase: () => {
          throw new Error("EACCES: permission denied");
        }
      }),
    /EACCES/
  );
});

test("initializeDatabase closes connection when migration fails due to lock", () => {
  let closed = false;
  assert.throws(
    () =>
      initializeDatabase({
        cwd: "CWD",
        openDatabase: () => ({
          exec: () => {
            throw new Error("SQLITE_BUSY: database is locked");
          },
          prepare: () => ({
            all: () => [],
            get: () => undefined,
            run: () => undefined
          }),
          close: () => {
            closed = true;
          }
        })
      }),
    /database is locked/
  );

  assert.equal(closed, true);
});
