# Docker Setup

This folder contains the necessary files and scripts to build and run a Docker container for the project. The container is designed to provide a consistent environment for running experiments and scripts.

## Files in this Folder

- **Dockerfile**: The Dockerfile defines the container's environment, including dependencies and configurations.
- **docker_helper.sh**: A helper script to simplify building and running the Docker container.

## Using the `docker_helper.sh` Script

The `docker_helper.sh` script automates the process of building and running the Docker container. It mounts the project directory into the container for persistence and ensures the container has access to the host's display (useful for GUI applications).

### Prerequisites

- Docker must be installed on your machine. If you don't have Docker installed, follow the official [Docker installation guide](https://docs.docker.com/get-docker/).
- Ensure your user has permission to run Docker commands (e.g., by adding your user to the `docker` group [linux guide](https://docs.docker.com/engine/install/linux-postinstall/)).

### Script Usage

The script supports two commands: `build` and `run`.

#### 1. Build the Docker Image

To build the Docker image, run:

```bash
./docker_helper.sh build
```

This will:
1. Apply a necessary patch to a file in the `HybVIO` dependency.
2. Build the Docker image using the provided `Dockerfile`.

#### 2. Run the Docker Container

To start the Docker container, run:

```bash
./docker_helper.sh run
```

This will:
1. Start the container in interactive mode (`-it`).
2. Mount the project directory into the container for persistence.
3. Grant the container privileged access (`--privileged`) and host network access (`--net=host`).
4. Enable GUI applications by mounting the host's X11 socket and setting the `DISPLAY` environment variable.

Once the container is running, you can attach to it and manually execute scripts as needed.

## Notes for Users

- **Manual Script Execution**: The container is designed for manual interaction. After starting the container, attach to it (using `docker_helper.sh`) and run the scripts as needed. This approach provides a consistent environment.

## Troubleshooting

- **Permission Issues**: If you encounter permission issues while running Docker commands, ensure your user is part of the `docker` group. You may need to log out and log back in for the changes to take effect.
- **GUI Applications**: If GUI applications fail to launch, ensure the `DISPLAY` environment variable is set correctly and the X11 socket is properly mounted.
