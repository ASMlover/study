import { startRepl } from "./repl";

export function runCli(argv: string[]): number {
  if (argv.length === 1 && (argv[0] === "-v" || argv[0] === "--version")) {
    process.stdout.write("minicli4-ts 0.1.0\n");
    return 0;
  }

  if (argv.length > 0 && argv[0] !== "repl") {
    process.stderr.write(`Unknown argument: ${argv[0]}\n`);
    return 1;
  }

  startRepl({
    input: process.stdin,
    output: process.stdout,
    stdout: (text) => process.stdout.write(text),
    stderr: (text) => process.stderr.write(text)
  });
  return 0;
}
