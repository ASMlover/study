export interface Theme {
  primary: string;
  secondary: string;
  success: string;
  warning: string;
  error: string;
  dim: string;
  bold: string;
  reset: string;
  code: string;
  codeBlock: string;
  heading: string;
  toolName: string;
  toolArg: string;
  spinner: string;
}

const ESC = "\x1b[";

function detectDarkMode(): boolean {
  const colorfgbg = process.env.COLORFGBG;
  if (colorfgbg) {
    const parts = colorfgbg.split(";");
    const bg = parseInt(parts[parts.length - 1], 10);
    return bg < 8;
  }
  return true; // default dark
}

const DARK: Theme = {
  primary:   `${ESC}38;5;75m`,    // blue
  secondary: `${ESC}38;5;245m`,   // gray
  success:   `${ESC}38;5;114m`,   // green
  warning:   `${ESC}38;5;221m`,   // yellow
  error:     `${ESC}38;5;203m`,   // red
  dim:       `${ESC}2m`,
  bold:      `${ESC}1m`,
  reset:     `${ESC}0m`,
  code:      `${ESC}38;5;223m`,   // light orange
  codeBlock: `${ESC}48;5;236m`,   // dark bg
  heading:   `${ESC}1;38;5;75m`,  // bold blue
  toolName:  `${ESC}38;5;141m`,   // purple
  toolArg:   `${ESC}38;5;180m`,   // tan
  spinner:   `${ESC}38;5;75m`,    // blue
};

const LIGHT: Theme = {
  primary:   `${ESC}38;5;27m`,
  secondary: `${ESC}38;5;242m`,
  success:   `${ESC}38;5;28m`,
  warning:   `${ESC}38;5;130m`,
  error:     `${ESC}38;5;160m`,
  dim:       `${ESC}2m`,
  bold:      `${ESC}1m`,
  reset:     `${ESC}0m`,
  code:      `${ESC}38;5;130m`,
  codeBlock: `${ESC}48;5;254m`,
  heading:   `${ESC}1;38;5;27m`,
  toolName:  `${ESC}38;5;91m`,
  toolArg:   `${ESC}38;5;94m`,
  spinner:   `${ESC}38;5;27m`,
};

let currentTheme: Theme | null = null;

export function getTheme(): Theme {
  if (!currentTheme) {
    currentTheme = detectDarkMode() ? DARK : LIGHT;
  }
  return currentTheme;
}

export function colorize(color: keyof Theme, text: string): string {
  const t = getTheme();
  return `${t[color]}${text}${t.reset}`;
}
