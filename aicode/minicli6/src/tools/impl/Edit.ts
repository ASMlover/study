import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { readFile, writeFile } from 'fs/promises';
import { existsSync } from 'fs';

export class EditTool extends Tool {
  name = 'edit';
  description = 'Performs exact string replacements in files. You must use your Read tool at least once in the conversation before editing. This tool will error if you attempt an edit without reading the file first. This tool will error if oldString is not found in the file or is found multiple times (use replaceAll for multiple matches).';
  parameters = {
    type: 'object' as const,
    properties: {
      filePath: {
        type: 'string' as const,
        description: 'The absolute path to the file to modify',
      },
      oldString: {
        type: 'string' as const,
        description: 'The text to replace',
      },
      newString: {
        type: 'string' as const,
        description: 'The text to replace with',
      },
      replaceAll: {
        type: 'boolean' as const,
        description: 'Replace all occurrences of oldString (default: false)',
      },
    },
    required: ['filePath', 'oldString', 'newString'],
  };
  permission: 'confirm' = 'confirm';

  private readFiles: Set<string> = new Set();

  markAsRead(filePath: string): void {
    this.readFiles.add(filePath);
  }

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { filePath, oldString, newString, replaceAll = false } = call.arguments;

    if (!existsSync(filePath)) {
      return {
        success: false,
        output: '',
        error: `File not found: ${filePath}`,
      };
    }

    let content: string;
    try {
      content = await readFile(filePath, 'utf-8');
    } catch (error) {
      return {
        success: false,
        output: '',
        error: `Failed to read file: ${error instanceof Error ? error.message : String(error)}`,
      };
    }

    if (!content.includes(oldString)) {
      return {
        success: false,
        output: '',
        error: `oldString not found in file: ${filePath}`,
      };
    }

    const occurrences = content.split(oldString).length - 1;
    if (occurrences > 1 && !replaceAll) {
      return {
        success: false,
        output: '',
        error: `Found ${occurrences} matches for oldString. Use replaceAll: true to replace all occurrences.`,
      };
    }

    let newContent: string;
    if (replaceAll) {
      newContent = content.split(oldString).join(newString);
    } else {
      const index = content.indexOf(oldString);
      newContent = content.slice(0, index) + newString + content.slice(index + oldString.length);
    }

    try {
      await writeFile(filePath, newContent, 'utf-8');
    } catch (error) {
      return {
        success: false,
        output: '',
        error: `Failed to write file: ${error instanceof Error ? error.message : String(error)}`,
      };
    }

    return {
      success: true,
      output: `Successfully edited ${filePath}`,
      metadata: {
        replacements: replaceAll ? occurrences : 1,
      },
    };
  }
}
