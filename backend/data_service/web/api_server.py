"""Research API. Paper-only. No broker or exchange order submission."""

import math
import os
import time
from datetime import date
from functools import lru_cache
from pathlib import Path
from typing import Literal

import numpy as np
import pandas as pd
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, ConfigDict, Field, model_validator

try:
    import aurum_core
except ImportError:
    aurum_core = None


class StrategyRequest(BaseModel):
    model_config = ConfigDict(extra="forbid")
    strategy_name: Literal["ma_crossover"] = "ma_crossover"
    symbols: list[str] = Field(
        default_factory=lambda: ["DEMO"], min_length=1, max_length=1
    )
    parameters: dict = Field(default_factory=dict)
    start_date: date = date(2025, 1, 1)
    end_date: date = date(2025, 12, 31)
    initial_capital: float = Field(default=100000, gt=0, le=1e12, allow_inf_nan=False)

    @model_validator(mode="after")
    def dates(self):
        if self.end_date <= self.start_date:
            raise ValueError("end_date must follow start_date")
        if (self.end_date - self.start_date).days > 3650:
            raise ValueError("Maximum range is 10 years")
        return self


class BacktestRequest(BaseModel):
    model_config = ConfigDict(extra="forbid")
    strategy_config: StrategyRequest = Field(default_factory=StrategyRequest)
    commission_rate: float = Field(default=0.001, ge=0, le=0.05, allow_inf_nan=False)
    slippage: float = Field(default=0.0005, ge=0, le=0.05, allow_inf_nan=False)
    source: Literal["demo", "yahoo"] = "demo"


@lru_cache(maxsize=64)
def load_bars(symbol, start, end, source, cache_bucket):
    if source == "demo":
        # Fixed synthetic fixture, not a quote or a forecast.
        idx = pd.bdate_range("2025-01-01", "2025-12-31")
        x = np.arange(len(idx))
        close = 100 + x * 0.025 + 9 * np.sin(x / 11) + 2 * np.sin(x / 3)
        opening = np.r_[close[0], close[:-1]] * (1 + 0.001 * np.sin(x))
        frame = pd.DataFrame(
            dict(
                open=opening,
                high=np.maximum(opening, close) * 1.01,
                low=np.minimum(opening, close) * 0.99,
                close=close,
                volume=100000 + x * 100,
            ),
            index=idx,
        )
        return frame.loc[start:end].copy()
    import yfinance as yf

    frame = yf.Ticker(symbol).history(
        start=start, end=end, auto_adjust=True, timeout=10
    )
    frame.columns = [str(c).lower() for c in frame.columns]
    return frame


def bars(symbol, start, end, source):
    try:
        frame = load_bars(
            symbol, str(start), str(end), source, int(time.time() // 300)
        ).copy()
    except Exception as exc:
        raise HTTPException(
            502, "Market data provider failed; no synthetic substitution was made"
        ) from exc
    if frame.empty:
        raise HTTPException(422, "No bars in the requested range")
    needed = ["open", "high", "low", "close", "volume"]
    if any(c not in frame for c in needed):
        raise HTTPException(502, "Provider returned an invalid OHLCV schema")
    values = frame[needed].to_numpy(dtype=float)
    if (
        not np.isfinite(values).all()
        or (values[:, :4] <= 0).any()
        or (values[:, 4] < 0).any()
    ):
        raise HTTPException(502, "Provider returned invalid OHLCV values")
    if not frame.index.is_monotonic_increasing or frame.index.has_duplicates:
        raise HTTPException(502, "Provider timestamps must be unique and increasing")
    return frame


def backtest(request):
    if aurum_core is None:
        raise HTTPException(503, "Build the C++ core before running a backtest")
    config = request.strategy_config
    unknown = set(config.parameters) - {"short_window", "long_window", "max_weight"}
    if unknown:
        raise HTTPException(422, "Unsupported strategy parameter")
    short = config.parameters.get("short_window", 10)
    long = config.parameters.get("long_window", 30)
    weight = config.parameters.get("max_weight", 0.1)
    if type(short) is not int or type(long) is not int or not 0 < short < long <= 500:
        raise HTTPException(422, "Require integer windows: 0 < short < long <= 500")
    if (
        isinstance(weight, bool)
        or not isinstance(weight, (int, float))
        or not math.isfinite(weight)
        or not 0 < weight <= 1
    ):
        raise HTTPException(422, "max_weight must be in (0,1]")
    frame = bars(config.symbols[0], config.start_date, config.end_date, request.source)
    if len(frame) <= long:
        raise HTTPException(422, "Insufficient history for long_window")
    signals = aurum_core.signals(frame.close.tolist(), short, long)
    cash, held = config.initial_capital, 0.0
    curve, trades = [], []
    for i, (timestamp, row) in enumerate(frame.iterrows()):
        # Signal uses yesterday's close. Fill at today's open plus adverse slippage.
        signal = signals[i - 1] if i else 0
        if signal == 1 and held == 0:
            price = float(row.open) * (1 + request.slippage)
            quantity = cash * weight / (price * (1 + request.commission_rate))
            if aurum_core.check_order(True, quantity, price, cash, held, weight):
                fee = quantity * price * request.commission_rate
                cash -= quantity * price + fee
                held += quantity
                trades.append(
                    dict(
                        date=timestamp.isoformat(),
                        side="buy",
                        quantity=quantity,
                        price=price,
                        fee=fee,
                    )
                )
        elif signal == -1 and held > 0:
            price = float(row.open) * (1 - request.slippage)
            if aurum_core.check_order(False, held, price, cash, held, weight):
                fee = held * price * request.commission_rate
                trades.append(
                    dict(
                        date=timestamp.isoformat(),
                        side="sell",
                        quantity=held,
                        price=price,
                        fee=fee,
                    )
                )
                cash += held * price - fee
                held = 0.0
        curve.append(
            dict(date=timestamp.isoformat(), value=float(cash + held * row.close))
        )
    equity = np.array([p["value"] for p in curve])
    daily = equity[1:] / equity[:-1] - 1
    drawdown = equity / np.maximum.accumulate(equity) - 1
    return dict(
        strategy_name="ma_crossover",
        source=request.source,
        execution="paper",
        engine="C++ signals/risk + Python accounting",
        total_return=float(equity[-1] / config.initial_capital - 1),
        max_drawdown=float(drawdown.min()),
        sharpe_ratio=float(daily.mean() / daily.std() * np.sqrt(252))
        if daily.std() > 0
        else 0.0,
        total_trades=len(trades),
        cash=float(cash),
        open_quantity=float(held),
        equity_curve=curve,
        trades=trades,
        assumptions=[
            "Long-only, one asset",
            "Next-bar open fills",
            "Fees and adverse slippage applied",
            "252 observations/year Sharpe, zero risk-free rate",
            "Final position marked to last close, not liquidated",
        ],
    )


class APIServer:
    def __init__(self, host="127.0.0.1", port=8000):
        self.host, self.port = host, port
        self.app = FastAPI(
            title="Aurum Research API",
            version="0.2.0",
            description="Paper research only. Demo data is synthetic. No broker execution.",
        )
        origins = os.environ.get(
            "ALLOWED_ORIGINS", "http://localhost:3000,http://127.0.0.1:3000"
        ).split(",")
        self.app.add_middleware(
            CORSMiddleware,
            allow_origins=origins,
            allow_methods=["GET", "POST"],
            allow_headers=["Content-Type"],
        )
        app = self.app

        @app.get("/api/health")
        def health():
            return dict(
                status="healthy" if aurum_core else "degraded",
                native_core=aurum_core is not None,
                mode="paper-only",
            )

        @app.get("/api/strategies")
        def strategies():
            return dict(
                strategies=[
                    dict(
                        name="ma_crossover",
                        description="C++ moving-average crossover, next-bar paper fills",
                    )
                ]
            )

        @app.post("/api/backtest/run")
        def run(request: BacktestRequest):
            return dict(status="success", results=backtest(request))

        @app.get("/api/market/data/{symbol}")
        def market(
            symbol: str,
            source: Literal["demo", "yahoo"] = "demo",
            start: date = date(2025, 1, 1),
            end: date = date(2025, 12, 31),
        ):
            if end <= start or (end - start).days > 3650:
                raise HTTPException(422, "Invalid date range")
            frame = bars(symbol, start, end, source)
            return dict(
                symbol=symbol,
                source=source,
                synthetic=source == "demo",
                data=[
                    dict(date=idx.isoformat(), price=float(row.close))
                    for idx, row in frame.iterrows()
                ],
            )

        @app.get("/api/terminal")
        def terminal():
            import json

            snapshot = json.loads(
                (Path(__file__).parent / "terminal_snapshot.json").read_text()
            )
            return {
                **snapshot,
                "dataSource": "legacy synthetic snapshot",
                "isFromRedis": False,
                "provenance": {
                    "synthetic": True,
                    "as_of": None,
                    "note": "Original frontend sample data, not live quotes",
                },
                "native_core": aurum_core is not None,
            }

        @app.get("/api/portfolio/status")
        def portfolio():
            return dict(
                mode="paper-only",
                total_value=0,
                cash=0,
                positions=[],
                daily_pnl=0,
                total_pnl=0,
                note="No persistent portfolio; each backtest is isolated",
            )

        @app.get("/api/trades/recent")
        def trades():
            return dict(
                trades=[],
                note="Trade records are returned by each backtest; no live account",
            )

        @app.post("/api/factors/analyze")
        @app.post("/api/ai/analyze")
        def unavailable():
            raise HTTPException(
                501,
                "This legacy endpoint is not implemented; no fabricated result is returned",
            )

    def run(self, debug=False):
        import uvicorn

        uvicorn.run(self.app, host=self.host, port=self.port)


def create_app():
    return APIServer().app


app = create_app()
