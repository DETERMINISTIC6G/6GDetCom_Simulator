# Agent Instructions

This document provides instructions for AI agents working on this project. It is based on the main `README.md` file and GitHub workflow files.

## 1. Environment Setup

All development and testing must be performed inside the specified Docker container.

**Docker Image:** `ghcr.io/omnetpp/omnetpp:u24.04-6.2.0`

Start the container by mounting your workspace directory. The workspace should be the parent directory of the `6GDetCom_Simulator` project.

```bash
# Execute this command from your workspace directory
docker run -it --rm -v "$(pwd):/root/workspace" -w /root/workspace ghcr.io/omnetpp/omnetpp:u24.04-6.2.0
```

Inside the container, you may need to install some tools:
```bash
apt-get update && apt-get install -y wget unzip
```

## 2. Workspace Setup

Inside the container, your workspace should have the following structure:

```
/root/workspace
├── 6GDetCom_Simulator
├── deterministic6g_data
└── inet
```

The following commands will set up the workspace correctly.

### Clone `6GDetCom_Simulator` (if not already present)
This project repository should be cloned into your workspace.

### Clone `inet` dependency
```bash
# From inside the container, in /root/workspace
# The full git history is needed for the setenv script to work correctly
git clone https://github.com/DETERMINISTIC6G/inet-gptp inet
```

### Download `deterministic6g_data`
```bash
# From inside the container, in /root/workspace
wget https://github.com/DETERMINISTIC6G/deterministic6g_data/releases/latest/download/histograms_for_omnetpp.zip
unzip histograms_for_omnetpp.zip -d deterministic6g_data
rm histograms_for_omnetpp.zip
```

## 3. Build Instructions

Follow these steps to build the dependencies and the main project.

### Build `inet`
```bash
# From inside the container, in /root/workspace
cd inet
source setenv
# Note: The path to requirements.txt might differ. This assumes it's in the root of inet.
pip install -r requirements.txt
make makefiles
make -j$(nproc)
cd ..
```

### Build `6GDetCom_Simulator`
```bash
# From inside the container, in /root/workspace
cd 6GDetCom_Simulator
source ../inet/setenv
make makefiles
make -j$(nproc)
cd ..
```
