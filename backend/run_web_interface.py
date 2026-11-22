"""Start the paper research API. Run from any directory."""

import sys
from pathlib import Path

import uvicorn

sys.path.insert(0, str(Path(__file__).resolve().parent))
if __name__ == "__main__":
    uvicorn.run("data_service.web.api_server:app", host="127.0.0.1", port=8000)
