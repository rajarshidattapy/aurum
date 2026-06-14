# Aurum Frontend — Bloomberg Terminal

A Bloomberg-style market data terminal built with Next.js 14, featuring real-time global market indices, AI-powered analysis, and a keyboard-driven interface.

---

## Tech Stack

| Layer | Technology |
|---|---|
| Framework | Next.js 14 (App Router) |
| Language | TypeScript |
| Styling | Tailwind CSS |
| UI Primitives | shadcn/ui (Radix UI) |
| State Management | Jotai (atomic state) |
| Server State / Caching | TanStack React Query v5 |
| Data Store | Upstash Redis (REST) |
| Market Data | Alpha Vantage API |
| AI Analysis | OpenAI API |
| Linting / Formatting | Biome |
| Git Hooks | Husky |

---

## Project Structure

```
frontend/
├── app/
│   ├── api/
│   │   ├── ai/                  # AI market analysis endpoint + rate limiting
│   │   ├── market-data/         # GET/POST market data (Redis + fallback)
│   │   ├── init-scheduler/      # Background market refresh scheduler
│   │   └── seed-redis/          # One-time Redis seed endpoint
│   ├── layout.tsx               # Root layout (metadata, global CSS)
│   └── page.tsx                 # Entry point — mounts BloombergTerminal
│
├── components/
│   ├── bloomberg/               # All terminal-specific components
│   │   ├── api/                 # Data fetching functions & query keys
│   │   ├── atoms/               # Jotai atoms (UI state, filters, watchlists)
│   │   ├── core/                # Shared interactive components
│   │   ├── hooks/               # Custom React hooks
│   │   ├── layout/              # Terminal skeleton (header, filter bar, layout)
│   │   ├── lib/                 # Utilities (currency, market math, theme, time)
│   │   ├── providers/           # React Query client provider
│   │   ├── ui/                  # Data display components (tables, charts)
│   │   ├── views/               # Full-screen views (market, news, movers, etc.)
│   │   └── types.ts             # Shared TypeScript types
│   │
│   ├── ui/                      # shadcn/ui base components
│   └── theme-provider.tsx       # Dark/light theme context
│
└── styles/
    └── globals.css              # Tailwind base styles
```

---

## Architecture

### Data Flow

```
Alpha Vantage API / Static fallback
         │
         ▼
  Upstash Redis (cache, TTL 1h)
         │
         ▼
  Next.js API Route (/api/market-data)
         │
         ▼
  React Query (polls every 30s real-time / 5min normal)
         │
         ▼
  Jotai atoms (UI state, diff tracking, sparklines)
         │
         ▼
  React components (market table, charts, modals)
```

### State Management

**Jotai atoms** handle all client-side state. Key atoms:

| Atom | Purpose |
|---|---|
| `writableFiltersAtom` | Column visibility filters (Movers, YTD, AVAT, etc.) |
| `isRealTimeEnabledAtom` | Toggles 30s vs 5min polling |
| `updatedCellsAtom` | Tracks which cells changed (for flash highlight) |
| `updatedSparklinesAtom` | Tracks sparkline updates |
| `watchlistsAtom` | User-created watchlists |
| `isConfirmModalOpenAtom` | Confirmation modal state |

**React Query** manages server state:
- `queryKey: ["marketData"]`
- `refetchInterval`: 30,000ms (real-time) or 300,000ms (default)
- `staleTime`: 10,000ms
- `gcTime`: 3,600,000ms (1 hour)

---

## Views

| View | Key (`currentView`) | Description |
|---|---|---|
| Market | `"market"` | Main table — Americas, EMEA, Asia-Pacific indices |
| News | `"news"` | Market news feed |
| Market Movers | `"movers"` | Top gainers / losers |
| Volatility | `"volatility"` | Volatility metrics per index |
| RMI | `"rmi"` | Relative Market Index chart |

---

## Data Model

### `MarketItem`

```ts
type MarketItem = {
  id: string;           // Index name (e.g. "S&P 500")
  num?: string;         // Display number
  rmi?: string;         // Relative Market Index value
  value: number;        // Current price
  change: number;       // Absolute change
  pctChange: number;    // Percentage change
  avat: number;         // Average trading volume
  time: string;         // Last update time (HH:MM)
  ytd: number;          // Year-to-date return (%)
  ytdCur: number;       // YTD currency-adjusted (%)
  sparkline1?: number[];  // 1-day mini chart data
  sparkline2?: number[];  // Secondary sparkline
  volatility?: number;
  isMover?: boolean;
};
```

### `MarketData`

Three regions: `americas`, `emea`, `asiaPacific` — each an array of `MarketItem`.

### `FilterState`

```ts
interface FilterState {
  showMovers: boolean;
  showVolatility: boolean;
  showRatios: boolean;
  showFutures: boolean;
  showAvat: boolean;
  show10D: boolean;
  showYTD: boolean;
  showCAD: boolean;
}
```

---

## API Routes

### `GET /api/market-data`
Returns market data. Checks Redis first; falls back to static seed data if Redis is unavailable. Enhances fallback data with generated sparklines.

### `POST /api/market-data` `{ action: "update" }`
Generates realistic correlated market updates (market sentiment + region factors + individual noise) and stores back to Redis.

### `GET /api/ai`
AI market analysis endpoint with per-IP rate limiting (uses `rate-limit.ts`).

### `GET /api/init-scheduler`
Starts the background scheduler that triggers market data refreshes.

### `GET /api/seed-redis`
Seeds Redis with initial market data.

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `1` | Market view |
| `2` | News view |
| `3` | Market Movers view |
| `4` | Volatility view |
| `Ctrl+N` | Create new watchlist |
| `Ctrl+B` | Reset all filters |
| `Ctrl+R` | Refresh data |
| `Ctrl+L` | Toggle live updates |
| `Escape` | Cancel current operation |
| `?` | Show shortcuts help |

---

## Theme

Two themes — dark (default) and light — toggled via the header button.

Bloomberg color palette:

| Token | Dark | Light |
|---|---|---|
| Background | `#121212` | `#f0f0f0` |
| Surface | `#1e1e1e` | `#e0e0e0` |
| Header | `#000000` | `#d0d0d0` |
| Accent | `#ff9900` | `#ff9900` |
| Positive | `#4CAF50` | `#4CAF50` |
| Negative | `#F44336` | `#F44336` |

---

## Environment Variables

Copy `.env.local.example` to `.env.local` and fill in:

```env
# Upstash Redis
UPSTASH_REDIS_REST_URL=
UPSTASH_REDIS_REST_TOKEN=
KV_REST_API_URL=          # alias for backward compat
KV_REST_API_TOKEN=        # alias for backward compat

# Market data
ALPHA_VANTAGE_API_KEY=

# AI features
OPENAI_API_KEY=

# CORS
ALLOWED_ORIGINS=https://your-domain.vercel.app,http://localhost:3000
```

---

## Getting Started

```bash
# Install dependencies
npm install

# Set up environment
cp .env.local.example .env.local
# Fill in your API keys

# Run development server
npm run dev

# Seed Redis with initial data (first run)
curl http://localhost:3000/api/seed-redis
```

---

## Key Implementation Notes

- **Sparklines** update every 5 minutes (tracked via `last_sparkline_update` key in Redis), not every poll cycle, to avoid excessive re-renders.
- **Market sentiment correlation**: Updates simulate real-market behavior — a shared sentiment factor influences all indices, with region-specific and individual noise layered on top.
- **Fallback resilience**: If Redis is down, all routes gracefully serve static seed data with generated sparklines. No hard dependency on Redis at runtime.
- **Cell flash highlighting**: `updatedCellsAtom` and `updatedSparklinesAtom` diff current vs previous data on each React Query fetch to drive CSS flash animations only on changed cells.
- **YTD calculation**: Year-start values are computed once on first load as `current / (1 + ytd/100)` and held in-memory to keep YTD consistent across updates.
