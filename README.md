# QuantFlow

**Open-Source Algorithmic Trading Platform**
*Production-grade bot execution engine for quantitative trading*

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Docker](https://img.shields.io/badge/Docker-Ready-2496ED?logo=docker)](https://www.docker.com/)

---

## What is QuantFlow?

**QuantFlow** is an open-source algorithmic trading execution engine that provides production-grade infrastructure for deploying and managing trading bots across multiple markets. Build strategies in your preferred language, then deploy them to a self-hosted execution engine with real-time monitoring and custom React dashboards.

### Key Features
*   **Multi-Language Support** — Build bots in Python, TypeScript, Rust, C++, C#, Scala, or Haskell.
*   **Custom Frontends** — Create React dashboards tailored to your trading strategies.
*   **Real-time Execution** — Deploy scheduled or continuous trading bots with isolated execution.
*   **Exchange Agnostic** — Design your bots to work with any trading platform.
*   **Self-Hosted** — Fully Docker and Kubernetes ready for production deployment.

---

## Quick Start (Docker Compose)

Deploy QuantFlow locally:

```bash
# Install the CLI tool
curl -sSL https://install.quantflow.io | sh

# Initialize local compose and configure the admin user
quantflow local init --email you@example.com --password testuse123

# Start the platform services
quantflow local start
```

Once started, access the components at:
*   **Web Dashboard**: `http://localhost:3001`
*   **API Gateway**: `http://localhost:3000`
*   **MinIO Console**: `http://localhost:9001` (admin / quantflowpassword)

---

## Bot Development & SDKs

QuantFlow supports strategy development with native SDK bindings:

| Language | SDK Path | Package Name |
|----------|----------|--------------|
| **Python** | [sdk/python](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/python) | `quantflow-sdk` |
| **TypeScript/Node.js** | [sdk/nodejs](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/nodejs) | `@quantflow/node` |
| **Rust** | [sdk/rust](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/rust) | `quantflow-sdk` |
| **C++** | [sdk/cpp](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/cpp) | Header-only (FetchContent) |
| **C#** | [sdk/dotnet](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/dotnet) | `QuantFlow.Sdk` |
| **Scala** | [sdk/scala](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/scala) | `quantflow-sdk` |
| **Haskell** | [sdk/haskell](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/haskell) | `quantflow-sdk` |
| **React (UI)** | [sdk/react](file:///Users/adityasrivastava/Downloads/the0-dev/sdk/react) | `@quantflow/react` |

---

## Project Structure

*   [api](file:///Users/adityasrivastava/Downloads/the0-dev/api) — NestJS Backend API server.
*   [frontend](file:///Users/adityasrivastava/Downloads/the0-dev/frontend) — Next.js 15 Web Dashboard interface.
*   [runtime](file:///Users/adityasrivastava/Downloads/the0-dev/runtime) — Go microservices (Scheduler & Runner).
*   [cli](file:///Users/adityasrivastava/Downloads/the0-dev/cli) — Go CLI application management utility.
*   [k8s](file:///Users/adityasrivastava/Downloads/the0-dev/k8s) — Helm chart configurations for Kubernetes.
*   [docs](file:///Users/adityasrivastava/Downloads/the0-dev/docs) — Detailed documentation pages.

---

## License

This project is licensed under the Apache License 2.0. Original copyright by [Alpha Neuron](https://the0.app). Rebranded modifications by QuantFlow Team.
