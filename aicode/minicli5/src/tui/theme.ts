export interface Theme {
  // Core semantics
  primary: string;
  secondary: string;
  accent: string;
  success: string;
  warning: string;
  error: string;
  info: string;

  // Text styles
  dim: string;
  bold: string;
  italic: string;
  underline: string;
  reset: string;

  // Code
  code: string;
  codeBlock: string;
  codeBorder: string;
  codeLang: string;
  codeLineNo: string;

  // Headings
  heading: string;
  subheading: string;

  // Tools
  toolName: string;
  toolArg: string;
  toolIcon: string;

  // Spinner
  spinner: string;
  spinnerPulse: string[];

  // Prompt
  promptChar: string;
  promptDim: string;

  // Borders & chrome
  border: string;
  borderAccent: string;
  borderDim: string;

  // Misc
  muted: string;
  highlight: string;
  tag: string;
}

const ESC = "\x1b[";

// 256-color foreground
const fg = (n: number) => `${ESC}38;5;${n}m`;
// 256-color background
const bg = (n: number) => `${ESC}48;5;${n}m`;
// RGB foreground (true-color)
const rgb = (r: number, g: number, b: number) => `${ESC}38;2;${r};${g};${b}m`;

function detectDarkMode(): boolean {
  const colorfgbg = process.env.COLORFGBG;
  if (colorfgbg) {
    const parts = colorfgbg.split(";");
    const bgVal = parseInt(parts[parts.length - 1], 10);
    return bgVal < 8;
  }
  return true; // default dark
}

// ─── Dark Theme: "Midnight Neon" ─────────────────────────────────────────────
const DARK: Theme = {
  // Core
  primary:     rgb(100, 180, 255),     // electric blue
  secondary:   fg(245),                // neutral gray
  accent:      rgb(255, 120, 200),     // hot pink
  success:     rgb(80, 250, 160),      // mint green
  warning:     rgb(255, 200, 60),      // amber gold
  error:       rgb(255, 90, 90),       // coral red
  info:        rgb(130, 170, 255),     // soft periwinkle

  // Text styles
  dim:         `${ESC}2m`,
  bold:        `${ESC}1m`,
  italic:      `${ESC}3m`,
  underline:   `${ESC}4m`,
  reset:       `${ESC}0m`,

  // Code
  code:        rgb(255, 210, 140),     // warm peach
  codeBlock:   bg(235),               // near-black bg
  codeBorder:  fg(240),               // subtle gray border
  codeLang:    rgb(150, 130, 255),     // lavender
  codeLineNo:  fg(242),               // dim gray

  // Headings
  heading:     `${ESC}1m${rgb(100, 180, 255)}`, // bold electric blue
  subheading:  `${ESC}1m${fg(146)}`,             // bold muted teal

  // Tools
  toolName:    rgb(180, 140, 255),     // soft purple
  toolArg:     rgb(200, 180, 130),     // sand
  toolIcon:    rgb(255, 200, 60),      // amber

  // Spinner — color pulse cycle (4 frames of gradient)
  spinner:     rgb(100, 180, 255),
  spinnerPulse: [
    rgb(100, 180, 255),  // electric blue
    rgb(130, 160, 255),  // periwinkle shift
    rgb(170, 140, 255),  // lavender
    rgb(200, 130, 255),  // soft violet
    rgb(230, 140, 240),  // orchid
    rgb(200, 130, 255),  // soft violet (return)
    rgb(170, 140, 255),  // lavender (return)
    rgb(130, 160, 255),  // periwinkle (return)
  ],

  // Prompt
  promptChar:  rgb(100, 180, 255),
  promptDim:   fg(242),

  // Borders
  border:      fg(240),
  borderAccent: rgb(100, 180, 255),
  borderDim:   fg(236),

  // Misc
  muted:       fg(244),
  highlight:   rgb(255, 240, 180),     // cream highlight
  tag:         `${bg(236)}${rgb(180, 140, 255)}`, // pill bg + purple text
};

// ─── Light Theme: "Paper Studio" ─────────────────────────────────────────────
const LIGHT: Theme = {
  // Core
  primary:     rgb(30, 100, 200),      // deep blue
  secondary:   fg(242),                // mid gray
  accent:      rgb(190, 50, 120),      // berry
  success:     rgb(20, 140, 80),       // forest green
  warning:     rgb(180, 120, 0),       // dark amber
  error:       rgb(200, 50, 50),       // brick red
  info:        rgb(60, 100, 180),      // steel blue

  // Text styles
  dim:         `${ESC}2m`,
  bold:        `${ESC}1m`,
  italic:      `${ESC}3m`,
  underline:   `${ESC}4m`,
  reset:       `${ESC}0m`,

  // Code
  code:        rgb(160, 90, 20),       // rust orange
  codeBlock:   bg(254),               // near-white bg
  codeBorder:  fg(248),
  codeLang:    rgb(100, 70, 180),      // violet
  codeLineNo:  fg(248),

  // Headings
  heading:     `${ESC}1m${rgb(30, 100, 200)}`,
  subheading:  `${ESC}1m${fg(66)}`,

  // Tools
  toolName:    rgb(100, 60, 170),      // deep purple
  toolArg:     rgb(120, 100, 60),
  toolIcon:    rgb(180, 120, 0),

  spinner:     rgb(30, 100, 200),
  spinnerPulse: [
    rgb(30, 100, 200),
    rgb(50, 90, 180),
    rgb(80, 70, 170),
    rgb(100, 60, 170),
    rgb(120, 60, 160),
    rgb(100, 60, 170),
    rgb(80, 70, 180),
    rgb(50, 90, 180),
  ],

  promptChar:  rgb(30, 100, 200),
  promptDim:   fg(248),

  border:      fg(248),
  borderAccent: rgb(30, 100, 200),
  borderDim:   fg(252),

  muted:       fg(246),
  highlight:   rgb(60, 60, 40),
  tag:         `${bg(254)}${rgb(100, 60, 170)}`,
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
  const val = t[color];
  if (Array.isArray(val)) return text; // skip array fields
  return `${val}${text}${t.reset}`;
}

// ─── Box-drawing helpers ─────────────────────────────────────────────────────

export const BOX = {
  // Single-line
  h: "─", v: "│",
  tl: "┌", tr: "┐", bl: "└", br: "┘",
  lt: "├", rt: "┤", tt: "┬", bt: "┴", cross: "┼",
  // Double-line
  dh: "═", dv: "║",
  dtl: "╔", dtr: "╗", dbl: "╚", dbr: "╝",
  dlt: "╠", drt: "╣", dtt: "╦", dbt: "╩",
  // Rounded
  rtl: "╭", rtr: "╮", rbl: "╰", rbr: "╯",
} as const;

// Create a horizontal rule with optional label
export function hrule(width: number, label?: string): string {
  const t = getTheme();
  if (!label) {
    return `${t.borderDim}${BOX.h.repeat(width)}${t.reset}`;
  }
  const padded = ` ${label} `;
  const left = 2;
  const right = Math.max(0, width - left - padded.length);
  return `${t.borderDim}${BOX.h.repeat(left)}${t.reset}${t.muted}${padded}${t.reset}${t.borderDim}${BOX.h.repeat(right)}${t.reset}`;
}
