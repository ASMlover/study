import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { glob as globFn } from 'glob';
import { stat } from 'fs/promises';
import path from 'path';

export class GlobTool extends Tool {
  name = 'glob';
  description = 'Fast file pattern matching tool that works with any codebase size. Supports glob patterns like "**/*.js" or "src/**/*.ts". Returns matching file paths sorted by modification time.';
  parameters = {
    type: 'object' as const,
    properties: {
      pattern: {
        type: 'string' as const,
        description: 'Glob pattern to match files (e.g., **/*.ts, src/**/*.js)',
      },
      path: {
        type: 'string' as const,
        description: 'Directory to search in (default: current working directory)',
      },
    },
    required: ['pattern'],
  };
  permission: 'auto' = 'auto';

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { pattern, path: searchPath = context.workingDirectory } = call.arguments;

    try {
      const files = await globFn(pattern, {
        cwd: searchPath,
        nodir: true,
        absolute: true,
        ignore: ['**/node_modules/**', '**/.git/**', '**/dist/**'],
      });

      const filesWithStats = await Promise.all(
        files.map(async (file) => {
          try {
            const stats = await stat(file);
            return { file, mtime: stats.mtime.getTime() };
          } catch {
            return { file, mtime: 0 };
          }
        })
      );

      filesWithStats.sort((a, b) => b.mtime - a.mtime);

      const sortedFiles = filesWithStats.map((f) => f.file);

      if (sortedFiles.length === 0) {
        return {
          success: true,
          output: `No files matching pattern "${pattern}" found in ${searchPath}`,
        };
      }

      return {
        success: true,
        output: sortedFiles.join('\n'),
        metadata: {
          count: sortedFiles.length,
          pattern,
          path: searchPath,
        },
      };
    } catch (error) {
      return {
        success: false,
        output: '',
        error: `Glob search failed: ${error instanceof Error ? error.message : String(error)}`,
      };
    }
  }
}
