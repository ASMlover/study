import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { writeFile, mkdir, access, stat } from 'fs/promises';
import { dirname } from 'path';
import { constants } from 'fs';

export class WriteTool extends Tool {
  readonly name = 'write';
  readonly description = 'Write content to a file. This will overwrite existing files.';
  readonly parameters = {
    type: 'object' as const,
    properties: {
      filePath: {
        type: 'string' as const,
        description: 'Absolute path to the file to write',
      },
      content: {
        type: 'string' as const,
        description: 'Content to write to the file',
      },
    },
    required: ['filePath', 'content'],
  };
  permission: 'confirm' = 'confirm';

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { filePath, content } = call.arguments;

    try {
      const exists = await this.fileExists(filePath);
      const dir = dirname(filePath);

      await this.ensureDirectory(dir);
      await writeFile(filePath, content, 'utf-8');

      return {
        success: true,
        output: `Successfully wrote ${content.length} characters to ${filePath}`,
        metadata: {
          path: filePath,
          size: content.length,
          isNew: !exists,
        },
      };
    } catch (error) {
      return {
        success: false,
        output: '',
        error: error instanceof Error ? error.message : String(error),
      };
    }
  }

  private async fileExists(filePath: string): Promise<boolean> {
    try {
      await access(filePath, constants.F_OK);
      return true;
    } catch {
      return false;
    }
  }

  private async ensureDirectory(dir: string): Promise<void> {
    try {
      await access(dir, constants.F_OK);
      const stats = await stat(dir);
      if (!stats.isDirectory()) {
        throw new Error(`Path exists but is not a directory: ${dir}`);
      }
    } catch (error: any) {
      if (error.code === 'ENOENT') {
        await mkdir(dir, { recursive: true });
      } else {
        throw error;
      }
    }
  }
}
