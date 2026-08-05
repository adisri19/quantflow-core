---
title: "Root Admin Configuration"
description: "Configure the quantflow root administrator"
tags: ["deployment", "auth", "admin"]
order: 3
---

# Root Admin Configuration

quantflow does not allow public registration. Every deployment must configure one root admin with:

- `QUANTFLOW_ADMIN_EMAIL`
- `QUANTFLOW_ADMIN_PASSWORD`

The frontend always starts at login. If either variable is missing, empty, or invalid, the API fails startup. The error is written to API logs and is not exposed through the frontend.

## How It Works

On startup, the API treats the configured root admin as deployment-managed state:

1. If the database has no users, the API creates the root admin automatically.
2. If users exist and the configured email matches a user, the API promotes that user to `admin`, activates the account, and applies the configured password when the hash differs.
3. If users exist and the configured email does not match a user, the API creates a new active admin for that email.
4. If the configured password already matches, the API does not update the user or increment `session_version`.

The configured root admin's email and password cannot be changed through the UI or API. Rotate the root admin password by updating `QUANTFLOW_ADMIN_PASSWORD` in deployment configuration and restarting or rolling out the API.

Additional admins and normal users are managed from `/settings/users` after login.

If you are upgrading an existing deployment, follow the [v1.14.0 Root Admin Migration Guide](/migration-guides/v1-14-root-admin) before rolling out the new API.

## Behavior Matrix

| State | Configuration | Startup behavior |
|-------|---------------|------------------|
| Fresh install, no config | none | API fails startup |
| Fresh install, email/password config | `QUANTFLOW_ADMIN_EMAIL` and `QUANTFLOW_ADMIN_PASSWORD` | Creates the root admin automatically |
| Upgrade, users exist, configured email matches a user | email/password for that existing user | Promotes and activates that user, then sets the password |
| Upgrade, users exist, configured email does not match a user | email/password for a new root admin | Creates a new active root admin |
| Upgrade or startup with email only | `QUANTFLOW_ADMIN_EMAIL` only | API fails startup |
| Configured root admin already matches config | same email and same password | No mutation |
| Configured root admin password rotated in config | same email and changed password | Updates password once and increments `session_version` |

v1.14.0 introduces the enforced admin role column. During upgrades, the API also preserves the old seed-script `metadata.role = "admin"` marker by promoting those users before syncing the configured root admin.

## Docker Compose

For local Docker Compose installs, initialize with root admin credentials:

```bash
quantflow local init --email you@example.com --password testuse123
quantflow local start
```

If flags are omitted, `quantflow local init` prompts for the email and password interactively.

To rotate the local root admin password:

```bash
quantflow local reset-admin-password new-password
```

This updates `~/.quantflow/compose/.env` and restarts `quantflow-api`. The CLI only validates that values can be safely written to `.env`; password policy validation happens in the API at startup.

## Kubernetes

Set the root admin email as a normal environment variable and the password from a Secret:

```yaml
quantflowApi:
  env:
    QUANTFLOW_ADMIN_EMAIL: "admin@example.com"
  extraEnv:
    - name: QUANTFLOW_ADMIN_PASSWORD
      valueFrom:
        secretKeyRef:
          name: quantflow-root-admin
          key: password
```

`quantflowApi.extraEnv` accepts full Kubernetes `EnvVar` entries, so it works with normal Secrets, External Secrets, Sealed Secrets, and similar controllers. Do not put admin passwords in plaintext Helm values.

To rotate the root admin password, update the referenced Secret and restart or roll out the API deployment.

## Last Admin Protection

quantflow prevents changes that would leave the platform without an active admin:

- An admin cannot demote their own account.
- An admin cannot deactivate their own account.
- The final active admin cannot be demoted, deactivated, or deleted.
- API keys cannot call admin user-management endpoints.

Use `/settings/users` as an admin to create users, assign roles, deactivate accounts, and reset passwords for non-root users.
