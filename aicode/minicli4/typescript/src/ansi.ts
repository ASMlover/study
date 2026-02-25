const RESET = "\x1b[0m";

function wrap(code: string, text: string): string {
  return `${code}${text}${RESET}`;
}

export const ansi = {
  reset: RESET,
  bold: (text: string): string => wrap("\x1b[1m", text),
  dim: (text: string): string => wrap("\x1b[2m", text),
  italic: (text: string): string => wrap("\x1b[3m", text),
  cyan: (text: string): string => wrap("\x1b[36m", text),
  blue: (text: string): string => wrap("\x1b[34m", text),
  green: (text: string): string => wrap("\x1b[32m", text),
  yellow: (text: string): string => wrap("\x1b[33m", text),
  magenta: (text: string): string => wrap("\x1b[35m", text),
  red: (text: string): string => wrap("\x1b[31m", text),
  gray: (text: string): string => wrap("\x1b[90m", text),
  code: (text: string): string => wrap("\x1b[38;5;81m", text)
};

export function stripAnsi(text: string): string {
  return text.replace(/\x1b\[[0-9;]*[A-Za-z]/g, "");
}
