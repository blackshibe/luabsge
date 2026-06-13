#!/usr/bin/env bash

cd "$(dirname "$0")"

INPUT="${1:-in/import.rbxl}"
OUT="${2:-out}"

if [ ! -f "$INPUT" ]; then
    echo "error: input rbxl not found: $INPUT" >&2
    exit 1
fi

lune run src/export.lua "$INPUT" "$OUT"
lune run src/download.lua "$OUT"
lune run src/decode.lua "$OUT"
