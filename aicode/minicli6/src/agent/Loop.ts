import { Agent, AgentEvent } from './Agent.js';

export class AgentLoop {
  private agent: Agent;
  private running: boolean = false;
  private abortController: AbortController | null = null;

  constructor(agent: Agent) {
    this.agent = agent;
  }

  async *run(userMessage: string): AsyncGenerator<AgentEvent> {
    if (this.running) {
      yield { type: 'error', error: new Error('Agent loop is already running') };
      return;
    }

    this.running = true;
    this.abortController = new AbortController();

    try {
      for await (const event of this.agent.sendMessage(userMessage)) {
        if (this.abortController.signal.aborted) {
          break;
        }

        yield event;

        if (!this.shouldContinue(event)) {
          break;
        }
      }
    } finally {
      this.running = false;
      this.abortController = null;
    }
  }

  stop(): void {
    this.running = false;
    if (this.abortController) {
      this.abortController.abort();
    }
    this.agent.stop();
  }

  isRunning(): boolean {
    return this.running;
  }

  private shouldContinue(event: AgentEvent): boolean {
    switch (event.type) {
      case 'done':
        return false;
      case 'error':
        return false;
      case 'tool_result':
        return true;
      default:
        return true;
    }
  }
}
