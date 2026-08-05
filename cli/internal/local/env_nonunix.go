//go:build !unix

package local

import "quantflow/internal/logger"

func detectDockerSocketGID() string {
	logger.Verbose("Docker socket GID detection is not supported on this platform, using default")
	return "0"
}
