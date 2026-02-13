import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { DatabaseConnection, initializeDatabase } from "../../src/db";
import { createRepositories } from "../../src/repository";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

function openSQLite(databasePath: string): DatabaseConnection {
  const sqliteModule = require("node:sqlite") as {
    DatabaseSync: new (dbPath: string) => DatabaseConnection;
  };
  return new sqliteModule.DatabaseSync(databasePath);
}

test("after /run, /history --audit can query execution audits", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-audit-history-"));
  const init = initializeDatabase({ cwd: tmpRoot });
  const connection = openSQLite(init.databasePath);
  try {
    const repositories = createRepositories(connection);
    const writes: string[] = [];
    const errors: string[] = [];
    const session = createReplSession(
      {
        stdout: (message) => writes.push(message),
        stderr: (message) => errors.push(message)
      },
      DEFAULT_MAX_INPUT_LENGTH,
      {
        runAuditRepository: repositories.runAudit,
        executeRunCommand: () => ({
          ok: true,
          stdout: "C:/workspace",
          stderr: "",
          exitCode: 0,
          stdoutTruncated: false,
          stderrTruncated: false
        })
      }
    );

    await session.onLine("/run pwd");
    writes.length = 0;
    await session.onLine("/history --audit");

    const output = writes.join("");
    assert.equal(errors.join(""), "");
    assert.match(output, /status=not_required/);
    assert.match(output, /risk=low/);
    assert.match(output, /cmd=pwd/);
    assert.match(output, /out=C:\/workspace/);
  } finally {
    connection.close();
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
