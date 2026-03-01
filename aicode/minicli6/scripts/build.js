import * as esbuild from 'esbuild';

const args = process.argv.slice(2);
const isProduction = args.includes('--prod') || process.env.NODE_ENV === 'production';
const shouldWatch = args.includes('--watch');

const config = {
  entryPoints: ['src/index.ts'],
  bundle: true,
  outfile: 'dist/index.js',
  platform: 'node',
  target: 'node18',
  format: 'esm',
  banner: {
    js: '#!/usr/bin/env node',
  },
  
  minify: isProduction,
  sourcemap: !isProduction,
  treeShaking: true,
  
  external: [
    'ink',
    'react',
    'ink-spinner',
    'ink-text-input',
    'ink-select-input',
    'commander',
    'yaml',
    'chalk',
    'marked',
    'marked-terminal',
    'node-emoji',
  ],
  
  define: {
    'process.env.NODE_ENV': JSON.stringify(isProduction ? 'production' : 'development'),
  },
};

if (shouldWatch && !isProduction) {
  const ctx = await esbuild.context(config);
  await ctx.watch();
  console.log('Watching for changes...');
} else {
  await esbuild.build(config);
  console.log('Build complete!');
}
