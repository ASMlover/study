import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { spawn, SpawnOptions } from 'child_process';

export class BashTool extends Tool {
  readonly name = 'bash';
  readonly description = 'Execute shell commands in a persistent shell session with optional timeout, ensuring proper handling and security measures.';
  readonly parameters = {
    type: 'object' as const,
    properties: {
      command: {
        type: 'string' as const,
        description: 'The command to execute',
      },
      timeout: {
        type: 'number' as const,
        description: 'Timeout in milliseconds (default: 120000)',
      },
      workdir: {
        type: 'string' as const,
        description: 'Working directory (default: current directory)',
      },
    },
    required: ['command'],
  };
  permission: 'confirm' = 'confirm';

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { command, timeout = 120000, workdir } = call.arguments;
    const cwd = workdir || context.workingDirectory;

    return new Promise((resolve) => {
      const stdoutChunks: Buffer[] = [];
      const stderrChunks: Buffer[] = [];
      let timeoutId: NodeJS.Timeout | null = null;
      let completed = false;

      const options: SpawnOptions = {
        cwd,
        shell: true,
        env: { ...process.env },
      };

      const childProcess = spawn(command, [], options);

      const cleanup = () => {
        if (timeoutId) {
          clearTimeout(timeoutId);
          timeoutId = null;
        }
      };

      const complete = (result: ToolResult) => {
        if (completed) return;
        completed = true;
        cleanup();
        resolve(result);
      };

      timeoutId = setTimeout(() => {
        if (!completed) {
          childProcess.kill('SIGKILL');
          complete({
            success: false,
            output: '',
            error: `Command timed out after ${timeout}ms`,
          });
        }
      }, timeout);

      if (childProcess.stdout) {
        childProcess.stdout.on('data', (data: Buffer) => {
          stdoutChunks.push(data);
        });
      }

      if (childProcess.stderr) {
        childProcess.stderr.on('data', (data: Buffer) => {
          stderrChunks.push(data);
        });
      }

      childProcess.on('error', (error: Error) => {
        complete({
          success: false,
          output: '',
          error: `Failed to execute command: ${error.message}`,
        });
      });

      childProcess.on('close', (code: number | null) => {
        const stdout = Buffer.concat(stdoutChunks).toString('utf-8');
        const stderr = Buffer.concat(stderrChunks).toString('utf-8');
        const output = stdout + (stderr ? `\nStderr:\n${stderr}` : '');

        complete({
          success: code === 0,
          output: output.trim(),
          metadata: {
            exitCode: code,
            cwd,
          },
        });
      });
    });
  }
}
