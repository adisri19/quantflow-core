---
title: "Secrets"
description: "Configure access to private package repositories"
tags: ["cli", "secrets", "github", "pypi", "private-repos"]
order: 5
---

# Secrets

When deploying custom bots with private dependencies, configure secrets to authenticate during the vendoring process. The CLI stores secrets locally and uses them during `quantflow custom-bot deploy`.

## Managing Secrets

### View Secrets

Display currently configured secrets (values are masked):

```bash
quantflow auth secrets show
```

### Set a Secret

Configure a build secret:

```bash
quantflow auth secrets set <name> <value>
```

### Clear All Secrets

Remove all saved secrets:

```bash
quantflow auth secrets clear
```

## Supported Secrets

| Secret | Description | Used By |
|--------|-------------|---------|
| `github-token` | GitHub Personal Access Token | Python, Node.js, Rust, Go |
| `pip-index-url` | Private PyPI index URL | Python |
| `npm-token` | npm registry token | Node.js |
| `nuget-token` | NuGet feed token | C# |
| `cargo-registry-token` | Cargo registry token | Rust |
| `maven-user` | Maven repository username | Scala |
| `maven-token` | Maven repository token | Scala |

## GitHub Private Repositories

For bots with dependencies from private GitHub repositories:

1. Create a Personal Access Token at [GitHub Settings > Developer settings > Personal access tokens](https://github.com/settings/tokens) with `repo` scope.

2. Set the secret:

```bash
quantflow auth secrets set github-token ghp_your_token_here
```

The CLI automatically rewrites git URLs to use HTTPS authentication during vendoring. All common URL formats are supported:

```txt
git+ssh://git@github.com/org/repo.git
git+https://github.com/org/repo.git
git+git@github.com:org/repo.git
```

## Private PyPI

For private Python package indexes:

```bash
quantflow auth secrets set pip-index-url https://user:pass@pypi.internal.com/simple/
```

Include credentials in the URL. The CLI passes this to pip during vendoring.

## Private npm Registry

For private Node.js packages:

```bash
quantflow auth secrets set npm-token npm_your_token_here
```

## Multiple Secrets

Configure multiple secrets for complex dependency setups:

```bash
quantflow auth secrets set github-token ghp_xxx
quantflow auth secrets set pip-index-url https://user:pass@pypi.internal.com/simple/
```

## Storage

Secrets are stored in `~/.quantflow/secrets.json` with restricted file permissions (0600). This file is never uploaded to the platform—secrets are only used locally during the vendoring process.

## Environment Variables

As an alternative to stored secrets, you can set environment variables before deploying:

```bash
export GITHUB_TOKEN="ghp_xxx"
export PIP_INDEX_URL="https://user:pass@pypi.internal.com/simple/"
quantflow custom-bot deploy
```

Environment variables take precedence over stored secrets.

## Troubleshooting

**"Permission denied (publickey)"**

Git is trying SSH authentication. Set `github-token` to enable HTTPS:

```bash
quantflow auth secrets set github-token ghp_xxx
```

**"Repository not found"**

Your token lacks access to the repository. Verify:
- The token has `repo` scope
- You have access to the repository

**"Could not find a version that satisfies the requirement"**

The private package can't be found. Check:
- `pip-index-url` is set correctly
- The package exists in your private index
- Credentials are valid

## Related

- [Custom Bot Commands](./custom-bot-commands) - Deploy custom bots
- [Authentication](./authentication) - CLI authentication
