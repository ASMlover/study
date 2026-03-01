import { exec } from 'child_process';
import { promisify } from 'util';
import fs from 'fs/promises';
import path from 'path';

const execAsync = promisify(exec);

const platforms = [
  { target: 'node18-linux-x64', output: 'minicli-linux' },
  { target: 'node18-macos-x64', output: 'minicli-macos' },
  { target: 'node18-macos-arm64', output: 'minicli-macos-arm64' },
  { target: 'node18-win-x64', output: 'minicli-win.exe' },
];

async function build() {
  console.log('Building with esbuild...');
  await execAsync('node scripts/build.js');

  await fs.mkdir('release', { recursive: true });

  for (const platform of platforms) {
    console.log(`Packaging for ${platform.target}...`);
    try {
      await execAsync(
        `npx pkg dist/index.js --target ${platform.target} --output release/${platform.output}`
      );
    } catch (err) {
      console.warn(`Warning: Failed to package ${platform.target}:`, err.message);
    }
  }

  console.log('Packaging complete!');
}

build().catch(console.error);
