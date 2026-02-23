import test from "node:test";
import assert from "node:assert/strict";
import {
  createReleaseBuildPlan,
  findMissingPackagingDependencies,
  formatMissingDependencyError,
  parseReleaseBuildArgs,
  resolvePackageOutputName,
  ensureReleaseEntrypointExists,
  validateEntryPathShape
} from "../../src/release";

test("build plan injects app version into esbuild define", () => {
  const options = parseReleaseBuildArgs(["--version", "1.2.3"]);
  const plan = createReleaseBuildPlan(options);

  assert.equal(plan.versionDefineValue, '"1.2.3"');
});

test("binary naming follows platform conventions", () => {
  assert.equal(resolvePackageOutputName("minicli", "win-x64"), "minicli-win-x64.exe");
  assert.equal(resolvePackageOutputName("minicli", "linux-x64"), "minicli-linux-x64");
});

test("reports missing packaging dependencies", () => {
  const missing = findMissingPackagingDependencies(() => {
    throw new Error("not found");
  });

  assert.deepEqual(missing, ["esbuild", "pkg"]);
  assert.match(formatMissingDependencyError(missing), /Missing release dependencies/);
});

test("validates release entrypoint existence", () => {
  assert.throws(
    () => ensureReleaseEntrypointExists("src/missing.ts", () => false),
    /Release entry not found/
  );

  assert.doesNotThrow(() =>
    ensureReleaseEntrypointExists("src/index.ts", () => true)
  );
});

test("validates release entry file extension", () => {
  assert.throws(() => validateEntryPathShape("src/index.txt"), /.ts or .js/);
  assert.doesNotThrow(() => validateEntryPathShape("src/index.ts"));
  assert.doesNotThrow(() => validateEntryPathShape("build/index.js"));
});

test("argument parsing handles boundaries and rejects invalid options", () => {
  const parsed = parseReleaseBuildArgs([
    "--targets",
    "win-x64,linux-x64,win-x64",
    "--binary-name",
    "mini"
  ]);
  assert.deepEqual(parsed.targets, ["win-x64", "linux-x64"]);
  assert.equal(parsed.binaryBaseName, "mini");

  assert.throws(() => parseReleaseBuildArgs(["--targets", ""]), /At least one release target/);
  assert.throws(() => parseReleaseBuildArgs(["--targets", "darwin-arm64"]), /Unsupported release target/);
  assert.throws(() => parseReleaseBuildArgs(["--unknown", "x"]), /Unknown option/);
  assert.throws(() => parseReleaseBuildArgs(["--entry"]), /Missing value/);
});
