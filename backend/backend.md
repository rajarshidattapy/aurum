# Backend — Aurum Quantitative Trading System

## Overview

The backend is a two-layer system: a **Python data/analytics layer** (`data_service/`) and a **C++ low-latency execution core** (`backend/`). Both layers work together to deliver a full pipeline from market data ingestion to order execution.

---

## Directory Structure

```
backend/
├── backend/                     # C++ core engine
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── common/types.hpp
│   │   ├── data_loader.hpp
│   │   ├── order_executor.hpp
│   │   ├── risk_manager.hpp
│   │   └── strategy.hpp
│   ├── src/
│   │   ├── main.cpp             # TradingEngine entry point
│   │   ├── data_loader.cpp
│   │   ├── order_executor.cpp
│   │   ├── risk_manager.cpp
│   │   └── strategy.cpp
│   └── tests/
│       ├── test_data_loader.cpp
│       └── test_risk_manager.cpp
│
├── data_service/                # Python analytics & services layer
│   ├── config.py                # API keys & default trading config
│   ├── ai/                      # AI/LLM/NLP modules
│   ├── api/                     # API manager (rate limiting, caching)
│   ├── backtest/                # Backtesting engine & performance analyzer
│   ├── dashboard/               # Streamlit dashboard (charts, widgets)
│   ├── factors/                 # Factor models & stock screening
│   ├── fetchers/                # Market data fetchers
│   ├── ml/                      # ML models & feature engineering
│   ├── processors/              # Data cleaning & processing
│   ├── realtime/                # WebSocket & real-time feeds
│   └── storage/                 # Database & cache managers
│
├── docs/                        # Module-level documentation
├── config.example.json          # Template for API keys
└── data_service.egg-info/       # Python package metadata
```

---

## C++ Core Engine (`backend/`)

The C++ layer handles low-latency order execution and real-time risk monitoring. It runs as a standalone process and operates on a 100ms event loop.

### `TradingEngine` (`src/main.cpp`)

The central class that orchestrates all components:

- `initialize()` — loads `config.json`, sets up logger, instantiates all components, registers OS signal handlers
- `run()` — main loop: `processMarketData()` → `processSignals()` → `updateRiskMetrics()` every 100ms
- `shutdown()` — gracefully stops the order executor and releases resources

### Components

| Component | Header | Responsibility |
|---|---|---|
| `DataLoader` | `data_loader.hpp` | Loads market data per symbol |
| `Strategy` (abstract) | `strategy.hpp` | Emits `Signal` structs (`symbol`, `side`, `strength ∈ [-1,1]`) |
| `MovingAverageStrategy` | `strategy.hpp` | Concrete MA crossover strategy (short/long period) |
| `RiskManager` | `risk_manager.hpp` | Validates orders against risk limits before submission |
| `OrderExecutor` | `order_executor.hpp` | Threaded queue-based order dispatch |
| `Portfolio` | `common/types.hpp` | Tracks open positions and total portfolio value |

### Risk Limits (`RiskManager::RiskLimits`)

```cpp
struct RiskLimits {
    double max_position_size;
    double max_drawdown;
    double max_leverage;
    double daily_loss_limit;
    double position_concentration;
};
```

### Order Execution (`OrderExecutor`)

Runs a dedicated background thread. Orders are submitted via `submitOrder()` and dispatched via an internal `std::queue`. Supports `cancelOrder()` and `getOrderStatus()`.

### Build

```bash
cd backend
mkdir build && cd build
cmake ..
make -j4
```

Requires: C++17 compiler, CMake 3.12+, spdlog.

---

## Python Data Service (`data_service/`)

### Configuration (`config.py`)

```python
BINANCE_API_KEY = "..."
BINANCE_API_SECRET = "..."
USE_BINANCE_US = False          # toggle for US users
DEFAULT_TRADING_PAIRS = ["BTCUSDT", "ETHUSDT", "BNBUSDT"]
DEFAULT_TIMEFRAME = "1h"        # 1m, 5m, 15m, 1h, 4h, 1d, ...
```

External API keys go in `config.json` (copied from `config.example.json`):
```json
{
  "binance":       { "api_key": "...", "secret_key": "..." },
  "openai":        { "api_key": "..." },
  "alpha_vantage": { "api_key": "..." }
}
```

---

### Data Fetchers (`data_service/fetchers/`)

| Module | Source | Notes |
|---|---|---|
| `binance_fetcher.py` | Binance / Binance.US | Crypto OHLCV, current price — no key needed for public endpoints |
| `alpha_vantage_fetcher.py` | Alpha Vantage | Equity data, requires API key |
| `yahoo_fetcher.py` | Yahoo Finance | Equity data, no key required |

---

### API Manager (`data_service/api/api_manager.py`)

`APIManager` — intelligent HTTP client wrapper:

- **Rate limiting** — sliding 1-minute window per endpoint
- **Response caching** — TTL-based in-memory cache, configurable per call
- **Async support** — `make_async_request()` via `aiohttp`
- **Metrics tracking** — per-endpoint `success_count`, `error_count`, average/min/max response times
- `get_performance_metrics()` — returns dict of all endpoint stats
- `clear_cache(endpoint_name?)` — clears all or specific endpoint cache

---

### Data Processing (`data_service/processors/`)

`data_processor.py` — cleans raw market data, handles missing values, normalises OHLCV columns.

### Feature Engineering (`data_service/ml/feature_engineering.py`)

Generates technical indicators and statistical features from OHLCV data for use in ML models.

---

### Machine Learning (`data_service/ml/ml_models.py`)

Three main classes:

#### `PredictionModel` (regression)
Supports: `linear_regression`, `ridge`, `lasso`, `random_forest`, `gradient_boosting`, `svr`, `mlp`, `decision_tree`, `knn`, `ada_boost`, `xgboost`, `lightgbm`

#### `ClassificationModel` (signal generation)
Same algorithm set, classification variants. Supports `predict_proba()`.

#### `MLModelManager`
Registry for multiple models. Key methods:
- `train_model(name, X, y, config?)` → `ModelResult`
- `get_best_model()` — ranks by validation score
- `compare_models()` → DataFrame
- `save_all_models(dir)` / `load_all_models(dir)` — joblib serialisation

All models optionally apply `StandardScaler` and 5-fold cross-validation.

---

### Factor Analysis (`data_service/factors/`)

| Module | Purpose |
|---|---|
| `factor_calculator.py` | Computes momentum, value, quality, size, volatility factors |
| `factor_screener.py` | Multi-factor stock screening / filtering |
| `factor_optimizer.py` | Factor weight optimisation |
| `factor_backtest.py` | Factor-level backtesting |
| `stock_selector.py` | Final stock selection from factor scores |

---

### Backtesting Engine (`data_service/backtest/`)

**`BacktestEngine`** (`backtest_engine.py`):
- `initial_capital` (default `$100,000`), `commission_rate` (default `0.1%`)
- Tracks: `positions`, `trades`, `equity_curve`
- `run_backtest(data, strategy_func, strategy_params?)` — feeds sorted OHLCV data through strategy callable
- `reset()` — resets state for repeated runs

**`PerformanceAnalyzer`** (`performance_analyzer.py`):
- Computes: Sharpe ratio, max drawdown, CAGR, win rate, Calmar ratio, and other standard metrics from the equity curve

---

### AI / LLM / NLP (`data_service/ai/`)

| Module | Purpose |
|---|---|
| `llm_integration.py` | OpenAI GPT wrapper — `analyze_market(factor_data, price_data)` |
| `langchain_agent.py` | LangChain agent for multi-step market reasoning |
| `sentiment_analyzer.py` | Sentiment scoring of text (positive/negative/neutral) |
| `nlp_processor.py` | NLP pipeline — tokenisation, entity extraction |
| `news_processor.py` | Fetches and processes financial news articles |
| `social_media_monitor.py` | Monitors social media for ticker mentions and sentiment |
| `sentiment_factor.py` | Converts sentiment scores into a tradeable factor |

---

### Real-time Data (`data_service/realtime/`)

| Module | Purpose |
|---|---|
| `websocket_client.py` | Raw WebSocket connection handler |
| `real_time_feed.py` | Subscribes to live market data streams (Binance WS) |

---

### Storage (`data_service/storage/`)

| Module | Backend |
|---|---|
| `database_manager.py` | SQLite / PostgreSQL |
| `cache_manager.py` | Redis |

---

### Dashboard (`data_service/dashboard/`)

Streamlit-based interactive UI:
- `dashboard_app.py` — main app entry, layout and routing
- `charts.py` — Plotly K-line / candlestick charts, indicator overlays
- `widgets.py` — reusable Streamlit input widgets

Launch:
```bash
python run_dashboard.py
# http://localhost:8501
```

Web interface (FastAPI):
```bash
python run_web_interface.py
# http://localhost:8000
```

---

## Dependencies

### Core (always installed)
`pandas`, `numpy`, `python-binance`, `websocket-client`, `alpha_vantage`, `fastapi`, `uvicorn`, `redis`, `requests`, `aiohttp`, `textblob`, `openpyxl`

### Optional feature groups

| Extra | Packages |
|---|---|
| `[ai]` | `openai`, `langchain`, `langchain-openai`, `transformers`, `torch`, `sentence-transformers`, `spacy`, `nltk`, `scikit-learn`, `wordcloud` |
| `[visualization]` | `matplotlib`, `seaborn`, `plotly`, `streamlit` |
| `[realtime]` | real-time WebSocket dependencies |
| `[web]` | FastAPI web interface extras |
| `[test]` | `pytest`, `pytest-cov`, `pytest-asyncio` |

Install all:
```bash
pip install -e .[ai,visualization,realtime,web]
```

---

## Testing

```bash
# Python tests
pytest tests/ -v

# C++ tests (after build)
cd backend/build && ctest
```
