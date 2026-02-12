import fs from "node:fs";
import path from "node:path";

export interface DatabaseStatement {
  all: (...params: unknown[]) => unknown[];
  get: (...params: unknown[]) => unknown;
  run: (...params: unknown[]) => unknown;
}

export interface DatabaseConnection {
  exec: (sql: string) => void;
  prepare: (sql: string) => DatabaseStatement;
  close: () => void;
}

export type OpenDatabase = (databasePath: string) => DatabaseConnection;
type MkdirSync = typeof fs.mkdirSync;

export interface DatabaseInitResult {
  databasePath: string;
  migrationVersion: number;
}

const MIGRATION_V1 = `
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS schema_migrations (
  version INTEGER PRIMARY KEY,
  applied_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS sessions (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  title TEXT NOT NULL DEFAULT 'New Session',
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  updated_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS messages (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  session_id INTEGER NOT NULL,
  role TEXT NOT NULL CHECK (role IN ('system', 'user', 'assistant', 'tool')),
  content TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  FOREIGN KEY (session_id) REFERENCES sessions(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS command_history (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  command TEXT NOT NULL,
  cwd TEXT NOT NULL,
  exit_code INTEGER,
  created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE INDEX IF NOT EXISTS idx_messages_session_id_created_at
ON messages(session_id, created_at, id);

CREATE INDEX IF NOT EXISTS idx_command_history_created_at
ON command_history(created_at, id);

INSERT OR IGNORE INTO schema_migrations(version, applied_at)
VALUES (1, datetime('now'));
`;

export function resolveDatabasePath(cwd: string): string {
  return path.join(cwd, ".minicli", "minicli.db");
}

export function runMigrationV1(connection: DatabaseConnection): void {
  connection.exec(MIGRATION_V1);
}

export function withTransaction<T>(
  connection: DatabaseConnection,
  work: () => T
): T {
  connection.exec("BEGIN");
  try {
    const result = work();
    connection.exec("COMMIT");
    return result;
  } catch (error) {
    try {
      connection.exec("ROLLBACK");
    } catch {
      // Ignore rollback errors and preserve original error.
    }
    throw error;
  }
}

function defaultOpenDatabase(databasePath: string): DatabaseConnection {
  const sqliteModule = require("node:sqlite") as {
    DatabaseSync: new (path: string) => DatabaseConnection;
  };
  return new sqliteModule.DatabaseSync(databasePath);
}

export function initializeDatabase(params?: {
  cwd?: string;
  mkdirSync?: MkdirSync;
  openDatabase?: OpenDatabase;
}): DatabaseInitResult {
  const cwd = params?.cwd ?? process.cwd();
  const mkdirSync = params?.mkdirSync ?? fs.mkdirSync;
  const openDatabase = params?.openDatabase ?? defaultOpenDatabase;
  const databasePath = resolveDatabasePath(cwd);

  mkdirSync(path.dirname(databasePath), { recursive: true });

  let connection: DatabaseConnection | undefined;
  try {
    connection = openDatabase(databasePath);
    runMigrationV1(connection);
    return {
      databasePath,
      migrationVersion: 1
    };
  } catch (error) {
    const e = error as Error;
    throw new Error(`Failed to initialize database "${databasePath}": ${e.message}`);
  } finally {
    connection?.close();
  }
}
