---
title: "Installation"
description: "How to install the quantflow CLI"
tags: ["cli", "installation"]
order: 1
---

# Installation

The `quantflow` CLI is a Go binary that can be installed with a single command or built from source.

## Prerequisites

To deploy bots that use Python or Node.js dependencies, you need Docker installed and running. The CLI uses Docker to vendor dependencies in a consistent Linux environment.

## Quick Install (Recommended)

Install with a single command:

```bash
curl -sSL https://install.quantflow.io | sh
```

This downloads the latest release binary, verifies its SHA256 checksum, and installs to `~/.quantflow/bin`. The script automatically configures your PATH.

Supported platforms:
- Linux (amd64, arm64)
- macOS (amd64, arm64)

## Install from Source

Building from source requires Go 1.21 or later.

```bash
# Clone the repository
git clone https://github.com/quantflow-io/quantflow.git
cd quantflow/cli

# Build and install to ~/bin
make install
```

This compiles the CLI and copies it to `~/bin/quantflow`. Ensure `~/bin` is in your PATH:

```bash
export PATH="$HOME/bin:$PATH"
```

Add this line to your shell configuration file (`~/.bashrc`, `~/.zshrc`, etc.) to make it permanent.

## Updating

The CLI can update itself to the latest release:

```bash
quantflow update
```

This checks GitHub Releases, downloads the new binary with checksum verification, and replaces the current binary.

## Verify Installation

Confirm the CLI is installed correctly:

```bash
quantflow --version
quantflow --help
```

The help command displays all available commands and global flags.

## Configuration

The CLI stores configuration in `~/.quantflow/`:

- `environments.json` - named API environments and their credentials (see [Environments](./environments))
- `auth.json` - legacy single-environment credential file; still honoured when no named environments exist

No manual configuration is required. The CLI creates this directory automatically when you first authenticate or run `quantflow env add`.

## Next Steps

- [Authentication](./authentication) - Configure your API credentials
- [Environments](./environments) - Target multiple API endpoints
- [Custom Bot Commands](./custom-bot-commands) - Deploy your first bot
