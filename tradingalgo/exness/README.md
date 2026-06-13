# Exness Trading Platform

A comprehensive real-time trading platform built with modern technologies, featuring live market data, order management, and PnL tracking.

## 🚀 Architecture

This is a Turborepo monorepo containing three main applications:

### Apps Structure

```
apps/
├── trading-frontend/     # Next.js 14 React application
├── trading-engine/       # Express.js backend server
└── market-data-service/  # WebSocket market data service
```

## 📋 Features

- **Real-time Trading**: Live order placement and management
- **Market Data**: Real-time price feeds via WebSocket
- **Authentication**: Email-based magic link authentication
- **PnL Tracking**: Comprehensive profit/loss analytics for closed positions
- **Portfolio Management**: Complete position and balance tracking
- **Responsive UI**: Modern trading interface with TailwindCSS

## 🛠️ Technology Stack

### Frontend
- **Next.js 14.2.5** - React framework with App Router
- **React 18** - UI library
- **TypeScript** - Type safety
- **TailwindCSS** - Styling framework
- **TradingView Charting Library** - Professional charts

### Backend
- **Express.js** - Web framework
- **Node.js** - Runtime environment
- **TypeScript** - Type safety
- **Nodemailer** - Email service for authentication
- **WebSocket** - Real-time communication

### Infrastructure
- **Turborepo** - Monorepo management
- **In-memory storage** - Fast data access for trading operations

## 🚦 Getting Started

### Prerequisites
- Node.js 18+ 
- npm or yarn
- Git

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd exness
   ```

2. **Install dependencies**
   ```bash
   npm install
   ```

3. **Set up environment variables**
   
   Create `.env` files in each app directory:

   **apps/trading-engine/.env**
   ```env
   EMAIL_USER=your-gmail@gmail.com
   EMAIL_PASS=your-app-password
   PORT=4000
   ```

   **apps/trading-frontend/.env.local**
   ```env
   NEXT_PUBLIC_API_URL=http://localhost:4000
   ```

4. **Start the development servers**
   ```bash
   npm run dev
   ```

   This will start all services:
   - Frontend: http://localhost:3000
   - Trading Engine: http://localhost:4000
   - Market Data Service: http://localhost:3001

## 📁 Project Structure

```
exness/
├── apps/
│   ├── trading-frontend/           # Frontend Application
│   │   ├── src/
│   │   │   ├── app/               # Next.js App Router pages
│   │   │   ├── components/        # React components
│   │   │   ├── contexts/          # React contexts
│   │   │   ├── hooks/             # Custom hooks
│   │   │   ├── lib/               # Utility libraries
│   │   │   └── types/             # TypeScript definitions
│   │   └── package.json
│   │
│   ├── trading-engine/             # Backend API Server
│   │   ├── src/
│   │   │   ├── api/               # API route handlers
│   │   │   ├── engine/            # Trading logic & state
│   │   │   ├── services/          # External services
│   │   │   └── types/             # TypeScript definitions
│   │   └── package.json
│   │
│   └── market-data-service/        # Market Data WebSocket Service
│       ├── src/
│       │   ├── websocket/         # WebSocket client/server
│       │   └── utils/             # Utility functions
│       └── package.json
│
├── turbo.json                      # Turborepo configuration
├── package.json                    # Root package configuration
└── README.md                       # This file
```

## 🔧 Development

### Running Individual Services

```bash
# Frontend only
cd apps/trading-frontend && npm run dev

# Backend only  
cd apps/trading-engine && npm run dev

# Market data service only
cd apps/market-data-service && npm run dev
```

### Building for Production

```bash
# Build all apps
npm run build

# Build specific app
npm run build --filter=trading-frontend
```

## 🔐 Authentication Setup

The platform uses email-based magic link authentication:

1. **Configure Gmail SMTP**:
   - Enable 2-factor authentication on your Gmail account
   - Generate an app-specific password
   - Add credentials to your `.env` file

2. **Email Flow**:
   - User enters email address
   - System sends magic link via email
   - User clicks link to authenticate
   - JWT token is issued for session management

## 💰 PnL Tracking System

The platform includes comprehensive profit/loss tracking:

- **Real-time P&L**: Live unrealized P&L for open positions
- **Realized P&L**: Historical performance for closed positions  
- **Analytics**: Win rate, average profit/loss, and performance metrics
- **Storage**: In-memory tracking with persistence options

## 📊 Trading Features

### Order Management
- Market orders with instant execution
- Position sizing and risk management
- Real-time order status updates

### Portfolio Tracking
- Live balance updates
- Position monitoring
- Historical transaction logs

### Market Data
- Real-time price feeds
- Professional charting with TradingView
- Multiple timeframe support

## 🔄 API Endpoints

### Authentication
- `POST /api/auth/send-verification` - Send magic link
- `POST /api/auth/verify` - Verify magic link token

### Trading
- `POST /api/orders` - Place new order
- `GET /api/positions` - Get open positions
- `POST /api/positions/close` - Close position

### Data
- `GET /api/pnl` - Get PnL analytics
- `GET /api/balance` - Get account balance

## 🧪 Testing

```bash
# Run all tests
npm run test

# Run tests for specific app
npm run test --filter=trading-frontend
```

## 📦 Deployment

### Production Build
```bash
npm run build
```

### Environment Configuration
Ensure all production environment variables are properly set in your deployment environment.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 📞 Support

For support and questions, please open an issue in the GitHub repository.

---

**Built with ❤️ for modern trading experiences**

Turborepo can use a technique known as [Remote Caching](https://turborepo.com/docs/core-concepts/remote-caching) to share cache artifacts across machines, enabling you to share build caches with your team and CI/CD pipelines.

By default, Turborepo will cache locally. To enable Remote Caching you will need an account with Vercel. If you don't have an account you can [create one](https://vercel.com/signup?utm_source=turborepo-examples), then enter the following commands:

```
cd my-turborepo

# With [global `turbo`](https://turborepo.com/docs/getting-started/installation#global-installation) installed (recommended)
turbo login

# Without [global `turbo`](https://turborepo.com/docs/getting-started/installation#global-installation), use your package manager
npx turbo login
yarn exec turbo login
pnpm exec turbo login
```

This will authenticate the Turborepo CLI with your [Vercel account](https://vercel.com/docs/concepts/personal-accounts/overview).

Next, you can link your Turborepo to your Remote Cache by running the following command from the root of your Turborepo:

```
# With [global `turbo`](https://turborepo.com/docs/getting-started/installation#global-installation) installed (recommended)
turbo link

# Without [global `turbo`](https://turborepo.com/docs/getting-started/installation#global-installation), use your package manager
npx turbo link
yarn exec turbo link
pnpm exec turbo link
```

## Useful Links

Learn more about the power of Turborepo:

- [Tasks](https://turborepo.com/docs/crafting-your-repository/running-tasks)
- [Caching](https://turborepo.com/docs/crafting-your-repository/caching)
- [Remote Caching](https://turborepo.com/docs/core-concepts/remote-caching)
- [Filtering](https://turborepo.com/docs/crafting-your-repository/running-tasks#using-filters)
- [Configuration Options](https://turborepo.com/docs/reference/configuration)
- [CLI Usage](https://turborepo.com/docs/reference/command-line-reference)
