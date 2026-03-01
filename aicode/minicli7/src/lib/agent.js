import { ensureProjectConfigTemplate } from "./config.js";
import { SessionStore } from "./store.js";
import { TaskManager } from "./tasks.js";
import { BackgroundManager } from "./background.js";
import { SkillManager } from "./skills.js";
import { compactContext, estimateContextTokens } from "./context.js";
import { buildToolSchemas, parseToolArguments, ToolExecutor } from "./tools.js";
import { Glm5Client } from "./model/glm5.js";
import { SubAgentRunner } from "./subagent.js";

export class Agent {
  constructor({ cwd, config, ui }) {
    this.cwd = cwd;
    this.config = config;
    this.ui = ui;

    this.store = new SessionStore();
    this.tasks = new TaskManager();
    this.background = new BackgroundManager(cwd);
    this.skills = new SkillManager(cwd);

    this.toolSchemas = buildToolSchemas();

    this.modelClient = new Glm5Client(config);
    this.subAgentRunner = new SubAgentRunner({
      modelClient: this.modelClient,
      tools: this.toolSchemas,
      baseMessages: this.baseSystemMessages(),
    });

    this.toolExecutor = new ToolExecutor({
      cwd,
      background: this.background,
      store: this.store,
      subAgentRunner: this.subAgentRunner,
    });

    for (const m of this.baseSystemMessages()) {
      this.store.addMessage(m);
    }
  }

  baseSystemMessages() {
    return [
      {
        role: "system",
        content:
          "You are minicli7 agent. Be precise, execute via tools when needed, ask for permission before any tool action, and keep outputs concise.",
      },
    ];
  }

  async runInteractive() {
    await ensureProjectConfigTemplate(this.cwd);

    while (true) {
      const input = (await this.ui.prompt("you> ")).trim();
      if (!input) {
        continue;
      }

      if (input.startsWith("/")) {
        const shouldContinue = await this.handleCommand(input);
        if (!shouldContinue) {
          this.ui.close();
          return;
        }
      } else {
        await this.handleUserMessage(input);
      }
    }
  }

  async handleCommand(raw) {
    const [cmd, ...rest] = raw.split(" ");

    if (cmd === "/exit" || cmd === "/quit") {
      return false;
    }

    if (cmd === "/help") {
      this.ui.printHelp();
      return true;
    }

    if (cmd === "/clear") {
      console.clear();
      return true;
    }

    if (cmd === "/compact") {
      this.applyCompaction(true);
      this.ui.notify("Context compacted.", "ok");
      return true;
    }

    if (cmd === "/status" || cmd === "/context") {
      this.showContextStatus();
      return true;
    }

    if (cmd === "/agent") {
      const goal = rest.join(" ").trim();
      if (!goal) {
        this.ui.error("Usage: /agent <goal>");
        return true;
      }
      const approved = await this.ui.confirm(`Run sub-agent for goal: ${goal}?`);
      if (!approved) {
        this.ui.notify("Cancelled.", "warn");
        return true;
      }
      this.ui.startSpinner("sub-agent working");
      try {
        const report = await this.subAgentRunner.run(goal, 4);
        this.ui.stopSpinner();
        this.ui.notify(`sub-agent> ${report}`, "tool");
      } catch (err) {
        this.ui.stopSpinner();
        this.ui.error(err.message);
      }
      return true;
    }

    if (cmd === "/todo") {
      await this.handleTodoCommand(rest);
      return true;
    }

    if (cmd === "/bg") {
      await this.handleBackgroundCommand(rest);
      return true;
    }

    if (cmd === "/skills") {
      await this.handleSkillsCommand(rest);
      return true;
    }

    if (cmd === "/task") {
      await this.handleTaskCommand(rest);
      return true;
    }

    this.ui.error(`Unknown command: ${cmd}. Try /help`);
    return true;
  }

  async handleTodoCommand(args) {
    const sub = args[0] || "list";

    if (sub === "add") {
      const text = args.slice(1).join(" ").trim();
      if (!text) {
        this.ui.error("Usage: /todo add <text>");
        return;
      }
      const todo = this.store.addTodo(text);
      this.ui.notify(`Added todo #${todo.id}: ${todo.text}`, "ok");
      return;
    }

    if (sub === "done") {
      const id = Number(args[1]);
      if (!id) {
        this.ui.error("Usage: /todo done <id>");
        return;
      }
      const todo = this.store.markTodoDone(id);
      this.ui.notify(todo ? `Done todo #${todo.id}` : "Todo not found", todo ? "ok" : "warn");
      return;
    }

    const todos = this.store.listTodos();
    if (!todos.length) {
      this.ui.notify("No todos.", "warn");
      return;
    }
    for (const t of todos) {
      this.ui.notify(`${t.done ? "[x]" : "[ ]"} #${t.id} ${t.text}`);
    }
  }

  async handleBackgroundCommand(args) {
    const sub = args[0];
    if (sub === "run") {
      const command = args.slice(1).join(" ").trim();
      if (!command) {
        this.ui.error("Usage: /bg run <shell-command>");
        return;
      }
      const approved = await this.ui.confirm(`Allow background shell command? ${command}`);
      if (!approved) {
        this.ui.notify("Cancelled.", "warn");
        return;
      }
      const job = this.background.start(command);
      this.ui.notify(`Started job #${job.id} pid=${job.pid}`, "ok");
      return;
    }

    const jobs = this.background.list();
    if (!jobs.length) {
      this.ui.notify("No background jobs.", "warn");
      return;
    }
    for (const j of jobs) {
      this.ui.notify(`#${j.id} ${j.status} pid=${j.pid} cmd=${j.command}`, "task");
    }
  }

  async handleSkillsCommand(args) {
    const sub = args[0] || "list";

    if (sub === "list") {
      const names = await this.skills.listSkills();
      if (!names.length) {
        this.ui.notify("No skills found under .minicli/skills", "warn");
        return;
      }
      for (const name of names) {
        this.ui.notify(`- ${name}`);
      }
      return;
    }

    if (sub === "show") {
      const name = args[1];
      if (!name) {
        this.ui.error("Usage: /skills show <name>");
        return;
      }
      try {
        const skill = await this.skills.getSkill(name);
        this.ui.notify(`skill ${name} (${skill.file})\n${skill.content}`);
      } catch {
        this.ui.error(`Skill not found: ${name}`);
      }
      return;
    }

    if (sub === "run") {
      const name = args[1];
      const input = args.slice(2).join(" ").trim();
      if (!name || !input) {
        this.ui.error("Usage: /skills run <name> <input>");
        return;
      }
      try {
        const prompt = await this.skills.buildSkillPrompt(name, input);
        await this.handleUserMessage(prompt);
      } catch {
        this.ui.error(`Skill not found: ${name}`);
      }
      return;
    }

    this.ui.error("Usage: /skills list|show|run");
  }

  async handleTaskCommand(args) {
    const sub = args[0] || "list";

    if (sub === "add") {
      const line = args.slice(1).join(" ");
      if (!line.trim()) {
        this.ui.error("Usage: /task add <title> [--deps=1,2] [--goal=text]");
        return;
      }

      const depsMatch = line.match(/--deps=([^\s]+)/);
      const goalMatch = line.match(/--goal=(.+)$/);
      const title = line
        .replace(/--deps=[^\s]+/, "")
        .replace(/--goal=.+$/, "")
        .trim();
      const deps = depsMatch
        ? depsMatch[1]
            .split(",")
            .map((v) => Number(v.trim()))
            .filter(Boolean)
        : [];
      const goal = goalMatch ? goalMatch[1].trim() : "";

      const task = this.tasks.addTask({ title, deps, goal });
      this.ui.notify(`Added task #${task.id}: ${task.title}`, "ok");
      return;
    }

    if (sub === "run") {
      await this.runTaskOrchestration();
      return;
    }

    const list = this.tasks.list();
    if (!list.length) {
      this.ui.notify("No tasks.", "warn");
      return;
    }
    for (const t of list) {
      this.ui.notify(`#${t.id} [${t.status}] deps=[${t.deps.join(",")}] ${t.title}`, "task");
    }
  }

  async runTaskOrchestration() {
    let progressed = false;

    while (true) {
      const ready = this.tasks.getReadyTasks();
      if (!ready.length) {
        break;
      }
      progressed = true;

      for (const task of ready) {
        this.tasks.updateStatus(task.id, "running");
        this.ui.notify(`Running task #${task.id}: ${task.title}`, "task");

        try {
          if (task.goal) {
            const approved = await this.ui.confirm(`Allow sub-agent for task #${task.id}?`);
            if (!approved) {
              this.tasks.updateStatus(task.id, "failed");
              this.ui.notify(`Task #${task.id} cancelled by user`, "warn");
              continue;
            }
            const result = await this.subAgentRunner.run(task.goal, 4);
            this.ui.notify(`Task #${task.id} result: ${result.slice(0, 400)}`, "tool");
          }
          this.tasks.updateStatus(task.id, "done");
        } catch (err) {
          this.tasks.updateStatus(task.id, "failed");
          this.ui.error(`Task #${task.id} failed: ${err.message}`);
        }
      }
    }

    if (!progressed) {
      this.ui.notify("No runnable tasks. Check dependencies/status.", "warn");
    }
  }

  applyCompaction(force = false) {
    const tokenEstimate = estimateContextTokens(this.store.messages);
    const shouldCompact = force || (this.config.autoCompact && tokenEstimate > this.config.maxContextTokens);
    if (!shouldCompact) {
      return;
    }

    this.store.messages = compactContext(this.store.messages, this.config.compactKeepRecentMessages);
  }

  showContextStatus() {
    const used = estimateContextTokens(this.store.messages);
    const max = Number(this.config.maxContextTokens || 0);
    const pct = max > 0 ? Math.min(999, (used / max) * 100) : 0;
    const remaining = max > 0 ? max - used : 0;
    const messageCount = this.store.messages.length;

    this.ui.notify(
      `Context: ${used}/${max} tokens (${pct.toFixed(1)}%), remaining ${remaining}, messages ${messageCount}, autoCompact=${this.config.autoCompact ? "on" : "off"}`
    );
  }

  async handleUserMessage(input) {
    this.store.addMessage({ role: "user", content: input });
    this.applyCompaction(false);

    await this.loopModelAndTools(0);
  }

  async loopModelAndTools(depth) {
    if (depth > 6) {
      this.ui.error("Exceeded tool loop depth.");
      return;
    }

    this.ui.startSpinner("assistant thinking");
    let response;
    try {
      response = await this.modelClient.streamChat({
        messages: this.store.messages,
        tools: this.toolSchemas,
        onThinking: () => {
          this.ui.stopSpinner();
          this.ui.beginAssistantStream();
        },
        onToken: (token) => {
          this.ui.pushAssistantToken(token);
        },
      });
    } catch (err) {
      this.ui.stopSpinner();
      this.ui.endAssistantStream();
      this.ui.error(err.message);
      return;
    }

    this.ui.stopSpinner();
    this.ui.endAssistantStream();

    const assistantMessage = {
      role: "assistant",
      content: response.content || "",
    };

    if (response.toolCalls.length) {
      assistantMessage.tool_calls = response.toolCalls;
    }

    this.store.addMessage(assistantMessage);

    if (!response.toolCalls.length) {
      return;
    }

    for (const tc of response.toolCalls) {
      const args = parseToolArguments(tc.function.arguments);
      const brief = `${tc.function.name} ${JSON.stringify(args).slice(0, 160)}`;
      const approved = await this.ui.confirm(`Allow tool call: ${brief}?`);
      if (!approved) {
        this.store.addMessage({
          role: "tool",
          tool_call_id: tc.id,
          content: "Tool call denied by user.",
        });
        continue;
      }

      try {
        const result = await this.toolExecutor.execute(tc.function.name, args);
        this.ui.notify(`tool:${tc.function.name}> ${String(result).slice(0, 600)}`, "tool");
        this.store.addMessage({
          role: "tool",
          tool_call_id: tc.id,
          content: String(result),
        });
      } catch (err) {
        const text = `Tool execution failed: ${err.message}`;
        this.ui.error(text);
        this.store.addMessage({
          role: "tool",
          tool_call_id: tc.id,
          content: text,
        });
      }
    }

    await this.loopModelAndTools(depth + 1);
  }
}
