---
title: "v1.14.0 Root Admin Migration"
description: "Move existing deployments to deployment-managed root admin credentials"
tags: ["migration", "deployment", "auth", "admin"]
order: 1
---

# v1.14.0 Root Admin Migration

v1.14.0 removes public registration and introduces an explicit admin role model. The API now requires a deployment-managed root admin on every startup:

- `QUANTFLOW_ADMIN_EMAIL`
- `QUANTFLOW_ADMIN_PASSWORD`

If either value is missing, empty, or invalid, the API fails startup and writes the reason to API logs. The frontend stays on login and does not expose bootstrap errors.

The configured root admin is managed by deployment configuration. Its email and password cannot be changed through the UI. To rotate the password, update deployment configuration and restart or roll out the API.

## Who Must Act

You must update configuration before upgrading if:

- You run an existing quantflow deployment.
- Your deployment relied on public registration.
- Your local Docker Compose files were generated before v1.14.0.
- Your Kubernetes values do not set `QUANTFLOW_ADMIN_EMAIL` and secret-backed `QUANTFLOW_ADMIN_PASSWORD`.

Fresh deployments also need these values. There is no no-config first-user path in v1.14.0.

## What Changes

| Before v1.14.0 | v1.14.0 and later |
| --- | --- |
| Public registration could create ordinary user accounts | Public registration is removed |
| Users did not have an enforced `admin`/`user` role column | Users have explicit roles and admin-only management routes |
| Any existing user password was user-managed | The configured root admin password is deployment-managed |
| Missing root admin config was not a startup concern | Missing root admin config fails API startup |

Existing users are preserved. The email you configure becomes the deployment-managed root admin: if it matches an existing user, that user is promoted and the configured password is applied; if it does not match, a new root admin user is created.

The old seed script stored `metadata.role = "admin"` on its seeded user even though there was no enforced role column. During startup, v1.14.0 preserves that legacy marker by promoting those users before syncing the configured root admin.

## Choose The Root Admin

Pick the email address that should be deployment-managed:

- If that user already exists, the API promotes and activates the user, then applies `QUANTFLOW_ADMIN_PASSWORD`.
- If that user does not exist, the API creates a new active admin for that email.
- If the password already matches, the API does not update the user again.

Use a real operator-controlled email address. Do not use a shared throwaway email for production.

## Docker Compose

Update the CLI first, then refresh the generated Compose files and configure the root admin.

For a source-mode local install:

```bash
quantflow local init --source /path/to/quantflow --email admin@example.com --password 'new-password'
quantflow local start
```

For a prebuilt-image local install:

```bash
quantflow local init --email admin@example.com --password 'new-password'
quantflow local start
```

Rerunning `quantflow local init` is important for older installs because it refreshes `~/.quantflow/compose/docker-compose.yml`. Older generated Compose files may not pass `QUANTFLOW_ADMIN_PASSWORD` into `quantflow-api`.

To rotate the local root admin password later:

```bash
quantflow local reset-admin-password 'new-password'
```

Then verify the stack:

```bash
quantflow local status
```

If `quantflow-api` fails to start, inspect:

```bash
quantflow local logs api
```

## Kubernetes

Set the root admin email in values and provide the password through a Secret. Do not put the password in plaintext Helm values.

Create or update the Secret:

```bash
kubectl create secret generic quantflow-root-admin \
  --namespace quantflow \
  --from-literal=password='new-password' \
  --dry-run=client \
  -o yaml | kubectl apply -f -
```

Configure values:

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

Upgrade:

```bash
helm upgrade quantflow ./k8s --namespace quantflow --values values.yaml
kubectl rollout status deploy/quantflow-api --namespace quantflow
```

To rotate the Kubernetes root admin password later, update the Secret and restart or roll out the API:

```bash
kubectl create secret generic quantflow-root-admin \
  --namespace quantflow \
  --from-literal=password='new-password' \
  --dry-run=client \
  -o yaml | kubectl apply -f -

kubectl rollout restart deploy/quantflow-api --namespace quantflow
kubectl rollout status deploy/quantflow-api --namespace quantflow
```

## Verify The Migration

Check API readiness:

```bash
curl -fsS http://localhost:3000/health/ready
```

Then sign in with:

- Email: `QUANTFLOW_ADMIN_EMAIL`
- Password: `QUANTFLOW_ADMIN_PASSWORD`

The authenticated user should have:

- `role: admin`
- `isConfiguredRootAdmin: true`

The selected user's previous password should stop working after migration or a configured password rotation.

## Troubleshooting

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| API fails with `QUANTFLOW_ADMIN_EMAIL must be configured` | Email env is missing or empty | Set `QUANTFLOW_ADMIN_EMAIL` and restart the API |
| API fails with `QUANTFLOW_ADMIN_PASSWORD must be configured` | Password env is missing or empty | Set `QUANTFLOW_ADMIN_PASSWORD` and restart the API |
| Docker Compose has password in `.env` but API still fails | Generated Compose file is stale | Rerun `quantflow local init ...` to refresh Compose files |
| Kubernetes login still uses old password | Pod has not restarted with updated Secret | Roll out or restart `quantflow-api` |
| Login works but root admin cannot change password in UI | Expected behavior | Rotate root admin password through deployment configuration |

See [Root Admin Configuration](/deployment/admin-bootstrap) for the full behavior matrix and last-admin protection rules.
