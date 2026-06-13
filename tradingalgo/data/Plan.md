# MutualPal
---
A full-stack **fintech analytics platform** for **mutual funds, ETFs, and algorithmic trading strategies**.  
Built to showcase expertise in **Data Engineering**, **Machine Learning Modeling**, **Quantitative Finance**, and **Real-Time Data Visualization**.

---

## 🚀 Features

### **1. Data Engineering & ETL**
- **Automated ETL pipeline** using Apache Airflow / dbt.
- **TimescaleDB** for scalable **time-series NAV & market data storage**.
- **Kafka-based event streaming** for real-time market updates.
- **DocETL integration** for LLM-powered document ingestion & data extraction.
- **Financial APIs integration** for mutual fund, ETF, and crypto market data.

### **2. Machine Learning & Quant Modeling**
- **Statistical modeling**: ARIMA, Prophet, GARCH for volatility & returns.
- **Time series forecasting** for NAV prediction.
- **ETF & Mutual Fund recommender system** based on investor profile & risk.
- **Portfolio optimization** using Modern Portfolio Theory & Risk Parity.
- **Algorithmic trading simulation** with backtesting engine.

### **3. Real-Time Frontend**
- **React/Next.js dashboard** with Tailwind UI.
- **WebSocket-powered** live data updates for NAVs, market indices, and trading signals.
- Interactive charts using Plotly/Recharts.
- User authentication and watchlists.

---

## 📊 Tech Stack

| Layer | Tools & Frameworks |
|-------|--------------------|
| **Data Storage** | PostgreSQL + TimescaleDB |
| **ETL / Orchestration** | Apache Airflow, dbt, DocETL |
| **Streaming** | Apache Kafka |
| **ML / Quant** | Python, scikit-learn, statsmodels, Prophet, PyPortfolioOpt |
| **Frontend** | Next.js / React, Tailwind CSS, WebSockets |
| **Backend** | FastAPI |
| **Deployment** | Docker, Kubernetes (future) |

---

## 📂 Project Structure

```

mutual-fund-etf-analytics/
│── data\_pipeline/         # Airflow DAGs, ETL scripts
│── models/                # ML models, backtesting
│── api/                   # FastAPI endpoints
│── frontend/              # Next.js dashboard
│── db/                    # TimescaleDB schemas & migrations
│── notebooks/             # Research & experimentation
│── README.md

````

---

## 🔍 Example Use Cases

- **Banks & Wealth Managers**: Fund screening, portfolio optimization, market forecasts.
- **Quant Research Firms**: Model testing, backtesting strategies.
- **Fintech Startups**: Personalized ETF/MF recommendation engines.
- **Crypto Funds** *(extension)*: Integrate DeFi/crypto asset analysis.

---

## 🛠️ Setup Instructions

1. **Clone repo**
   ```bash
   git clone https://github.com/your-username/mutual-fund-etf-analytics.git
   cd mutual-fund-etf-analytics

2. **Start Services**

   ```bash
   docker-compose up -d
   ```

3. **Run ETL Pipeline**

   ```bash
   airflow dags trigger etl_fund_data
   ```

4. **Launch Dashboard**

   ```bash
   cd frontend && npm install && npm run dev
   ```

---

## 📈 Example Screenshots

*(Add dashboard preview images here)*

---
