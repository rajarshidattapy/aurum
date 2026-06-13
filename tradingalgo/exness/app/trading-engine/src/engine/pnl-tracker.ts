 export interface ClosedPosition {
  positionId: string;
  asset: string;
  type: 'long' | 'short';
  margin: number;
  leverage: number;
  slippage: number;
  entryPrice: number;
  exitPrice: number;
  quantity: number;
  realizedPnL: number;
  openTimestamp: number;
  closeTimestamp: number;
  duration: number; // in milliseconds
  status: 'closed' | 'liquidated';
  fees?: number;
}


export interface PnLSummary {
  totalRealizedPnL: number;
  totalTrades: number;
  winningTrades: number;
  losingTrades: number;
  winRate: number;
  averageWin: number;
  averageLoss: number;
  largestWin: number;
  largestLoss: number;
  totalFees: number;
}

const closedPostions: Record<string,ClosedPosition>={};

export const addClosedPosition = ( closedPostion:ClosedPosition):void => {
    console.log(`[PnL Tracker] Adding closed position ${closedPostion.positionId}`);
    closedPostions[closedPostion.positionId] = closedPostion;
};

export const getClosedPositions = ():ClosedPosition[]=>{
    return Object.values(closedPostions).sort((a,b) => b.closeTimestamp - a.closeTimestamp);
};



