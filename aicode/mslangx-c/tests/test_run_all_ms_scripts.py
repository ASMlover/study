import contextlib
import importlib.util
import io
import pathlib
import tempfile
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
SCRIPT_PATH = REPO_ROOT / "tests" / "run_all_ms_scripts.py"


def load_module():
  spec = importlib.util.spec_from_file_location("run_all_ms_scripts", SCRIPT_PATH)
  module = importlib.util.module_from_spec(spec)
  assert spec.loader is not None
  spec.loader.exec_module(module)
  return module


class RunAllMsScriptsTest(unittest.TestCase):

  def test_find_ms_files_ignores_sidecar_artifacts(self):
    module = load_module()

    with tempfile.TemporaryDirectory() as temp_dir:
      root = pathlib.Path(temp_dir)
      (root / "alpha.ms").write_text("print 1\n", encoding="utf-8", newline="\n")
      (root / "alpha.ms.ast").write_text("", encoding="utf-8", newline="\n")
      (root / "beta.ms.diag").write_text("", encoding="utf-8", newline="\n")
      nested = root / "nested"
      nested.mkdir()
      (nested / "gamma.ms").write_text("print 2\n", encoding="utf-8", newline="\n")
      (root / "note.txt").write_text("", encoding="utf-8", newline="\n")

      actual = module.find_ms_files(root)

    self.assertEqual(
      [path.as_posix() for path in actual],
      ["alpha.ms", "nested/gamma.ms"],
    )

  def test_main_runs_all_scripts_and_reports_failures(self):
    module = load_module()

    with tempfile.TemporaryDirectory() as temp_dir:
      root = pathlib.Path(temp_dir)
      runner = root / "fake_runner.cmd"
      runner.write_text(
        "@echo off\n"
        "if /I \"%~nx1\"==\"fail.ms\" (\n"
        "  echo failed %~nx1 1>&2\n"
        "  exit /b 7\n"
        ")\n"
        "echo ran %~nx1\n"
        "exit /b 0\n",
        encoding="utf-8",
        newline="\n",
      )
      (root / "ok.ms").write_text("print 1\n", encoding="utf-8", newline="\n")
      (root / "fail.ms").write_text("print 2\n", encoding="utf-8", newline="\n")

      stdout = io.StringIO()
      stderr = io.StringIO()
      with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
        exit_code = module.main(["--runner", str(runner), "--root", str(root)])

    self.assertEqual(exit_code, 1)
    self.assertIn("PASS ok.ms", stdout.getvalue())
    self.assertIn("FAIL fail.ms", stdout.getvalue())
    self.assertIn("failed fail.ms", stderr.getvalue())
    self.assertIn("Summary: 1 passed, 1 failed, 2 total", stdout.getvalue())


if __name__ == "__main__":
  unittest.main()