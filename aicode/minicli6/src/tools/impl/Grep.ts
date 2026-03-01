import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { readdir, stat, readFile } from 'fs/promises';
import path from 'path';

interface MatchResult {
  file: string;
  line: number;
  content: string;
}

export class GrepTool extends Tool {
  name = 'grep';
  description = 'Fast content search tool that works with any codebase size. Searches file contents using regular expressions. Supports full regex syntax (e.g., "log.*Error", "function\\s+\\w+"). Returns file paths and line numbers with matches, sorted by modification time.';
  parameters = {
    type: 'object' as const,
    properties: {
      pattern: {
        type: 'string' as const,
        description: 'Regular expression pattern to search for in file contents',
      },
      path: {
        type: 'string' as const,
        description: 'Directory to search in (default: current working directory)',
      },
      include: {
        type: 'string' as const,
        description: 'File pattern to include (e.g., *.ts, *.{js,ts})',
      },
    },
    required: ['pattern'],
  };
  permission: 'auto' = 'auto';

  private async walkDir(dir: string, results: string[]): Promise<void> {
    const entries = await readdir(dir, { withFileTypes: true });
    for (const entry of entries) {
      const fullPath = path.join(dir, entry.name);
      if (entry.isDirectory()) {
        if (!['node_modules', '.git', 'dist', 'build', '.next'].includes(entry.name)) {
          await this.walkDir(fullPath, results);
        }
      } else if (entry.isFile()) {
        results.push(fullPath);
      }
    }
  }

  private matchesPattern(filename: string, includePattern?: string): boolean {
    if (!includePattern) return true;

    const patterns = includePattern.replace(/^\*\./, '').split(',').map(p => p.trim().replace(/^\*\.?/, ''));
    const ext = path.extname(filename).slice(1);
    return patterns.some(p => ext === p || filename.endsWith(p));
  }

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { pattern, path: searchPath = context.workingDirectory, include } = call.arguments;

    let regex: RegExp;
    try {
      regex = new RegExp(pattern, 'gm');
    } catch (error) {
      return {
        success: false,
        output: '',
        error: `Invalid regex pattern: ${error instanceof Error ? error.message : String(error)}`,
      };
    }

    let files: string[];
    try {
      files = [];
      await this.walkDir(searchPath, files);
    } catch (error) {
      return {
        success: false,
        output: '',
        error: `Failed to read directory: ${error instanceof Error ? error.message : String(error)}`,
      };
    }

    const filteredFiles = files.filter(f => this.matchesPattern(f, include));

    const matches: MatchResult[] = [];
    const fileStats: Map<string, number> = new Map();

    await Promise.all(
      filteredFiles.map(async (file) => {
        try {
          const [content, stats] = await Promise.all([
            readFile(file, 'utf-8'),
            stat(file),
          ]);
          
          fileStats.set(file, stats.mtime.getTime());
          
          const lines = content.split('\n');
          for (let i = 0; i < lines.length; i++) {
            regex.lastIndex = 0;
            if (regex.test(lines[i])) {
              matches.push({
                file,
                line: i + 1,
                content: lines[i].trim().slice(0, 200),
              });
            }
          }
        } catch {
          // Skip files that can't be read
        }
      })
    );

    matches.sort((a, b) => (fileStats.get(b.file) || 0) - (fileStats.get(a.file) || 0));

    const uniqueFiles = [...new Set(matches.map(m => m.file))];

    if (matches.length === 0) {
      return {
        success: true,
        output: `No matches found for pattern "${pattern}" in ${searchPath}`,
      };
    }

    const output = matches
      .slice(0, 100)
      .map(m => `${m.file}:${m.line}: ${m.content}`)
      .join('\n');

    return {
      success: true,
      output: output + (matches.length > 100 ? `\n... and ${matches.length - 100} more matches` : ''),
      metadata: {
        totalMatches: matches.length,
        filesWithMatches: uniqueFiles.length,
        pattern,
        path: searchPath,
      },
    };
  }
}
