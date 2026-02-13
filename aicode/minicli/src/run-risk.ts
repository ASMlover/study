export type RunCommandRiskLevel = "low" | "medium" | "high";

export interface RunRiskAssessment {
  level: RunCommandRiskLevel;
  matchedRules: string[];
}

export const HIGH_RISK_COMMAND_BLACKLIST = [
  "rm",
  "del",
  "erase",
  "rmdir",
  "format",
  "mkfs",
  "shutdown",
  "reboot",
  "init",
  "poweroff",
  "chmod",
  "chown",
  "takeown",
  "icacls",
  "mv",
  "move",
  "cp",
  "copy",
  "reg",
  "sc"
] as const;

export const MEDIUM_RISK_OPERATOR_RULES = [
  "&&",
  "||",
  "|",
  ";",
  "$(",
  "`",
  ">",
  "<"
] as const;

function escapeRegExp(source: string): string {
  return source.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

function buildBypassRegex(token: string): RegExp {
  const sequence = token
    .split("")
    .map((char) => escapeRegExp(char))
    .join("[\\s\"'`\\\\]*");
  return new RegExp(`(^|[^a-z0-9])${sequence}([^a-z0-9]|$)`, "i");
}

export function classifyRunCommandRisk(command: string): RunRiskAssessment {
  const normalized = command.trim();
  if (normalized.length === 0) {
    return {
      level: "low",
      matchedRules: []
    };
  }

  const highMatched = HIGH_RISK_COMMAND_BLACKLIST.filter((token) =>
    buildBypassRegex(token).test(normalized)
  );
  if (highMatched.length > 0) {
    return {
      level: "high",
      matchedRules: highMatched
    };
  }

  const mediumMatched = MEDIUM_RISK_OPERATOR_RULES.filter((rule) =>
    normalized.includes(rule)
  );
  if (mediumMatched.length > 0) {
    return {
      level: "medium",
      matchedRules: mediumMatched
    };
  }

  return {
    level: "low",
    matchedRules: []
  };
}
