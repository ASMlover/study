import test from "node:test";
import assert from "node:assert/strict";
import { main } from "../../src/index";

test("minicli --version exits with code 0", () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const result = main(
    ["--version"],
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    "linux"
  );

  assert.equal(result, 0);
  assert.equal(errors.length, 0);
  assert.notEqual(writes.join("").trim().length, 0);
});
