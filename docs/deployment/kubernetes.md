---
title: "Kubernetes"
description: "Deploy quantflow platform on Kubernetes with Helm"
tags: ["deployment", "kubernetes", "helm", "production"]
order: 2
---

# Kubernetes Deployment


Kubernetes deployment provides production-ready orchestration for quantflow platform. Use Kubernetes mode when:

- Deploying across multiple nodes
- High availability is required
- You need K8s-native observability and management

The runtime uses a **controller pattern** in Kubernetes mode: each bot becomes its own Pod, and the controller manages the lifecycle by comparing desired state (MongoDB) with actual state (Pods) and making corrections.

## Prerequisites

### Required Tools

- **Docker** - For building container images ([install guide](https://docs.docker.com/get-docker/))
- **kubectl** - Kubernetes CLI ([install guide](https://kubernetes.io/docs/tasks/tools/))
- **Helm 3.0+** - Kubernetes package manager ([install guide](https://helm.sh/docs/intro/install/))
- **minikube** (local development) - Local Kubernetes cluster ([install guide](https://minikube.sigs.k8s.io/docs/start/))

### Cluster Requirements

**Minikube (development)**:
- 4GB RAM minimum
- 4 CPU cores
- 20GB+ disk space

**Production clusters**:
- 8+ CPU cores across nodes
- 16GB+ RAM
- 100GB+ disk for persistent volumes
- Enough capacity to run the API, frontend, docs, runtime controller, and bot Pods

## Install from Helm Repository

The Helm repository gives you the chart, not a complete production
environment. A real Kubernetes deployment must provide:

- PostgreSQL for users, bots, settings, and application data
- MongoDB for runtime desired state
- NATS for runtime coordination and streaming
- S3-compatible object storage for bot packages, logs, and backtests
- JWT signing configuration
- `QUANTFLOW_ADMIN_EMAIL` and a Secret-backed `QUANTFLOW_ADMIN_PASSWORD`

Start by adding the chart repository:

```bash
helm repo add quantflow https://quantflow-io.github.io/quantflow
helm repo update
```

Then prepare your backing services and values file before installing. The chart
does not provision production-grade PostgreSQL, MongoDB, or object storage for
you. Use the providers you trust, then pass their connection details to the
chart through Kubernetes Secrets.

Create the namespace and credentials Secret. This example uses one Secret named
`quantflow-secrets` for database, object storage, JWT, and root admin credentials.
For production, create the same Secret through your normal secret workflow such
as Sealed Secrets, External Secrets, or a protected Secret manifest.

Use connection strings from your backing services. The expected formats are:

```text
postgresql://<user>:<password>@<postgres-host>:5432/<database>?sslmode=require
mongodb://<user>:<password>@<mongo-host>:27017/<database>?authSource=admin
```

If your MongoDB provider gives you a `mongodb+srv://...` connection string, use
that full string for `MONGO_URL`.

```bash
kubectl create namespace quantflow --dry-run=client -o yaml | kubectl apply -f -

read -rsp "PostgreSQL DATABASE_URL: " DATABASE_URL; echo
read -rsp "MongoDB connection URL: " MONGO_URL; echo
read -rsp "Root admin password: " QUANTFLOW_ADMIN_PASSWORD; echo
read -rsp "JWT secret: " JWT_SECRET; echo
read -rsp "Object storage access key: " S3_ACCESS_KEY; echo
read -rsp "Object storage secret key: " S3_SECRET_KEY; echo

kubectl -n quantflow create secret generic quantflow-secrets \
  --from-literal=database-url="$DATABASE_URL" \
  --from-literal=mongo-url="$MONGO_URL" \
  --from-literal=minio-access-key="$S3_ACCESS_KEY" \
  --from-literal=minio-secret-key="$S3_SECRET_KEY" \
  --from-literal=jwt-secret="$JWT_SECRET" \
  --from-literal=admin-password="$QUANTFLOW_ADMIN_PASSWORD" \
  --dry-run=client -o yaml | kubectl apply -f -

unset DATABASE_URL
unset MONGO_URL
unset QUANTFLOW_ADMIN_PASSWORD
unset JWT_SECRET
unset S3_ACCESS_KEY
unset S3_SECRET_KEY
```

Avoid passing passwords with command-line literals in shell history. The command
above prompts interactively for generated secrets, but production GitOps
deployments should usually create `quantflow-secrets` from encrypted or external
secret sources.

Create `values.yaml`. This example assumes PostgreSQL, MongoDB, and object
storage are external, while NATS runs in the cluster with persistence:

```yaml
global:
  imagePullPolicy: Always
  existingSecret: "quantflow-secrets"

postgresql:
  enabled: false

mongodb:
  enabled: false

minio:
  enabled: false
  external:
    endpoint: "s3-compatible-endpoint.example.com"
    port: 443
    useSSL: true

nats:
  enabled: true
  persistence:
    enabled: true
    size: 5Gi

quantflowApi:
  image:
    repository: ghcr.io/quantflow-io/quantflow/api
  env:
    QUANTFLOW_ADMIN_EMAIL: "admin@example.com"
    NODE_ENV: "production"
    FRONTEND_URL: "https://quantflow.example.com"
    API_BASE_URL: "https://api.quantflow.example.com"
  extraEnv:
    - name: QUANTFLOW_ADMIN_PASSWORD
      valueFrom:
        secretKeyRef:
          name: quantflow-secrets
          key: admin-password

quantflowFrontend:
  image:
    repository: ghcr.io/quantflow-io/quantflow/frontend

quantflowDocs:
  image:
    repository: ghcr.io/quantflow-io/quantflow/docs

botController:
  image:
    repository: ghcr.io/quantflow-io/quantflow/runtime
  runtimeImagePullPolicy: Always

gc:
  image:
    repository: ghcr.io/quantflow-io/quantflow/runtime
```

For this external-services configuration, `quantflow-secrets` must contain:

| Key | Purpose |
| --- | --- |
| `database-url` | PostgreSQL connection string used by the API |
| `mongo-url` | MongoDB connection string used by the runtime controller and GC |
| `minio-access-key` | S3-compatible object storage access key |
| `minio-secret-key` | S3-compatible object storage secret key |
| `jwt-secret` | JWT signing secret shared by API and frontend |
| `admin-password` | Password exposed to the API as `QUANTFLOW_ADMIN_PASSWORD` |

Your object storage endpoint must be S3-compatible. Create the buckets expected
by your values before deploying, or keep the defaults from `k8s/values.yaml`:
`custom-bots`, `bot-logs`, and `backtests`. Set `minio.external.endpoint` to
the endpoint host only, without `https://`; `minio.external.port` and
`minio.external.useSSL` provide the connection scheme details.

Install only after the values and Secrets exist:

```bash
helm install quantflow quantflow/quantflow --namespace quantflow -f values.yaml
```

This installs the latest chart version. The chart's `appVersion` determines the default image tags, so you don't need to specify tags manually.

## Upgrading Existing Installations

Before upgrading to v1.14.0 or later, configure `QUANTFLOW_ADMIN_EMAIL` and `QUANTFLOW_ADMIN_PASSWORD` for the deployment-managed root admin. Use the email of the user that should become the root admin, and provide the password from a Secret as shown above.

Then upgrade through Helm:

```bash
helm repo update
helm upgrade quantflow quantflow/quantflow --namespace quantflow -f values.yaml
```

On startup:

- The API fails startup if either root admin variable is missing or invalid.
- If the configured email matches an existing user, the API promotes and activates that user, then applies the configured password when needed.
- If the configured email does not match an existing user, the API creates the root admin.
- If the configured password changes later, restarting or rolling out the API updates the root admin password once and invalidates old sessions.

See [Root Admin Configuration](./admin-bootstrap) for the full fresh install, upgrade, password rotation, and last-admin protection behavior.

## Quick Start with Minikube

For local development, minikube provides the simplest path to a running cluster:

If you need a completely fresh local environment, delete the minikube profile
first. Otherwise, persistent volumes from earlier runs may preserve database and
object storage state:

```bash
minikube delete
```

```bash
cd k8s

# Create local-only root admin values
cat > minikube-values.local.yaml <<'YAML'
quantflowApi:
  env:
    QUANTFLOW_ADMIN_EMAIL: "admin@example.com"
  extraEnv:
    - name: QUANTFLOW_ADMIN_PASSWORD
      valueFrom:
        secretKeyRef:
          name: quantflow-root-admin
          key: password
YAML

# Create the root admin password Secret
read -rsp "Root admin password: " QUANTFLOW_ADMIN_PASSWORD; echo
minikube start --memory=4096 --cpus=4 --disk-size=20g --driver=docker
kubectl create namespace quantflow --dry-run=client -o yaml | kubectl apply -f -
printf '%s' "$QUANTFLOW_ADMIN_PASSWORD" \
  | kubectl -n quantflow create secret generic quantflow-root-admin --from-file=password=/dev/stdin --dry-run=client -o yaml \
  | kubectl apply -f -
unset QUANTFLOW_ADMIN_PASSWORD

# Build local images inside minikube's Docker daemon
eval "$(minikube docker-env)"
docker build -t quantflow-api:latest ../api
docker build \
  --build-arg NEXT_PUBLIC_API_URL=http://api.quantflow.local:30000 \
  --build-arg NEXT_PUBLIC_DOCS_URL=http://docs.quantflow.local:30004 \
  -t quantflow-frontend:latest ../frontend
docker build -t quantflow-docs:latest ../docs
docker build -t runtime:latest ../runtime

# Deploy chart-managed PostgreSQL, MongoDB, NATS, and MinIO
helm upgrade --install quantflow . \
  --namespace quantflow \
  -f minikube-values.local.yaml \
  --set global.imagePullPolicy=IfNotPresent \
  --set quantflowApi.image.tag=latest \
  --set quantflowApi.imagePullPolicy=Never \
  --set quantflowFrontend.image.tag=latest \
  --set quantflowFrontend.imagePullPolicy=Never \
  --set quantflowDocs.image.tag=latest \
  --set quantflowDocs.imagePullPolicy=Never \
  --set botController.image.tag=latest \
  --set botController.runtimeImage=runtime:latest \
  --set gc.image.repository=runtime \
  --set gc.image.tag=latest \
  --set gc.image.pullPolicy=Never \
  --set minikube.enabled=true \
  --set externalServices.enabled=true

kubectl wait --for=condition=available deployment --all -n quantflow --timeout=240s
kubectl get pods -n quantflow
```

These commands perform these steps:

1. Start minikube with appropriate resources
2. Build all Docker images in minikube's Docker daemon
3. Create the root admin password Secret
4. Deploy all services with Helm using chart defaults plus `minikube-values.local.yaml`
5. Wait for all deployments to become available

After deployment, configure local DNS:

```bash
MINIKUBE_IP="$(minikube ip)"
sudo tee -a /etc/hosts >/dev/null <<EOF
$MINIKUBE_IP quantflow.local
$MINIKUBE_IP api.quantflow.local
$MINIKUBE_IP minio.quantflow.local
$MINIKUBE_IP docs.quantflow.local
EOF
```

This adds entries to `/etc/hosts` for the `.local` domains.

Verify the API and root admin login:

```bash
curl -fsS http://api.quantflow.local:30000/health
curl -fsS http://api.quantflow.local:30000/health/ready

curl -i \
  -H 'Content-Type: application/json' \
  -d '{"email":"admin@example.com","password":"<password you entered>"}' \
  http://api.quantflow.local:30000/auth/login
```

### Minikube Endpoints

| Service | URL | Description |
|---------|-----|-------------|
| Frontend | http://quantflow.local:30001 | Web dashboard |
| API | http://api.quantflow.local:30000 | REST API |
| MinIO Console | http://minio.quantflow.local:30002 | Object storage admin |
| Documentation | http://docs.quantflow.local:30004 | Platform docs |

## Production Deployment

For production clusters, deploy with external infrastructure:

```bash
helm repo add quantflow https://quantflow-io.github.io/quantflow
helm repo update
kubectl create namespace quantflow --dry-run=client -o yaml | kubectl apply -f -
helm upgrade --install quantflow quantflow/quantflow --namespace quantflow -f production-values.yaml
```

Do not run production with the chart defaults. Prepare `production-values.yaml`
and the referenced Secrets first.

### Using External Infrastructure

In production, you are responsible for providing PostgreSQL, MongoDB, and
S3-compatible object storage. Disable the chart-managed development services in
`values.yaml`:

```yaml
postgresql:
  enabled: false
mongodb:
  enabled: false
minio:
  enabled: false
nats:
  enabled: true
```

Put database connection strings, object storage credentials, and JWT signing
material in the Secret named by `global.existingSecret`. Do not put these
credentials directly in Helm values.

```yaml
global:
  existingSecret: "quantflow-secrets"

minio:
  enabled: false
  external:
    endpoint: "s3-compatible-endpoint.example.com"
    port: 443
    useSSL: true
```

Configure the root admin email and set the password from a Secret:

```yaml
quantflowApi:
  env:
    QUANTFLOW_ADMIN_EMAIL: "admin@example.com"
  extraEnv:
    - name: QUANTFLOW_ADMIN_PASSWORD
      valueFrom:
        secretKeyRef:
          name: quantflow-secrets
          key: admin-password
```

Do not put admin passwords in plaintext Helm values. `extraEnv` supports full Kubernetes `EnvVar` entries, including `secretKeyRef` values generated by Sealed Secrets. See [Root Admin Configuration](./admin-bootstrap) for fresh install, upgrade, password rotation, and last-admin protection.

## Helm Configuration

The chart is configured through `values.yaml`. Key sections:

### Global Settings

```yaml
global:
  imagePullPolicy: Never  # Never (local), Always (registry), IfNotPresent
  storageClass: ""        # Leave empty for default, or specify class name
  existingSecret: ""      # Name of an existing K8s Secret with credentials
```

Set `global.existingSecret` to the name of a pre-existing Kubernetes Secret containing your database URLs, object storage keys, and JWT secret. When set, the chart skips creating its own Secret and uses yours instead.

### Infrastructure Services

Each infrastructure service can be enabled/disabled and configured:

```yaml
postgresql:
  enabled: true
  image: postgres:15-alpine
  port: 5432
  persistence:
    enabled: true
    size: 10Gi
  resources:
    requests:
      memory: 256Mi
      cpu: 100m
    limits:
      memory: 512Mi
      cpu: 500m
```

Similar blocks exist for MongoDB, NATS, and MinIO.

### Application Services

Application services have environment variables matching the Docker Compose configuration:

```yaml
quantflowApi:
  enabled: true
  replicas: 1
  image:
    repository: quantflow-api
    tag: latest
  port: 3000
  env:
    DATABASE_URL: "postgresql://quantflow:quantflow_password@quantflow-postgres:5432/quantflow_oss"
    JWT_SECRET: "your-jwt-secret-key"
    # ... other environment variables
  resources:
    requests:
      memory: 512Mi
      cpu: 200m
    limits:
      memory: 1Gi
      cpu: 1000m
```

### Runtime Controller Configuration

In Kubernetes mode, the runtime runs as a controller that manages bots as Pods:

```yaml
botController:
  enabled: true
  image:
    repository: runtime
    tag: latest
  replicas: 1
  resources:
    requests:
      memory: 256Mi
      cpu: 100m
    limits:
      memory: 512Mi
      cpu: 500m
  env:
    NAMESPACE: quantflow
    RECONCILE_INTERVAL: "30s"
    MINIO_ENDPOINT: "quantflow-minio:9000"
```

The controller creates a Pod for each enabled bot. Bot Pods run with resource limits and are automatically restarted by Kubernetes if they fail.

### External Access

For minikube, NodePort services provide external access:

```yaml
externalServices:
  enabled: true
  nodePort:
    frontend: 30001
    api: 30000
    minio: 30002
    docs: 30004
```

For production clusters, use Ingress instead:

```yaml
ingress:
  enabled: true
  className: nginx
  hosts:
    - host: quantflow.yourdomain.com
      paths:
        - path: /
          service: quantflow-frontend
```

## Management Commands

Use `kubectl`, `helm`, and `minikube` directly while learning the deployment:

```bash
# Check deployment status
kubectl get deploy -n quantflow
kubectl get pods -n quantflow

# View service URLs
kubectl get svc -n quantflow
minikube service list

# View logs
kubectl logs -n quantflow deploy/quantflow-api
kubectl logs -n quantflow deploy/quantflow-bot-controller

# Pause or resume minikube
minikube pause
minikube unpause

# Stop minikube but preserve state
minikube stop

# Remove the Helm release and namespace
helm uninstall quantflow -n quantflow
kubectl delete namespace quantflow

# Full local cleanup
minikube delete
```

## Scaling

### Bot Scaling

In Kubernetes mode, each bot runs as its own Pod. The controller automatically creates and removes Pods based on the enabled bots in MongoDB. There's no need to manually scale workers - Kubernetes handles scheduling across nodes.

### API Scaling

Scale the API service by adjusting replica counts:

```yaml
quantflowApi:
  replicas: 3
```

Apply changes:

```bash
helm upgrade quantflow . --namespace quantflow
```

### Vertical Scaling

Adjust resource limits based on workload:

```yaml
quantflowApi:
  resources:
    requests:
      memory: 1Gi
      cpu: 500m
    limits:
      memory: 2Gi
      cpu: 2000m
```

## Health Checks

All services include liveness and readiness probes. The Helm chart configures these automatically based on each service's health endpoints.

API and frontend services expose HTTP health endpoints. Runtime services expose `/healthz` endpoints for Kubernetes to monitor.

## Networking

### Internal Communication

Services communicate internally via Kubernetes DNS:

- `quantflow-api.quantflow.svc.cluster.local:3000` - API service
- `quantflow-postgres.quantflow.svc.cluster.local:5432` - PostgreSQL
- `quantflow-nats.quantflow.svc.cluster.local:4222` - NATS

### External Access

**Minikube**: Uses NodePort services with fixed ports, accessed via `.local` domains after configuring `/etc/hosts`.

**Production**: Use an Ingress controller or cloud load balancer for external access. Configure TLS termination at the Ingress level.

## Troubleshooting

### Pods Not Starting

Check pod status and events:

```bash
kubectl get pods -n quantflow
kubectl describe pod <pod-name> -n quantflow
```

### Image Pull Failures

For local images, ensure minikube's Docker environment was used:

```bash
eval $(minikube docker-env)
docker images | grep quantflow
```

### Service Connectivity

Test internal DNS resolution:

```bash
kubectl run debug --image=busybox -it --rm --restart=Never -n quantflow -- sh
# Inside pod:
nslookup quantflow-api.quantflow.svc.cluster.local
```

### View Events

```bash
kubectl get events -n quantflow --sort-by='.lastTimestamp'
```

### Debug Pods

```bash
kubectl exec -it <pod-name> -n quantflow -- sh
kubectl logs <pod-name> -n quantflow
kubectl logs <pod-name> -n quantflow --previous  # Previous container logs
```

## Docker Compose vs Kubernetes

| Feature | Docker Compose | Kubernetes |
|---------|----------------|------------|
| Setup command | `quantflow local start` | `minikube` + `kubectl` + `helm` |
| Bot model | Containers on single host | Pod per bot |
| Recommended for | Single-host deployments | Multi-node, HA |
| Infrastructure | Included | Included (configurable) |
| Bot scaling | Single-process reconciliation | Controller + Pods |
| Health checks | Application level | Liveness + readiness probes |
| Service discovery | Container names | DNS-based |
| Persistence | Docker volumes | PersistentVolumes |
| Complexity | Simple | More complex |

Use Docker Compose for development and single-host production. Use Kubernetes when you need multi-node scaling or high availability.
