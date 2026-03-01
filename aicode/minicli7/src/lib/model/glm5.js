export class Glm5Client {
  constructor(config) {
    this.config = config;
  }

  async streamChat({ messages, tools, onThinking, onToken }) {
    if (!this.config.apiKey) {
      throw new Error(
        `Missing API key. Set ${this.config.apiKeyEnv} or apiKey in ${this.config.configPath}`
      );
    }

    const body = {
      model: this.config.model,
      messages,
      stream: true,
      tools,
      tool_choice: "auto",
    };

    const res = await fetch(this.config.apiBaseUrl, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${this.config.apiKey}`,
      },
      body: JSON.stringify(body),
    });

    if (!res.ok || !res.body) {
      const text = await res.text();
      throw new Error(`GLM request failed: ${res.status} ${text}`);
    }

    const reader = res.body.getReader();
    const decoder = new TextDecoder("utf-8");

    let buf = "";
    let content = "";
    const toolCallByIndex = new Map();
    let emittedThinking = false;

    while (true) {
      const { done, value } = await reader.read();
      if (done) {
        break;
      }
      buf += decoder.decode(value, { stream: true });
      const split = buf.split("\n");
      buf = split.pop() ?? "";

      for (const lineRaw of split) {
        const line = lineRaw.trim();
        if (!line.startsWith("data:")) {
          continue;
        }
        const payload = line.slice(5).trim();
        if (payload === "[DONE]") {
          continue;
        }

        let json;
        try {
          json = JSON.parse(payload);
        } catch {
          continue;
        }

        const delta = json?.choices?.[0]?.delta ?? {};

        if (!emittedThinking) {
          emittedThinking = true;
          onThinking?.();
        }

        if (delta.content) {
          content += delta.content;
          onToken?.(delta.content);
        }

        if (Array.isArray(delta.tool_calls)) {
          for (const item of delta.tool_calls) {
            const key = item.index ?? 0;
            if (!toolCallByIndex.has(key)) {
              toolCallByIndex.set(key, {
                id: item.id || `tool_${key}`,
                type: "function",
                function: {
                  name: item.function?.name || "",
                  arguments: item.function?.arguments || "",
                },
              });
            } else {
              const current = toolCallByIndex.get(key);
              if (item.id) {
                current.id = item.id;
              }
              if (item.function?.name) {
                current.function.name += item.function.name;
              }
              if (item.function?.arguments) {
                current.function.arguments += item.function.arguments;
              }
            }
          }
        }
      }
    }

    return {
      content,
      toolCalls: [...toolCallByIndex.values()],
    };
  }
}
