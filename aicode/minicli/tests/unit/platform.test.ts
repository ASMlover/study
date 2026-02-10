import test from "node:test";
import assert from "node:assert/strict";
import { resolveBinaryName } from "../../src/cli";

test("uses .exe binary on windows", () => {
  assert.equal(resolveBinaryName("win32"), "minicli.exe");
});

test("uses unix binary name for linux", () => {
  assert.equal(resolveBinaryName("linux"), "minicli");
});
