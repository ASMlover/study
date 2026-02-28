import type { ToolDefinition } from "../types.js";

export const TOOL_DEFINITIONS: ToolDefinition[] = [
  {
    type: "function",
    function: {
      name: "read_file",
      description: "Read the contents of a file at the given path. Returns the file content as text.",
      parameters: {
        type: "object",
        properties: {
          path: { type: "string", description: "Path to the file to read (relative to project root)" },
          offset: { type: "string", description: "Line number to start reading from (1-based)" },
          limit: { type: "string", description: "Maximum number of lines to read" },
        },
        required: ["path"],
      },
    },
  },
  {
    type: "function",
    function: {
      name: "write_file",
      description: "Write content to a file. Creates the file if it doesn't exist, overwrites if it does.",
      parameters: {
        type: "object",
        properties: {
          path: { type: "string", description: "Path to the file to write (relative to project root)" },
          content: { type: "string", description: "The content to write to the file" },
        },
        required: ["path", "content"],
      },
    },
  },
  {
    type: "function",
    function: {
      name: "list_dir",
      description: "List the contents of a directory. Returns filenames with type indicators (/ for dirs).",
      parameters: {
        type: "object",
        properties: {
          path: { type: "string", description: "Path to the directory (relative to project root, default: '.')" },
        },
        required: [],
      },
    },
  },
  {
    type: "function",
    function: {
      name: "grep",
      description: "Search file contents using a regex pattern. Returns matching lines with file paths and line numbers.",
      parameters: {
        type: "object",
        properties: {
          pattern: { type: "string", description: "Regular expression pattern to search for" },
          path: { type: "string", description: "Directory or file to search in (default: '.')" },
          glob: { type: "string", description: "File glob pattern to filter files (e.g., '*.ts')" },
        },
        required: ["pattern"],
      },
    },
  },
  {
    type: "function",
    function: {
      name: "shell",
      description: "Execute a shell command and return its output. Use for system commands, git, npm, etc.",
      parameters: {
        type: "object",
        properties: {
          command: { type: "string", description: "The shell command to execute" },
          timeout: { type: "string", description: "Timeout in milliseconds (default: 30000)" },
        },
        required: ["command"],
      },
    },
  },
  {
    type: "function",
    function: {
      name: "glob",
      description: "Find files matching a glob pattern. Returns matching file paths.",
      parameters: {
        type: "object",
        properties: {
          pattern: { type: "string", description: "Glob pattern (e.g., '**/*.ts', 'src/**/*.json')" },
          path: { type: "string", description: "Base directory to search from (default: '.')" },
        },
        required: ["pattern"],
      },
    },
  },
  {
    type: "function",
    function: {
      name: "project_tree",
      description: "Show a tree view of the project structure. Returns an indented directory tree.",
      parameters: {
        type: "object",
        properties: {
          path: { type: "string", description: "Root directory for the tree (default: '.')" },
          depth: { type: "string", description: "Maximum depth to traverse (default: 3)" },
        },
        required: [],
      },
    },
  },
];
