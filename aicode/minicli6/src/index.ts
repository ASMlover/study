import { createCLI } from './cli.js';

createCLI()
  .then((cli) => cli.parse())
  .catch((error) => {
    console.error('Failed to initialize CLI:', error instanceof Error ? error.message : String(error));
    process.exit(1);
  });
