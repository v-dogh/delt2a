#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

if ! command -v doxygen >/dev/null 2>&1; then
    echo "Doxygen is missing. On Arch: sudo pacman -S doxygen" >&2
    exit 1
fi

# Relative paths in Doxyfile are relative to the working directory.
doxygen docs/LLM/tools/Doxyfile.llm
python3 docs/LLM/tools/doxygen_to_llm.py build/doxygen/xml docs/LLM/api

echo "Generated docs/api/INDEX.md and docs/api/SYMBOLS.jsonl"
echo "Doxygen HTML: build/doxygen/html/index.html"
