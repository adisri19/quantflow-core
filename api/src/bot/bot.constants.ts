export const BOT_TYPE_PATTERN =
  /^[a-z0-9]+(-[a-z0-9]+)*\/[a-z0-9]+(-[a-z0-9]+)*$/;

// NATS topics for bot events (match runtime expectations)
export const BOT_TOPICS = {
  CREATED: "quantflow.bot.created",
  UPDATED: "quantflow.bot.updated",
  DELETED: "quantflow.bot.deleted",
} as const;

// NATS topics for scheduled bot events
export const SCHEDULED_BOT_TOPICS = {
  CREATED: "quantflow.bot-schedule.created",
  UPDATED: "quantflow.bot-schedule.updated",
  DELETED: "quantflow.bot-schedule.deleted",
} as const;

// NATS topics for bot log streaming
const BOT_LOG_PREFIX = "quantflow.bot.logs";

export const BOT_LOG_TOPICS = {
  PREFIX: BOT_LOG_PREFIX,
  forBot(botId: string) {
    return `${BOT_LOG_PREFIX}.${botId}`;
  },
} as const;
