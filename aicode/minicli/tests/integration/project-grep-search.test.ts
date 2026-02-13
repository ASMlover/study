import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

test("grep searches real project files and prints matches", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-grep-integration-"));
  const prevCwd = process.cwd();
  try {
    fs.mkdirSync(path.join(tmpRoot, "src"));
    fs.writeFileSync(
      path.join(tmpRoot, "src", "notes.txt"),
      ["alpha", "needle item", "omega"].join("\n"),
      "utf8"
    );
    process.chdir(tmpRoot);

    const writes: string[] = [];
    const errors: string[] = [];
    const session = createReplSession(
      {
        stdout: (message) => writes.push(message),
        stderr: (message) => errors.push(message)
      },
      DEFAULT_MAX_INPUT_LENGTH
    );

    await session.onLine("/grep needle");

    assert.equal(errors.join(""), "");
    assert.equal(
      writes.join(""),
      `Grep matches:\n[1] ${path.join("src", "notes.txt")}:2: needle item\n`
    );
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
