const RESET = "\x1b[0m";

type ThemePalette = {
  cyan: string;
  blue: string;
  green: string;
  yellow: string;
  magenta: string;
  red: string;
  gray: string;
  code: string;
};

export type AnsiThemeName = "dark" | "light";

const THEMES: Record<AnsiThemeName, ThemePalette> = {
  dark: {
    cyan: "\x1b[36m",
    blue: "\x1b[34m",
    green: "\x1b[32m",
    yellow: "\x1b[33m",
    magenta: "\x1b[35m",
    red: "\x1b[31m",
    gray: "\x1b[90m",
    code: "\x1b[38;5;81m"
  },
  light: {
    cyan: "\x1b[36m",
    blue: "\x1b[34m",
    green: "\x1b[32m",
    yellow: "\x1b[38;5;130m",
    magenta: "\x1b[35m",
    red: "\x1b[31m",
    gray: "\x1b[90m",
    code: "\x1b[38;5;24m"
  }
};

let activeTheme: AnsiThemeName = detectAnsiTheme();

function wrap(code: string, text: string): string {
  return `${code}${text}${RESET}`;
}

function palette(): ThemePalette {
  return THEMES[activeTheme];
}

export const ansi = {
  reset: RESET,
  bold: (text: string): string => wrap("\x1b[1m", text),
  dim: (text: string): string => wrap("\x1b[2m", text),
  italic: (text: string): string => wrap("\x1b[3m", text),
  cyan: (text: string): string => wrap(palette().cyan, text),
  blue: (text: string): string => wrap(palette().blue, text),
  green: (text: string): string => wrap(palette().green, text),
  yellow: (text: string): string => wrap(palette().yellow, text),
  magenta: (text: string): string => wrap(palette().magenta, text),
  red: (text: string): string => wrap(palette().red, text),
  gray: (text: string): string => wrap(palette().gray, text),
  code: (text: string): string => wrap(palette().code, text)
};

export function detectAnsiTheme(env: NodeJS.ProcessEnv = process.env): AnsiThemeName {
  const configured = (env.MINICLI4_THEME ?? "").trim().toLowerCase();
  if (configured === "dark" || configured === "light") {
    return configured;
  }

  const colorFgBg = env.COLORFGBG;
  if (colorFgBg) {
    const parts = colorFgBg.split(";").map((v) => Number(v.trim())).filter((v) => Number.isFinite(v));
    const bg = parts.length > 0 ? parts[parts.length - 1] : NaN;
    if (Number.isFinite(bg)) {
      return bg <= 6 ? "dark" : "light";
    }
  }

  return "dark";
}

export function setAnsiTheme(theme: AnsiThemeName): void {
  activeTheme = theme;
}

export function getAnsiTheme(): AnsiThemeName {
  return activeTheme;
}

export function stripAnsi(text: string): string {
  return text.replace(/\x1b\[[0-9;]*[A-Za-z]/g, "");
}
