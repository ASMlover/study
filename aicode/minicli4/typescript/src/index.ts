import { runCli } from "./cli";

const code = runCli(process.argv.slice(2));
if (code !== 0) {
  process.exitCode = code;
}
