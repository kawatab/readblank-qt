#!/bin/sh

PIPER_PYTHON="$HOME/.local/pipx/venvs/piper-tts/bin/python"
MODEL_DIR="$HOME/Models/Piper"

exec "$PIPER_PYTHON" \
    -m piper.http_server \
    --model en_US-lessac-medium \
    --data-dir "$MODEL_DIR" \
    --host 127.0.0.1 \
    --port 5000
