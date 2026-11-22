# Aurum master document

## Goal and boundaries

Aurum's original README describes an AI mutual-fund terminal for Indian markets. The checked-in code is broader and less integrated: a global-index UI, Python analytics modules, a C++ engine scaffold, and many independent experiments. The first dependable milestone is therefore a **paper research vertical slice**, not a broker or a mutual-fund execution platform.

This milestone keeps C++ and Python as the backend. It avoids a frontend redesign. It does not connect money, place orders, collect brokerage credentials, or promise production readiness.

## Runtime architecture

```text
Existing browser terminal / (unchanged visual code)
  -> Next.js /api/market-data (server-side proxy)
  -> FastAPI /api/terminal
  -> original terminal sample dataset, provenance in the response

HTTP client / interactive API docs
  -> FastAPI /api/backtest/run
      -> validated OHLCV bars (synthetic fixture or Yahoo history)
      -> aurum_core.signals, compiled C++ MovingAverageStrategy
      -> Python next-bar accounting loop
          -> aurum_core.check_order, compiled C++ RiskManager
          -> fee and slippage, cash/units, fill ledger
      -> equity curve, return, Sharpe and maximum drawdown

```

`backend/backend/src/bindings.cpp` exposes the native research functions using pybind11. The build writes the extension into `backend/`, where the Python server can import it. If the extension is missing, health is degraded and backtests return `503`. A missing backend produces a non-success proxy response, not a fallback result.

## Execution semantics

- Only one symbol per backtest. No short positions, margin, borrowing, leverage or broker connectivity.
- Short and long moving-average windows are positive integers, short < long, maximum long 500 bars.
- Crossover state is per symbol. A new bar must have a strictly newer timestamp. Equal or older timestamps do not create repeated signals.
- The first observed non-flat MA regime initializes state. Only a later regime transition creates a signal.
- A close-based signal is acted on at the next bar's open. The last signal cannot execute without another bar.
- Buying allocates at most `max_weight` of cash, including commission. One position at a time. Selling closes the held position.
- Fees apply to every fill. Slippage moves buy prices up and sell prices down.
- Equity marks the open position at each close. The final position is not forcibly liquidated.
- Maximum drawdown is the minimum of equity/running peak - 1. Sharpe uses observed bar returns, a 252-bar annualization and zero risk-free rate. These assumptions are included in the response.
- Synthetic backtest metrics test the wiring and arithmetic. They say nothing about investability or expected returns.

## Repository inventory

| Area | Contents | Relationship to the verified path |
| --- | --- | --- |
| `frontend/` | Next.js 15, React 19, Tailwind, Jotai, React Query | Original pages/components/layout/styles retained exactly; API/data-layer proxy only |
| `backend/data_service/web/api_server.py` | Canonical FastAPI service | Validated requests, real calculations, explicit provenance |
| `backend/backend/` | C++17 types, strategy, risk, loader, paper executor, bindings | Native signal/risk functions used by Python; standalone diagnostic executable also builds |
| `backend/data_service/fetchers/` | Yahoo, Binance, Alpha Vantage adapters | Yahoo is used by optional historical-data requests; Binance is not an execution venue here |
| `backend/data_service/backtest/`, `factors/`, `ml/`, `ai/` | Older analytics/AI implementations | Retained for future review; not represented as audited production modules |
| `backend/static/`, legacy launchers/dashboard modules | Earlier web and Streamlit experiments | Not the canonical application |
| `tradingalgo/Bitcoin-price-prediction/` | Bitcoin datasets and prediction/sentiment notebooks | Research reference, not integrated |
| `tradingalgo/bitcoin trading/` | Notebook and notes | Research reference |
| `tradingalgo/Mutual_Fund_Plan.ipynb`, NVIDIA and stock-analysis notebooks | Portfolio and analysis experiments | Not an implemented mutual-fund NAV service |
| `tradingalgo/assignment-hkirat/` | TypeScript/Express exchange exercise with Jest setup | Separate application, not C++ engine code |
| `tradingalgo/exchange/` | Python matching/exchange exercise and notes | Educational exchange model, not a broker adapter |
| `tradingalgo/exness/` | Node/Next/Vite trading-platform experiment | Separate monorepo; not launched or merged into the verified backend |
| `tradingalgo/options-pricing-model/` | Option-pricing methods, Streamlit material, nested opinion-trading project | Separate experiment and provenance review needed |
| `tradingalgo/portfolio_track/` | Flask-style portfolio tracker and CSV material | Separate app, not canonical persistence |
| `tradingalgo/stock market analysis/` | Python analysis scripts and templates | Separate app |
| `tradingalgo/quant/` | Notebook, submission CSV and notes | Research reference |
| `tradingalgo/trading agent/` | Eight workflow JSON exports | Not automatically imported, executed or trusted with credentials |
| `tradingalgo/data/` | ETL plans, PDFs, notes and screenshots | Future data-engineering ideas, not a running pipeline |
| `tradingalgo/Nova/` | Empty directory in the inspected checkout | No implementation to integrate |
| `os.md` | Repository links, videos and social posts | Reference inventory; see REFERENCES.md |

## Backend repairs in this milestone

The original native target could not compile: missing logger/config headers, incorrect DataLoader declarations, undeclared price state, missing interpreter lifetime, and a call to a nonexistent strategy method. Those wiring errors are repaired. Risk checks no longer recursively lock the same mutex or accept orders without a usable price. Position cost basis no longer changes on a partial sell. Paper execution has status/cancel implementations, duplicate submission checks and repeatable start/stop behavior.

The Python package exported a nonexistent processor class and omitted YahooFetcher. Binance imported a removed WebSocket module and lacked methods used by its example. Those import/API mismatches are repaired. Most importantly, the canonical HTTP API no longer returns hardcoded market, portfolio or backtest performance as if it were computed.

## Interfaces and errors

| Endpoint | Behavior |
| --- | --- |
| `GET /api/health` | Native-core readiness and paper-only mode |
| `GET /api/strategies` | Only the implemented `ma_crossover` strategy |
| `POST /api/backtest/run` | Computed backtest and trade ledger |
| `GET /api/market/data/{symbol}` | Historical close series; explicit `demo` or `yahoo` source |
| `GET /api/terminal` | Original terminal sample dataset; synthetic provenance in the response |
| `GET /api/portfolio/status` | Empty account, with explicit no-persistent-portfolio note |
| `GET /api/trades/recent` | Empty account ledger; backtest trades live in its response |
| Legacy factor and AI POST endpoints | `501`, not fabricated analytics |

Validation errors return `422`. Provider failures return `502`, with no demo substitution. A missing native module returns `503` for a backtest. No credentials are needed for the demo. The API binds to loopback and CORS defaults to local frontend origins. It has no authentication or public-deployment hardening; do not expose it directly to the internet.

## Remaining integration boundaries

The original frontend AI streaming route and news provider have not been migrated to Python. They remain optional, separate integrations and are not included in the offline terminal-data/backtest guarantee. No visual files were changed to expose the backtest; use HTTP or the API documentation.

## Next milestones, not completed features

1. Add an Indian mutual-fund NAV source with fund identifiers, valuation dates, provenance, adjustments and benchmark alignment. Mutual-fund NAVs are not tick-level exchange prices.
2. Extend tests for splits/dividends, calendar gaps, delistings, lookahead, parameter selection and out-of-sample evaluation. Add walk-forward evaluation before comparing strategies.
3. Add durable paper portfolios with transactional reservations, idempotent commands, restart recovery and an audit trail.
4. Audit the legacy analytics modules individually. Integrate only reviewed functions behind typed interfaces. Do not merge all exploratory apps wholesale.
5. Implement historical/live provider interfaces with source timestamps, cache policy and clear stale-data behavior. A polling loop is not proof of a live feed.
6. Add optional LLM explanations grounded in the actual calculation output. Keep model-generated text outside order authority.
7. Only after a separate, approved specification: deployment, user auth, monitoring, broker sandbox integration, secret management, limits and regulatory review. Real trading is out of scope for this milestone.
