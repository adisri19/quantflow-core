/**
 * @quantflow/react - React SDK for bot custom frontends
 *
 * This SDK provides hooks and utilities for building custom dashboards
 * that visualize bot metrics and events.
 */

import { useContext, createContext } from "react";

// ============================================================================
// Types
// ============================================================================

export interface BotEvent {
  timestamp: Date;
  type: "log" | "metric";
  data: string | Record<string, unknown>;
  metricType?: string;
  level?: "DEBUG" | "INFO" | "WARN" | "ERROR";
  raw: string;
}

export interface BotEventUtils {
  /** Get events from the last duration (e.g., "1h", "24h", "7d") */
  since: (duration: string) => BotEvent[];
  /** Get events between two dates */
  between: (start: Date, end: Date) => BotEvent[];
  /** Filter events by metric type */
  filterByType: (metricType: string) => BotEvent[];
  /** Get only metric events */
  metrics: () => BotEvent[];
  /** Get only log events */
  logs: () => BotEvent[];
  /** Get the latest event of a specific metric type */
  latest: (metricType: string) => BotEvent | null;
  /** Group events by metric type */
  groupByMetricType: () => Record<string, BotEvent[]>;
  /** Group events by execution run (for scheduled bots) */
  groupByRun: (gapThresholdMs?: number) => BotEvent[][];
  /** Extract time series data for charting */
  extractTimeSeries: (
    metricType: string,
    valueKey?: string
  ) => { timestamp: Date; value: number }[];
}

export interface QuantFlowEventsContextValue {
  events: BotEvent[];
  loading: boolean;
  error: string | null;
  utils: BotEventUtils;
  refresh: () => void;
}

// ============================================================================
// Context (shared with main app via global)
// ============================================================================

declare global {
  interface Window {
    __QUANTFLOW_EVENTS_CONTEXT__?: React.Context<QuantFlowEventsContextValue | null>;
  }
}

// Get or create the shared context
function getSharedContext(): React.Context<QuantFlowEventsContextValue | null> {
  if (typeof window !== "undefined" && window.__QUANTFLOW_EVENTS_CONTEXT__) {
    return window.__QUANTFLOW_EVENTS_CONTEXT__;
  }

  const context = createContext<QuantFlowEventsContextValue | null>(null);

  if (typeof window !== "undefined") {
    window.__QUANTFLOW_EVENTS_CONTEXT__ = context;
  }

  return context;
}

export const QuantFlowEventsContext = getSharedContext();

// ============================================================================
// Hooks
// ============================================================================

/**
 * Hook to access bot events and utilities in custom dashboards.
 *
 * @example
 * ```tsx
 * import { useQuantFlowEvents } from '@quantflow/react';
 *
 * export default function Dashboard() {
 *   const { events, utils, loading } = useQuantFlowEvents();
 *
 *   const trades = utils.filterByType('trade');
 *   const latestPortfolio = utils.latest('portfolio_value');
 *
 *   return (
 *     <div>
 *       <h1>Portfolio: ${latestPortfolio?.data.value}</h1>
 *       <TradeList trades={trades} />
 *     </div>
 *   );
 * }
 * ```
 */
export function useQuantFlowEvents(): QuantFlowEventsContextValue {
  const context = useContext(QuantFlowEventsContext);

  if (!context) {
    throw new Error(
      "useQuantFlowEvents must be used within a quantflow dashboard context. " +
        "This hook only works in custom bot frontends loaded by quantflow platform."
    );
  }

  return context;
}

// Alias for convenience
export const useBotEvents = useQuantFlowEvents;

// ============================================================================
// Re-exports for convenience
// ============================================================================

export { QuantFlowEventsContext as BotEventsContext };
