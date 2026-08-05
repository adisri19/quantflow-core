---
title: "Environments"
description: "Manage named API environments with their own URL and credentials"
tags: ["cli", "environments", "api"]
order: 3
---

# Environments

Named environments let the CLI target multiple quantflow API endpoints (for example, a local Docker Compose stack and a production cluster) without rewriting environment variables or overwriting your saved API key.

Each environment stores its own:

- **URL** - e.g. `http://localhost:3000` or `https://api.quantflow.io`
- **API key** - validated against the URL before it is saved

Environments live in `~/.quantflow/environments.json`.

## Add an Environment

```bash
quantflow env add local --url http://localhost:3000
quantflow env add prod  --url https://api.quantflow.io --api-key quantflow_xxxxxxxxxxxx
```

If `--api-key` is omitted, the CLI prompts for it. The key is tested against the URL and only persisted if the API accepts it, so typos fail fast.

The first environment you add becomes the active one automatically. If you previously logged in with `quantflow auth login`, that existing credential is migrated into a `default` environment so nothing is lost.

## Switch the Active Environment

```bash
quantflow env use prod
```

All subsequent commands use the new active environment.

## One-off Override

Every command accepts a global `--env <name>` flag that overrides the active environment for that single invocation:

```bash
quantflow bot list --env prod
quantflow custom-bot deploy --env local
```

## List Environments

```bash
quantflow env list
```

```
ACTIVE  NAME   URL
*       local  http://localhost:3000
        prod   https://api.quantflow.io
```

API keys are never printed.

## Show the Active Environment

```bash
quantflow env current
```

## Remove an Environment

```bash
quantflow env remove prod
```

If you remove the active environment, no environment is active until you run `quantflow env use <name>`.

## Resolution Precedence

When the CLI needs a URL and API key, it resolves them in this order (highest wins):

1. `--env <name>` flag passed on the command
2. Active environment in `environments.json`
3. Legacy `QUANTFLOW_API_URL` env var + `~/.quantflow/auth.json` (back-compat for users who haven't run `quantflow env add` yet)
4. Default URL `http://localhost:3000`

## Migrating From `QUANTFLOW_API_URL`

The `QUANTFLOW_API_URL` environment variable and the old `quantflow auth config` command have been replaced by named environments. Existing setups keep working until you run your first `quantflow env add`, which migrates your legacy credential into a `default` environment.

## Next Steps

- [Authentication](./authentication) - `auth login` writes to the active environment
- [Installation](./installation) - Install the CLI
