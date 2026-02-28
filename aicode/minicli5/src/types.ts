// ─── LLM Message Types ───────────────────────────────────────────────────────

export type Role = "system" | "user" | "assistant" | "tool";

export interface ToolCall {
  id: string;
  type: "function";
  function: {
    name: string;
    arguments: string;
  };
}

export interface Message {
  role: Role;
  content: string | null;
  tool_calls?: ToolCall[];
  tool_call_id?: string;
  name?: string;
}

// ─── Tool Definitions ────────────────────────────────────────────────────────

export interface ToolParameter {
  type: string;
  description: string;
  enum?: string[];
  items?: { type: string };
  default?: unknown;
}

export interface ToolDefinition {
  type: "function";
  function: {
    name: string;
    description: string;
    parameters: {
      type: "object";
      properties: Record<string, ToolParameter>;
      required: string[];
    };
  };
}

export interface ToolResult {
  success: boolean;
  output: string;
  error?: string;
}

export interface Tool {
  name: string;
  definition: ToolDefinition;
  execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult>;
}

export interface ToolContext {
  projectRoot: string;
  abortSignal?: AbortSignal;
  approve?: (toolName: string, detail: string, context?: string) => Promise<boolean>;
}

// ─── Provider Types ──────────────────────────────────────────────────────────

export interface ChatRequest {
  messages: Message[];
  tools?: ToolDefinition[];
  stream?: boolean;
  temperature?: number;
  max_tokens?: number;
  model?: string;
}

export interface ChatChoice {
  index: number;
  message: Message;
  finish_reason: string | null;
}

export interface ChatResponse {
  id: string;
  choices: ChatChoice[];
  usage?: {
    prompt_tokens: number;
    completion_tokens: number;
    total_tokens: number;
  };
}

export interface StreamDelta {
  role?: Role;
  content?: string;
  tool_calls?: Array<{
    index: number;
    id?: string;
    type?: "function";
    function?: {
      name?: string;
      arguments?: string;
    };
  }>;
}

export interface StreamChunk {
  id: string;
  choices: Array<{
    index: number;
    delta: StreamDelta;
    finish_reason: string | null;
  }>;
  usage?: {
    prompt_tokens: number;
    completion_tokens: number;
    total_tokens: number;
  };
}

export interface Provider {
  chat(req: ChatRequest): Promise<ChatResponse>;
  chatStream(req: ChatRequest): AsyncIterable<StreamChunk>;
}

// ─── Agent Types ─────────────────────────────────────────────────────────────

export type AgentStage = "planning" | "coding" | "reviewing" | "orchestrating";

export interface AgentTurn {
  role: "user" | "assistant";
  content: string;
  toolCalls?: Array<{ name: string; args: Record<string, unknown>; result: ToolResult }>;
  tokenUsage?: { prompt: number; completion: number };
  timestamp: number;
}

export interface AgentContext {
  messages: Message[];
  turns: AgentTurn[];
  stage: AgentStage;
  tokenUsage: { prompt: number; completion: number; total: number };
  abortController: AbortController;
}

// ─── Sub-Agent Types ─────────────────────────────────────────────────────────

export type SubAgentType = "explore" | "plan" | "general";

export interface SubAgentRequest {
  type: SubAgentType;
  prompt: string;
  tools?: string[];    // tool names allowed
  maxTurns?: number;
}

export interface SubAgentResult {
  output: string;
  tokenUsage: { prompt: number; completion: number };
}

// ─── Task Types ──────────────────────────────────────────────────────────────

export type TaskStatus = "pending" | "in_progress" | "completed" | "deleted";

export interface Task {
  id: string;
  subject: string;
  description: string;
  status: TaskStatus;
  owner?: string;
  blockedBy: string[];
  blocks: string[];
  metadata: Record<string, unknown>;
  createdAt: number;
  updatedAt: number;
}

// ─── Skill Types ─────────────────────────────────────────────────────────────

export interface Skill {
  name: string;
  description: string;
  promptTemplate: string;
  toolRestrictions?: string[];
}

// ─── Session Types ───────────────────────────────────────────────────────────

export interface Session {
  id: string;
  name: string;
  turns: AgentTurn[];
  tasks: Task[];
  compactedSummary?: string;
  createdAt: number;
  updatedAt: number;
}

// ─── Config Types ────────────────────────────────────────────────────────────

export interface AppConfig {
  provider: string;
  model: string;
  api_key: string;
  api_base: string;
  max_tokens: number;
  temperature: number;
  max_retries: number;
  stream: boolean;
  max_tool_rounds: number;
  context_max_tokens: number;
  compact_threshold: number;
}
