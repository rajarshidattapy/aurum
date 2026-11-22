#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
PYTHON=${PYTHON:-python3}
cmake -S backend/backend -B backend/backend/build -DPython3_EXECUTABLE="$(command -v "$PYTHON")"
cmake --build backend/backend/build -j2
ctest --test-dir backend/backend/build --output-on-failure --timeout 30
(cd backend && "$PYTHON" -m pytest tests -q)
(cd frontend && pnpm typecheck && pnpm build)
