import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { main } from "../../src/index";
import { loadRuntimeConfig } from "../../src/config";

test("startup does not crash when config is malformed and prints warning", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-bad-config-"));
  const homeDir = path.join(tmpRoot, "home");
  const cwd = path.join(tmpRoot, "project");
  const projectConfigDir = path.join(cwd, ".minicli");
  fs.mkdirSync(path.join(homeDir, ".minicli"), { recursive: true });
  fs.mkdirSync(projectConfigDir, { recursive: true });
  fs.writeFileSync(path.join(projectConfigDir, "config.json"), "{ bad json", "utf8");

  const writes: string[] = [];
  const errors: string[] = [];

  try {
    const result = main(
      [],
      {
        stdout: (message) => writes.push(message),
        stderr: (message) => errors.push(message),
        loadConfig: () => loadRuntimeConfig({ homeDir, cwd })
      },
      "linux"
    );

    assert.equal(result, 0);
    assert.match(writes.join(""), /Starting minicli interactive shell\./);
    assert.match(errors.join(""), /\[config:parse_error\]/);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
