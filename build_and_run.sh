#!/usr/bin/env sh

set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/project"
BUILD_DIR="$PROJECT_DIR/build"
BIN_DIR="$BUILD_DIR/bin"

echo "==> Configuring (Qt 5.15 must be discoverable by CMake)"
if [ -d "$BUILD_DIR" ]; then
    echo "==> Cleaning previous build directory"
    rm -rf "$BUILD_DIR"
fi
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"

echo "==> Building"
cmake --build "$BUILD_DIR"

SERVER_BIN="$BIN_DIR/qt-server"
CLIENT_BIN="$BIN_DIR/qt-client"

if [[ ! -x "$SERVER_BIN" ]]; then
    echo "Ошибка: не найден исполняемый файл сервера $SERVER_BIN" >&2
    exit 1
fi

if [[ ! -x "$CLIENT_BIN" ]]; then
    echo "Ошибка: не найден исполняемый файл клиента $CLIENT_BIN" >&2
    exit 1
fi

echo "==> Запуск сервера (оставьте окно открытым для работы)"
"$SERVER_BIN" &
SERVER_PID=$!

cleanup() {
    if [ -n "${SERVER_PID:-}" ] && ps -p "$SERVER_PID" >/dev/null 2>&1; then
        echo "==> Остановка сервера"
        kill "$SERVER_PID" >/dev/null 2>&1 || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}

trap cleanup INT TERM EXIT

sleep 1
echo "==> Запуск клиента"
"$CLIENT_BIN"

wait "$SERVER_PID" 2>/dev/null || true

