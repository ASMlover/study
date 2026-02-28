import type { Provider, Message, SubAgentRequest, SubAgentResult, AppConfig, ToolContext } from "../types.js";
import { ToolRegistry } from "../tools/registry.js";
import { runCoder } from "./coder.js";
import { log } from "../utils/logger.js";

const SUB_AGENT_TOOLS: Record<string, string[]> = {
  explore: ["read_file", "list_dir", "grep", "glob", "project_tree"],
  plan: ["read_file", "list_dir", "grep", "glob", "project_tree"],
  general: ["read_file", "write_file", "list_dir", "grep", "shell", "glob", "project_tree"],
};

const SUB_AGENT_SYSTEMS: Record<string, string> = {
  explore: "You are an explorer sub-agent. Your job is to investigate the codebase and return findings. You have read-only access. Be thorough but concise in your report.",
  plan: "You are a planning sub-agent. Analyze the codebase and create an implementation plan. You have read-only access. Return a clear, actionable plan.",
  general: "You are a general-purpose sub-agent. Complete the assigned task using available tools. Be efficient and report results clearly.",
};

export async function spawnSubAgent(
  req: SubAgentRequest,
  provider: Provider,
  toolRegistry: ToolRegistry,
  config: AppConfig,
  projectRoot: string,
): Promise<SubAgentResult> {
  const allowedTools = req.tools ?? SUB_AGENT_TOOLS[req.type] ?? SUB_AGENT_TOOLS.general;
  const systemPrompt = SUB_AGENT_SYSTEMS[req.type] ?? SUB_AGENT_SYSTEMS.general;
  const maxTurns = req.maxTurns ?? 5;

  const messages: Message[] = [
    { role: "system", content: systemPrompt },
    { role: "user", content: req.prompt },
  ];

  const toolCtx: ToolContext = { projectRoot };
  const toolDefs = toolRegistry.getDefinitions(allowedTools);

  log("info", "Sub-agent spawned", { type: req.type, prompt: req.prompt.slice(0, 100) });

  const result = await runCoder(
    provider,
    messages,
    toolDefs,
    toolRegistry,
    toolCtx,
    maxTurns,
    config.model,
  );

  // Extract the final assistant message content
  const lastAssistant = [...result.messages].reverse().find(m => m.role === "assistant" && m.content);
  const output = lastAssistant?.content ?? "(no output)";

  log("info", "Sub-agent completed", { type: req.type, outputLen: output.length });

  return {
    output,
    tokenUsage: result.tokenUsage,
  };
}

export async function spawnSubAgents(
  requests: SubAgentRequest[],
  provider: Provider,
  toolRegistry: ToolRegistry,
  config: AppConfig,
  projectRoot: string,
): Promise<SubAgentResult[]> {
  return Promise.all(
    requests.map(req => spawnSubAgent(req, provider, toolRegistry, config, projectRoot))
  );
}
