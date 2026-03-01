function estimateTokensForMessage(msg) {
  const text = JSON.stringify(msg);
  return Math.ceil(text.length / 4);
}

export function estimateContextTokens(messages) {
  return messages.reduce((sum, m) => sum + estimateTokensForMessage(m), 0);
}

export function compactContext(messages, keepRecent = 12) {
  if (messages.length <= keepRecent + 2) {
    return messages;
  }

  const recent = messages.slice(-keepRecent);
  const older = messages.slice(0, -keepRecent);

  const summaryLines = older
    .filter((m) => m.role !== "system")
    .slice(-30)
    .map((m) => {
      const content = typeof m.content === "string" ? m.content : JSON.stringify(m.content);
      return `[${m.role}] ${content.slice(0, 160).replace(/\s+/g, " ")}`;
    });

  const summaryMessage = {
    role: "system",
    content: `Compacted session summary:\n${summaryLines.join("\n")}`,
  };

  const systems = messages.filter((m) => m.role === "system").slice(0, 1);
  return [...systems, summaryMessage, ...recent.filter((m) => m.role !== "system")];
}
