---
title: "quantflow CLI"
description: "Command-line interface for quantflow platform"
order: 3
---

# quantflow CLI

The `quantflow` CLI is the primary interface for deploying custom bots and managing bot instances on quantflow platform. It handles authentication, dependency vendoring, compilation, and deployment.

## Command Groups

The CLI is organized into the following command groups:

**auth** - Manage authentication

```bash
quantflow auth login          # Set API key for the active environment
quantflow auth logout         # Remove the API key for the active environment
quantflow auth status         # Check authentication status
quantflow auth secrets        # Manage build secrets
```

**env** - Manage named API environments

```bash
quantflow env add <name> --url <url>  # Add a new environment (validates key)
quantflow env use <name>              # Switch active environment
quantflow env list                    # List all environments
quantflow env remove <name>           # Delete an environment
quantflow env current                 # Show the active environment
```

**custom-bot** - Deploy and manage custom bot definitions

```bash
quantflow custom-bot deploy           # Deploy from current directory
quantflow custom-bot list             # List deployed custom bots
quantflow custom-bot versions <name>  # List versions of a custom bot
quantflow custom-bot schema <version> <name>  # Get schema for a custom bot
```

**bot** - Manage bot instances

```bash
quantflow bot deploy <config.json>    # Deploy a bot instance
quantflow bot list                    # List bot instances
quantflow bot update <id> <config>    # Update instance configuration
quantflow bot delete <id>             # Delete an instance
quantflow bot logs <id>               # View instance logs
```

**local** - Manage local Docker Compose development environment

```bash
quantflow local init                         # Initialize local environment and prompt for root admin credentials
quantflow local init --email <email> --password <password>
quantflow local start                        # Start all services
quantflow local reset-admin-password <password>
quantflow local stop                         # Stop all services
quantflow local status                       # Show service status
quantflow local logs [service]               # View service logs
quantflow local dev                          # Start in development mode
```

**update** - Self-update the CLI

```bash
quantflow update                # Update to latest version
```

## Global Flags

All commands support the following flags:

```bash
-v, --verbose      Enable verbose output
    --env <name>   Use the named environment for this command
-h, --help         Show help for any command
```

## Documentation

- [Installation](./installation) - Install the CLI
- [Authentication](./authentication) - Configure API credentials
- [Environments](./environments) - Manage multiple API endpoints (local, prod, ...)
- [Bot Commands](./bot-commands) - Bot instance management reference
- [Custom Bot Commands](./custom-bot-commands) - Custom bot deployment reference
- [Local Development](./local-development) - Local environment management reference
- [Secrets](./secrets) - Configure private dependencies
