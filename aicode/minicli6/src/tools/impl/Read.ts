import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { readFile, stat } from 'fs/promises';
import { access, constants } from 'fs/promises';

export class ReadTool extends Tool {
  readonly name = 'read';
  readonly description = 'Read a file or directory from the local filesystem. If the path does not exist, an error is returned.';
  readonly parameters = {
    type: 'object' as const,
    properties: {
      filePath: {
        type: 'string' as const,
        description: 'The absolute path to the file or directory to read',
      },
      offset: {
        type: 'number' as const,
        description: 'Line number to start reading from (1-indexed)',
      },
      limit: {
        type: 'number' as const,
        description: 'Maximum number of lines to read',
      },
    },
    required: ['filePath'],
  };
  permission: 'auto' = 'auto';

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { filePath, offset = 1, limit = 2000 } = call.arguments;

    try {
      await access(filePath, constants.R_OK);
    } catch {
      return {
        success: false,
        output: '',
        error: `File or directory does not exist or is not readable: ${filePath}`,
      };
    }

    const stats = await stat(filePath);

    if (stats.isDirectory()) {
      return this.readDirectory(filePath);
    }

    return this.readFile(filePath, offset, limit, stats);
  }

  private async readDirectory(dirPath: string): Promise<ToolResult> {
    const { readdir } = await import('fs/promises');
    const entries = await readdir(dirPath, { withFileTypes: true });
    
    const lines = entries
      .sort((a, b) => a.name.localeCompare(b.name))
      .map(entry => `${entry.name}${entry.isDirectory() ? '/' : ''}`);

    return {
      success: true,
      output: lines.join('\n') || '(empty directory)',
      metadata: {
        type: 'directory',
        path: dirPath,
        count: lines.length,
      },
    };
  }

  private async readFile(
    filePath: string,
    offset: number,
    limit: number,
    stats: { size: number }
  ): Promise<ToolResult> {
    const MAX_FILE_SIZE = 10 * 1024 * 1024;
    
    if (stats.size > MAX_FILE_SIZE) {
      return {
        success: false,
        output: '',
        error: `File is too large (${(stats.size / 1024 / 1024).toFixed(2)}MB). Maximum size is 10MB.`,
      };
    }

    const content = await readFile(filePath, 'utf-8');
    const lines = content.split('\n');
    const totalLines = lines.length;

    const startLine = Math.max(1, offset) - 1;
    const endLine = Math.min(totalLines, startLine + limit);
    const selectedLines = lines.slice(startLine, endLine);

    const numberedLines = selectedLines.map((line, idx) => {
      const lineNum = startLine + idx + 1;
      const truncatedLine = line.length > 2000 ? line.slice(0, 2000) + '...[truncated]' : line;
      return `${lineNum}: ${truncatedLine}`;
    });

    let output = numberedLines.join('\n');
    
    if (totalLines > endLine) {
      output += `\n\n[Showing lines ${startLine + 1}-${endLine} of ${totalLines}]`;
    }

    return {
      success: true,
      output,
      metadata: {
        type: 'file',
        path: filePath,
        totalLines,
        displayedLines: numberedLines.length,
        offset: startLine + 1,
        limit,
      },
    };
  }
}
