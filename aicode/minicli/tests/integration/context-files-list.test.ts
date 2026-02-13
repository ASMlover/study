import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

test("after /add, /files shows sorted context list with optional filters", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-context-files-"));
  const prevCwd = process.cwd();
  try {
    fs.mkdirSync(path.join(tmpRoot, "docs"));
    fs.mkdirSync(path.join(tmpRoot, "src"));
    fs.writeFileSync(path.join(tmpRoot, "src", "zeta.ts"), "zeta\n", "utf8");
    fs.writeFileSync(path.join(tmpRoot, "docs", "alpha.md"), "alpha\n", "utf8");
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

    await session.onLine("/add src/zeta.ts");
    await session.onLine("/add docs/alpha.md");
    writes.length = 0;
    await session.onLine("/files --q .md --limit 1");

    assert.equal(errors.join(""), "");
    assert.equal(writes.join(""), `Context files:\n[1] ${path.join("docs", "alpha.md")}\n`);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
