import chalk from 'chalk';

export const theme = {
  colors: {
    primary: '#00E5FF',
    secondary: '#FF2A6D',
    accent: '#05FFA1',
    warning: '#FFD600',
    info: '#B967FF',
    dim: '#4A5568',
    text: '#E2E8F0',
    background: '#0D1117',
    success: '#05FFA1',
    error: '#FF2A6D',
    user: '#00E5FF',
    assistant: '#B967FF',
    system: '#FFD600',
    tool: '#FF6B9D',
  },

  gradients: {
    neon: ['#00E5FF', '#B967FF', '#FF2A6D'],
    success: ['#05FFA1', '#00E5FF'],
    warning: ['#FFD600', '#FF6B9D'],
  },

  borders: {
    primary: '═',
    secondary: '─',
    corner: {
      topLeft: '╔',
      topRight: '╗',
      bottomLeft: '╚',
      bottomRight: '╝',
    },
    connector: {
      left: '╠',
      right: '╣',
      top: '╦',
      bottom: '╩',
      cross: '╬',
    },
    vertical: '║',
    thinVertical: '│',
    thinHorizontal: '─',
    doubleVertical: '║',
    doubleHorizontal: '═',
  },

  icons: {
    user: '◈',
    assistant: '◆',
    system: '▸',
    tool: '◇',
    success: '✓',
    error: '✗',
    warning: '!',
    info: '○',
    pending: '○',
    inProgress: '◐',
    completed: '●',
    cancelled: '✕',
    arrow: '→',
    arrowDouble: '»',
    bullet: '▪',
    diamond: '⬡',
    square: '⬢',
    star: '★',
    sparkle: '✦',
    divider: '┈',
    cursor: '▊',
    cursorAlt: '▌',
    progress: {
      empty: '░',
      partial: '▒',
      full: '█',
    },
  },

  spinners: {
    dots: ['⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏'],
    arc: ['◜', '◠', '◝', '◞', '◡', '◟'],
    pulse: ['◉', '◎', '○', '◎'],
    neon: ['⬤', '◯', '⬤', '◯'],
    dots2: ['⣷', '⣯', '⣟', '⡿', '⢿', '⣻', '⣽', '⣾'],
  },
} as const;

export const c = {
  primary: chalk.hex(theme.colors.primary),
  secondary: chalk.hex(theme.colors.secondary),
  accent: chalk.hex(theme.colors.accent),
  warning: chalk.hex(theme.colors.warning),
  info: chalk.hex(theme.colors.info),
  dim: chalk.hex(theme.colors.dim),
  text: chalk.hex(theme.colors.text),
  user: chalk.hex(theme.colors.user),
  assistant: chalk.hex(theme.colors.assistant),
  system: chalk.hex(theme.colors.system),
  tool: chalk.hex(theme.colors.tool),
  success: chalk.hex(theme.colors.success),
  error: chalk.hex(theme.colors.error),
};

export const ui = {
  horizontalLine: (width: number = 60, char: string = theme.borders.thinHorizontal): string => {
    return char.repeat(width);
  },

  box: (
    content: string[],
    options: {
      title?: string;
      borderColor?: string;
      padding?: number;
      width?: number;
    } = {}
  ): string[] => {
    const { title, borderColor = theme.colors.primary, padding = 1, width = 60 } = options;
    const lines: string[] = [];
    const padStr = ' '.repeat(padding);
    const innerWidth = width - 2 - padding * 2;

    const topBorder = title
      ? `${theme.borders.corner.topLeft}${theme.borders.doubleHorizontal}${
          theme.borders.connector.left
        } ${title} ${theme.borders.connector.right}${theme.borders.doubleHorizontal.repeat(
          Math.max(0, innerWidth - title.length - 2)
        )}${theme.borders.corner.topRight}`
      : `${theme.borders.corner.topLeft}${theme.borders.doubleHorizontal.repeat(
          width - 2
        )}${theme.borders.corner.topRight}`;

    lines.push(chalk.hex(borderColor)(topBorder));

    for (const line of content) {
      const paddedLine = line.padEnd(innerWidth);
      lines.push(
        chalk.hex(borderColor)(theme.borders.vertical) +
          padStr +
          paddedLine +
          padStr +
          chalk.hex(borderColor)(theme.borders.vertical)
      );
    }

    lines.push(
      chalk.hex(borderColor)(
        `${theme.borders.corner.bottomLeft}${theme.borders.doubleHorizontal.repeat(
          width - 2
        )}${theme.borders.corner.bottomRight}`
      )
    );

    return lines;
  },

  progressBar: (
    percent: number,
    width: number = 20,
    filled: string = theme.icons.progress.full,
    empty: string = theme.icons.progress.empty
  ): string => {
    const filledCount = Math.round((percent / 100) * width);
    const emptyCount = width - filledCount;
    return filled.repeat(filledCount) + empty.repeat(emptyCount);
  },

  badge: (text: string, color: string = theme.colors.primary): string => {
    return chalk.hex(color)(`[${text}]`);
  },

  keyValue: (key: string, value: string, keyColor: string = theme.colors.dim): string => {
    return chalk.hex(keyColor)(key) + ' ' + value;
  },
};

export const getStatusColor = (status: string): string => {
  switch (status) {
    case 'idle':
      return theme.colors.dim;
    case 'thinking':
      return theme.colors.warning;
    case 'tool_use':
      return theme.colors.info;
    case 'waiting_input':
      return theme.colors.success;
    default:
      return theme.colors.text;
  }
};

export const getTodoStatusColor = (status: string): string => {
  switch (status) {
    case 'pending':
      return theme.colors.dim;
    case 'in_progress':
      return theme.colors.warning;
    case 'completed':
      return theme.colors.success;
    case 'cancelled':
      return theme.colors.error;
    default:
      return theme.colors.text;
  }
};

export const getPriorityColor = (priority: string): string => {
  switch (priority) {
    case 'high':
      return theme.colors.secondary;
    case 'medium':
      return theme.colors.warning;
    case 'low':
      return theme.colors.dim;
    default:
      return theme.colors.text;
  }
};

export const getStatusIcon = (status: string): string => {
  switch (status) {
    case 'pending':
      return theme.icons.pending;
    case 'in_progress':
      return theme.icons.inProgress;
    case 'completed':
      return theme.icons.completed;
    case 'cancelled':
      return theme.icons.cancelled;
    default:
      return theme.icons.bullet;
  }
};

export const getPriorityIcon = (priority: string): string => {
  switch (priority) {
    case 'high':
      return theme.icons.error;
    case 'medium':
      return theme.icons.warning;
    case 'low':
      return theme.icons.info;
    default:
      return theme.icons.bullet;
  }
};

export const getRoleConfig = (role: string) => {
  switch (role) {
    case 'user':
      return {
        color: theme.colors.user,
        icon: theme.icons.user,
        label: 'USER',
      };
    case 'assistant':
      return {
        color: theme.colors.assistant,
        icon: theme.icons.assistant,
        label: 'AI',
      };
    case 'system':
      return {
        color: theme.colors.system,
        icon: theme.icons.system,
        label: 'SYS',
      };
    case 'tool':
      return {
        color: theme.colors.tool,
        icon: theme.icons.tool,
        label: 'TOOL',
      };
    default:
      return {
        color: theme.colors.text,
        icon: theme.icons.bullet,
        label: role.toUpperCase(),
      };
  }
};
