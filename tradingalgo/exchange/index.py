from fastapi import FastAPI
from pydantic import BaseModel
from typing import Dict, List

app = FastAPI()

TICKER = "GOOGLE"


# ---------- Data Models ----------

class Order(BaseModel):
    userId: str
    price: float
    quantity: int


class OrderRequest(BaseModel):
    side: str   # "bid" | "ask"
    price: float
    quantity: int
    userId: str


class User:
    def __init__(self, id: str, balances: Dict[str, float]):
        self.id = id
        self.balances = balances


# ---------- In-Memory State ----------

users: List[User] = [
    User("1", {"GOOGLE": 10, "USD": 50000}),
    User("2", {"GOOGLE": 10, "USD": 50000}),
]

bids: List[Order] = []
asks: List[Order] = []


# ---------- Helpers ----------

def get_user(user_id: str):
    return next((u for u in users if u.id == user_id), None)


def flip_balance(user_id1: str, user_id2: str, quantity: int, price: float):
    user1 = get_user(user_id1)
    user2 = get_user(user_id2)
    if not user1 or not user2:
        return

    user1.balances[TICKER] -= quantity
    user2.balances[TICKER] += quantity
    user1.balances["USD"] += quantity * price
    user2.balances["USD"] -= quantity * price


def fill_orders(side: str, price: float, quantity: int, user_id: str) -> int:
    remaining = quantity

    if side == "bid":
        for i in range(len(asks) - 1, -1, -1):
            if asks[i].price > price:
                continue

            if asks[i].quantity > remaining:
                asks[i].quantity -= remaining
                flip_balance(asks[i].userId, user_id, remaining, asks[i].price)
                return 0
            else:
                remaining -= asks[i].quantity
                flip_balance(asks[i].userId, user_id, asks[i].quantity, asks[i].price)
                asks.pop()

    else:  # ask
        for i in range(len(bids) - 1, -1, -1):
            if bids[i].price < price:
                continue

            if bids[i].quantity > remaining:
                bids[i].quantity -= remaining
                flip_balance(user_id, bids[i].userId, remaining, price)
                return 0
            else:
                remaining -= bids[i].quantity
                flip_balance(user_id, bids[i].userId, bids[i].quantity, price)
                bids.pop()

    return remaining


# ---------- Routes ----------

@app.post("/order")
def place_order(req: OrderRequest):
    remaining_qty = fill_orders(
        req.side, req.price, req.quantity, req.userId
    )

    if remaining_qty == 0:
        return {"filledQuantity": req.quantity}

    order = Order(
        userId=req.userId,
        price=req.price,
        quantity=remaining_qty
    )

    if req.side == "bid":
        bids.append(order)
        bids.sort(key=lambda x: x.price)
    else:
        asks.append(order)
        asks.sort(key=lambda x: -x.price)

    return {"filledQuantity": req.quantity - remaining_qty}


@app.get("/depth")
def get_depth():
    depth: Dict[str, Dict] = {}

    for b in bids:
        p = str(b.price)
        if p not in depth:
            depth[p] = {"type": "bid", "quantity": b.quantity}
        else:
            depth[p]["quantity"] += b.quantity

    for a in asks:
        p = str(a.price)
        if p not in depth:
            depth[p] = {"type": "ask", "quantity": a.quantity}
        else:
            depth[p]["quantity"] += a.quantity

    return {"depth": depth}


@app.get("/balance/{user_id}")
def get_balance(user_id: str):
    user = get_user(user_id)
    if not user:
        return {"USD": 0, TICKER: 0}
    return {"balances": user.balances}


@app.get("/quote")
def quote():
    # Not implemented (same as your TODO)
    return {}
