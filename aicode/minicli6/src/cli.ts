import { Command } from 'commander';
import { readFileSync } from 'fs';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';
import React from 'react';
import { render } from 'ink';
import { App } from './tui/App.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

function getVersion(): string {
  try {
    const pkg = JSON.parse(
      readFileSync(join(__dirname, '..', 'package.json'), 'utf-8')
    );
    return pkg.version;
  } catch {
    return '0.1.0';
  }
}

export async function createCLI(): Promise<Command> {
  const program = new Command();

  program
    .name('minicli')
    .description('AI Agent CLI tool powered by GLM5')
    .version(getVersion());

  program
    .command('start')
    .description('Start interactive agent session')
    .argument('[prompt]', 'Initial prompt to send to the agent')
    .option('-c, --config <path>', 'Config file path', './minicli.yaml')
    .option('--model <model>', 'AI model to use')
    .option('--debug', 'Enable debug mode')
    .action(async (prompt: string | undefined, options: { config: string; model?: string; debug?: boolean }) => {
      try {
        if (options.debug) {
          console.log('Config:', options.config);
          if (options.model) {
            console.log('Model:', options.model);
          }
          if (prompt) {
            console.log('Initial prompt:', prompt);
          }
        }

        const { waitUntilExit } = render(React.createElement(App, { 
          initialPrompt: prompt,
          configPath: options.config,
          model: options.model,
          debug: options.debug,
        }));

        await waitUntilExit();
      } catch (error) {
        console.error('Failed to start:', error instanceof Error ? error.message : String(error));
        process.exit(1);
      }
    });

  return program;
}
