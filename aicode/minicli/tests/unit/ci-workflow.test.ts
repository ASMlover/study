import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";

const workflowPath = path.resolve(".github", "workflows", "ci.yml");

function readWorkflow(): string {
  assert.equal(fs.existsSync(workflowPath), true, "CI workflow file must exist");
  return fs.readFileSync(workflowPath, "utf8");
}

test("workflow has valid core sections", () => {
  const yaml = readWorkflow();
  assert.match(yaml, /^name:\s+CI Quality Gate/m);
  assert.match(yaml, /^on:\s*$/m);
  assert.match(yaml, /^\s+pull_request:\s*$/m);
  assert.match(yaml, /^\s+push:\s*$/m);
  assert.match(yaml, /^jobs:\s*$/m);
});

test("workflow enforces gate order via needs chain", () => {
  const yaml = readWorkflow();
  assert.match(yaml, /^  typecheck:\s*$/m);
  assert.match(yaml, /^  unit:\s*[\s\S]*?^\s+needs:\s+typecheck\s*$/m);
  assert.match(yaml, /^  integration:\s*[\s\S]*?^\s+needs:\s+unit\s*$/m);
  assert.match(yaml, /^  coverage:\s*[\s\S]*?^\s+needs:\s+integration\s*$/m);
  assert.match(yaml, /^  e2e:\s*[\s\S]*?^\s+needs:\s+coverage\s*$/m);
});

test("workflow has coverage threshold gate command", () => {
  const yaml = readWorkflow();
  assert.match(yaml, /npm run test:coverage/);
  assert.match(
    fs.readFileSync(path.resolve("package.json"), "utf8"),
    /--test-coverage-lines=80[\s\S]*--test-coverage-functions=80[\s\S]*--test-coverage-branches=70/
  );
});

test("workflow includes retry, cache, and artifacts strategy", () => {
  const yaml = readWorkflow();
  assert.match(yaml, /cache:\s+npm/);
  assert.match(yaml, /retry once/i);
  assert.match(yaml, /actions\/upload-artifact@v4/);
});
