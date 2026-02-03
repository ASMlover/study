#!/usr/bin/env bash
set -euo pipefail

ACTION="${1:-}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PID_FILE="$ROOT_DIR/run/hotfix_server.pid"
LOG_FILE="${LOG_FILE:-$ROOT_DIR/run/hotfix_server.log}"
PYTHON_BIN="${PYTHON_BIN:-python3}"

usage() {
  echo "Usage: ./run.sh <start|stop|restart|status|logs>"
}

ensure_run_dir() {
  mkdir -p "$ROOT_DIR/run"
}

start_server() {
  ensure_run_dir
  if [[ -f "$PID_FILE" ]]; then
    if ps -p "$(cat "$PID_FILE")" > /dev/null 2>&1; then
      echo "HotfixServer is already running. PID=$(cat "$PID_FILE")"
      return 0
    fi
  fi
  nohup "$PYTHON_BIN" "$ROOT_DIR/hotfix_server.py" "$@" --log-file "$LOG_FILE" \
    > /dev/null 2>&1 &
  echo $! > "$PID_FILE"
  echo "HotfixServer started. PID=$(cat "$PID_FILE")"
}

stop_server() {
  if [[ ! -f "$PID_FILE" ]]; then
    echo "PID file not found."
    return 1
  fi
  PID="$(cat "$PID_FILE")"
  if [[ -z "$PID" ]]; then
    echo "PID file is empty."
    return 1
  fi
  if ps -p "$PID" > /dev/null 2>&1; then
    kill "$PID"
    rm -f "$PID_FILE"
    echo "HotfixServer stopped. PID=$PID"
    return 0
  fi
  echo "HotfixServer not running (stale pid file)."
  rm -f "$PID_FILE"
  return 1
}

status_server() {
  if [[ ! -f "$PID_FILE" ]]; then
    echo "HotfixServer status: stopped (pid file missing)"
    return 1
  fi
  PID="$(cat "$PID_FILE")"
  if [[ -z "$PID" ]]; then
    echo "HotfixServer status: unknown (pid file empty)"
    return 1
  fi
  if ps -p "$PID" > /dev/null 2>&1; then
    echo "HotfixServer status: running (PID=$PID)"
    return 0
  fi
  echo "HotfixServer status: stopped (stale pid file)"
  return 1
}

logs_server() {
  ensure_run_dir
  if [[ ! -f "$LOG_FILE" ]]; then
    echo "Log file not found: $LOG_FILE"
    return 1
  fi
  tail -n "${LINES:-200}" -f "$LOG_FILE"
}

restart_server() {
  stop_server || true
  sleep 0.2
  start_server "${@:2}"
}

case "$ACTION" in
  start)
    start_server "${@:2}"
    ;;
  stop)
    stop_server
    ;;
  restart)
    restart_server "$@"
    ;;
  status)
    status_server
    ;;
  logs)
    logs_server
    ;;
  *)
    usage
    exit 1
    ;;
esac
