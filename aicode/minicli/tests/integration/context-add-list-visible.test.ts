import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

test("after /add, context list is visible in output", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-context-add-"));
  const prevCwd = process.cwd();
  try {
    const target = path.join(tmpRoot, "guide.md");
    fs.writeFileSync(target, "# guide\n", "utf8");
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

    await session.onLine("/add guide.md");

    const output = writes.join("");
    assert.equal(errors.join(""), "");
    assert.match(output, /\[add\] added:/);
    assert.match(output, /Context files:/);
    assert.match(output, /\[1\] guide\.md/);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
