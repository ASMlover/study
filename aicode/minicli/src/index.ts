#!/usr/bin/env node
import { CliIo, runCli } from "./cli";

export function main(
  argv: string[],
  io?: CliIo,
  platform?: NodeJS.Platform
): number {
  return runCli(argv, io, platform);
}

if (require.main === module) {
  process.exitCode = main(process.argv.slice(2));
}
