# Verification checkpoint

Local verification: 22 November 2026. Base repository commit: `99160e80a927631ebd6bec9cb05df6c54628c995`.

## Results

- Python: `python -m pytest tests -q` in `backend/`: 38 passed, 1 skipped.
- Native: CMake build succeeded, `ctest --output-on-failure --timeout 20`: 8 passed.
- Frontend: `pnpm typecheck` and `pnpm build` succeeded.
- Running terminal: production Next.js -> FastAPI -> original sample dataset. The backend API separately called the compiled C++ module and returned six computed paper fills in its ledger.
- UI parity: built a pristine checkout of the original commit and the revised frontend. At 1440x1150, using the same original sample data, the two terminal screenshots were pixel-identical. The pristine baseline received a controlled fixture; the revised app obtained that same fixture through its actual Python proxy. Visual code was also checked against Git: no page, component-layout, CSS or style changes remain.
- Standalone native executable initialized its Python loader and shut down cleanly after SIGTERM. This smoke run does not prove a trading strategy executed or that a broker was contacted. No broker was used.

## What the tests cover

C++ tests cover missing-price rejection, valid and oversized orders, long-only sells, deterministic loader conversion/caching, partial-sale cost basis, invalid MA windows, duplicate-bar isolation, paper executor double-start, fill/status, duplicate submit and stop.

Python tests include the existing processor/fetcher/LLM unit tests plus the research API. New checks cover deterministic results, reconciliation of every fill to cash and holdings, drawdown arithmetic, input bounds, signal-prefix causality, next-bar open fills, invalid native order values, explicit provider failures and honest legacy-endpoint responses.

Native loader tests inject a deterministic Python fetcher, not the network. The optional live-provider path is not part of the offline CI guarantee. One existing live API integration test is explicitly skipped (`Skipping live API test`). No paid model calls are required for the checks.

## Visual evidence

- `images/terminal-before.png`: pristine original terminal under a controlled sample dataset.
- `images/terminal-after.png`: same terminal, same dataset, served through the Python backend.

Both captures were visually inspected. No /research page, banner or visual redesign remains.

Screenshots are local application runs, not a claim that a deployment has been updated.

## Limits

These tests are a milestone, not a proof of perfection. Legacy ML/AI/factor algorithms, experimental apps under `tradingalgo/`, real exchange WebSockets, long-running concurrency, live broker execution, persistent portfolio recovery, multi-user auth and deployment security have not been fully audited. The existing frontend AI streaming route and news provider remain separate optional integrations, not newly routed through Python. The terminal market-data path is joined; this is not a claim that every original screen or feature is now one backend. The native standalone paper engine is a diagnostic scaffold, not the canonical backtest runner. Its session-level P&L is not a calendar-day risk service.

The synthetic fixture has smooth, designed price behavior. The resulting return or Sharpe must not be used as evidence of an economic edge. Real fills can have gaps, liquidity constraints, taxes, lot-size restrictions and costs absent from this model.
