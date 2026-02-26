from __future__ import annotations

import sys

from .cli import run_cli


def main() -> None:
    raise SystemExit(run_cli(sys.argv[1:]))


if __name__ == "__main__":
    main()
