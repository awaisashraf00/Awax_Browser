#!/bin/sh
set -eu

PROJECT_DIR=$(CDPATH= cd "$(dirname "$0")" && pwd)
BUILD_DIR="$PROJECT_DIR/build"
APP="$BUILD_DIR/bin/Awax_browser"

if [ "$#" -gt 1 ]; then
    echo "Usage: $0 [0|1|other-single-argument]" >&2
    exit 2
fi

if [ "${1-0}" = "1" ]; then
    echo "Removing the previous build..."
    rm -rf "$BUILD_DIR"
fi

echo "Configuring the project..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"

echo "Building the project..."
cmake --build "$BUILD_DIR" --parallel

if [ ! -x "$APP" ]; then
    echo "Error: expected executable was not created: $APP" >&2
    exit 1
fi

echo "Starting Awax Browser..."
exec "$APP"