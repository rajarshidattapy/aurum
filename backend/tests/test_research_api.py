import math

import aurum_core
import numpy as np
import pandas as pd
import pytest
from data_service.web import api_server as api
from fastapi.testclient import TestClient

client = TestClient(api.app)


def test_health():
    assert client.get("/api/health").json()["native_core"] is True


def test_backtest_is_deterministic_and_accounts_for_every_fill():
    first = client.post("/api/backtest/run", json={}).json()["results"]
    assert first == client.post("/api/backtest/run", json={}).json()["results"]
    assert first["total_trades"] > 0
    cash, qty = 100000.0, 0.0
    for trade in first["trades"]:
        sign = 1 if trade["side"] == "buy" else -1
        qty += sign * trade["quantity"]
        cash -= sign * trade["quantity"] * trade["price"] + trade["fee"]
        assert qty >= -1e-10 and cash >= -1e-10
    assert cash == pytest.approx(first["cash"])
    assert qty == pytest.approx(first["open_quantity"])
    assert first["total_return"] == pytest.approx(
        first["equity_curve"][-1]["value"] / 100000.0 - 1
    )
    values = np.array([point["value"] for point in first["equity_curve"]])
    assert first["max_drawdown"] == pytest.approx(
        (values / np.maximum.accumulate(values) - 1).min()
    )


@pytest.mark.parametrize(
    "parameters",
    [
        {"short_window": 0},
        {"long_window": 1},
        {"max_weight": 2},
        {"short_window": 1.5},
        {"unknown": 4},
    ],
)
def test_invalid_parameters(parameters):
    assert (
        client.post(
            "/api/backtest/run", json={"strategy_config": {"parameters": parameters}}
        ).status_code
        == 422
    )


@pytest.mark.parametrize(
    "payload",
    [
        {"commission_rate": -1},
        {"slippage": 1},
        {"strategy_config": {"initial_capital": 0}},
        {"strategy_config": {"start_date": "2025-12-31", "end_date": "2025-01-01"}},
    ],
)
def test_invalid_requests(payload):
    assert client.post("/api/backtest/run", json=payload).status_code == 422


def test_core_signal_has_no_lookahead():
    prices = [100 + 10 * math.sin(i / 7) for i in range(100)]
    full = aurum_core.signals(prices, 3, 8)
    assert aurum_core.signals(prices[:55], 3, 8) == full[:55]
    assert full[:8] == [0] * 8


def test_risk_rejects_bad_values_and_short_sales():
    for quantity in [0, -1, float("nan"), float("inf")]:
        assert not aurum_core.check_order(True, quantity, 100, 10000, 0, 0.1)
    for price in [0, -1, float("nan"), float("inf")]:
        assert not aurum_core.check_order(True, 1, price, 10000, 0, 0.1)
    assert not aurum_core.check_order(False, 1, 100, 10000, 0, 0.1)
    assert not aurum_core.check_order(True, 20, 100, 10000, 0, 0.1)
    assert aurum_core.check_order(True, 5, 100, 10000, 0, 0.1)


def test_legacy_endpoints_cannot_fabricate_results():
    assert client.post("/api/ai/analyze", json={}).status_code == 501
    assert client.post("/api/factors/analyze", json={}).status_code == 501
    assert client.get("/api/trades/recent").json()["trades"] == []


def test_provider_errors_do_not_turn_into_demo(monkeypatch):
    def fail(*args):
        raise RuntimeError("offline")

    monkeypatch.setattr(api, "load_bars", fail)
    assert client.get("/api/market/data/AAPL?source=yahoo").status_code == 502


def test_terminal_provenance():
    result = client.get("/api/terminal").json()
    assert result["provenance"]["synthetic"] is True
    assert result["americas"][0]["id"] == "DOW JONES"
    assert len(result["asiaPacific"]) > 1


def test_next_bar_open_execution():
    result = client.post(
        "/api/backtest/run", json={"commission_rate": 0, "slippage": 0}
    ).json()["results"]
    frame = api.bars("DEMO", api.date(2025, 1, 1), api.date(2025, 12, 31), "demo")
    signals = aurum_core.signals(frame.close.tolist(), 10, 30)
    for trade in result["trades"]:
        idx = frame.index.get_loc(pd.Timestamp(trade["date"]))
        assert signals[idx - 1] == (1 if trade["side"] == "buy" else -1)
        assert trade["price"] == pytest.approx(frame.iloc[idx].open)
