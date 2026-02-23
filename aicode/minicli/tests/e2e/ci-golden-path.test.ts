import test from "node:test";
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import path from "node:path";
import { APP_VERSION } from "../../src/version";

test("golden path: compiled cli returns version in smoke run", (t) => {
  const cliPath = path.resolve("build", "src", "index.js");
  const result = spawnSync(process.execPath, [cliPath, "--version"], {
    encoding: "utf8",
    stdio: "pipe"
  });

  const error = result.error as NodeJS.ErrnoException | undefined;
  if (error && error.code === "EPERM") {
    t.skip("subprocess execution is blocked in this environment");
    return;
  }

  assert.equal(result.status, 0, result.stderr ?? "");
  assert.match((result.stdout ?? "").trim(), new RegExp(`^minicli ${APP_VERSION}$`));
});
