import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

test("tree prints directory layout with depth limit", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-tree-integration-"));
  const prevCwd = process.cwd();
  try {
    fs.mkdirSync(path.join(tmpRoot, "docs"));
    fs.mkdirSync(path.join(tmpRoot, "src"));
    fs.mkdirSync(path.join(tmpRoot, "src", "nested"));
    fs.writeFileSync(path.join(tmpRoot, "docs", "guide.md"), "guide\n", "utf8");
    fs.writeFileSync(path.join(tmpRoot, "src", "main.ts"), "main\n", "utf8");
    fs.writeFileSync(path.join(tmpRoot, "src", "nested", "deep.txt"), "deep\n", "utf8");
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

    await session.onLine("/tree --depth 2");

    assert.equal(errors.join(""), "");
    assert.equal(
      writes.join(""),
      [
        "Tree:",
        "./",
        "|-- docs/",
        "|   `-- guide.md",
        "`-- src/",
        "    |-- main.ts",
        "    `-- nested/",
        ""
      ].join("\n")
    );
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
