#!/usr/bin/env bash
set -e

OS="$(uname -s)"
SERVICE="$1"

if [ -z "$SERVICE" ]; then
  echo "Usage: $0 <service>"
  exit 1
fi

SERVICE_NAME="$SERVICE"
SERVICE_DIR="services/$SERVICE"
SERVICE_FILE="$SERVICE_DIR/$SERVICE.service"
BINARY_FILE="$SERVICE_DIR/$SERVICE"

require_root() {
  if [ "$EUID" -ne 0 ]; then
    echo "Please run as root."
    exit 1
  fi
}

build_service() {
  echo "Preparing $SERVICE_NAME..."

  if [ -x "$BINARY_FILE" ]; then
    echo "Executable already exists. Skipping build."
    return
  fi

  if [ -f "$SERVICE_DIR/go.mod" ]; then
    (cd "$SERVICE_DIR" && go build -o "$SERVICE_NAME")

  elif [ -f "$SERVICE_DIR/Makefile" ]; then
    (cd "$SERVICE_DIR" && make)

  elif [ -f "$BINARY_FILE" ]; then
    chmod +x "$BINARY_FILE"

  else
    echo "No executable or build system found for $SERVICE_NAME."
    exit 1
  fi

  if [ ! -x "$BINARY_FILE" ]; then
    echo "Executable not created."
    exit 1
  fi
}

install_linux() {
  echo "Linux detected."

  if [ ! -f "$SERVICE_FILE" ]; then
    echo "Service file not found: $SERVICE_FILE"
    exit 1
  fi

  mkdir -p /usr/local/sbin

  echo "Installing binary."
  install -m 755 "$BINARY_FILE" \
    "/usr/local/sbin/$SERVICE_NAME"

  echo "Installing systemd service."
  install -m 644 "$SERVICE_FILE" \
    "/etc/systemd/system/$SERVICE_NAME.service"

  systemctl daemon-reload
  systemctl enable "$SERVICE_NAME"
}

install_mac() {
  echo "MacOS detected."
  echo "Installing binary only."

  mkdir -p /usr/local/sbin

  install -m 755 "$BINARY_FILE" \
    "/usr/local/sbin/$SERVICE_NAME"
}

require_root

if [ "$OS" = "Linux" ] && command -v systemctl >/dev/null 2>&1; then
  build_service
  install_linux
elif [ "$OS" = "Darwin" ]; then
  build_service
  install_mac
else
  echo "Operating System is unsupported"
  exit 1
fi

echo "$SERVICE installation complete."
