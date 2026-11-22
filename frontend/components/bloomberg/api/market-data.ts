// API functions for fetching market data

/**
 * Fetches market data from the API
 */
export async function fetchMarketData() {
  const response = await fetch("/api/market-data");

  if (!response.ok) {
    throw new Error(`Failed to fetch market data: ${response.status}`);
  }

  return response.json();
}

import type { MarketData, MarketItem } from "../types";

/**
 * Simulates market data updates on the client side
 * @param currentData The current market data
 */
export async function simulateMarketUpdate(currentData: MarketData) {
  // Refresh from the backend. Never invent price changes in the browser.
  return fetchMarketData();
}
