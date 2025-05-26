# 6GDetCom Simulator Framework
A simulator framework for validating the concepts for a wireless-friendly design for end-to-end deterministic communication.

## Acknowledgments

This software is part of the deliverables
D4.1 "DetCom Simulator Framework (Release 1)" and
D4.1 "DetCom Simulator Framework (Release 1)" of the DETERMINISTIC6G project,
which has received funding from the European Union's Horizon Europe research
and innovation programme under grant agreement No. 101096504.

[DETERMINISTIC6G Project Website](https://deterministic6g.eu/).

DETERMINISTIC6G e-mail: coordinator@deterministic6g.eu

## License

The software is licensed under the [GNU LESSER GENERAL PUBLIC LICENSE Version 3](LICENSE.md).

If you use this software, please cite the following paper:

Haug, Lucas; Dürr, Frank; Egger, Simon; Grohmann, Lorenz; Gross, James; Sharma, Gourav Prateek; Sachs, Joachim: Simulating and Emulating the Characteristic Packet Delay of Logical 5G TSN Bridges. 2025, https://doi.org/10.15496/PUBLIKATION-105097.


## Compatibility
This version of the D6G framework is compatible with OMNeT++ 6.1 and a current INET master branch build
(NOTE: current release 4.5.4 is not compatible).

## Getting Started
There are two methods to use this project.

1. Run the [install.sh](install.sh) script to automatically install the framework and its dependencies (Linux only).
2. Follow the [manual installation](#manual-installation) instructions below.


## Manual Installation
The following guide assumes you already have a working OMNeT++ installation (see [here](doc/install-omnetpp.md) for instructions).
The following guide assumes you already have a working OMNeT++ installation (see [here](doc/install-omnetpp.md) for instructions).


### Workspace Setup
Your workspace should have the following structure:
```
[path_to_your_workspace]
├── 6GDetCom_Simulator
├── deterministic6g_data (optional)
└── inet
```

### INET installation
If you already have INET installed, you can skip this step.

1. Clone the INET repository with the correct version (make sure to follow the correct [Workspace Setup](#workspace-setup)).
NOTE: The current INET release (4.5.4) is not compatible with this version.
We tested this release with the following commit of the INET master branch:
https://github.com/inet-framework/inet/commit/6211cd63ebe479b9b01f5ccecc632d5c2e1952af
Newer master commits may work.
```shell
git clone https://github.com/inet-framework/inet.git
git checkout 6211cd63ebe479b9b01f5ccecc632d5c2e1952af
```
We hope necessary fixes of the master branch will soon be included in an INET release.
This documentation will be updated then.

2. Build INET:
```shell
cd inet
source setenv
make makefiles
make -j$(nproc)
```

### Deterministic6G installation
1. Make sure this project is in your workspace (see [Workspace Setup](#workspace-setup)).
2. Build the Deterministic6G framework:
```shell
cd deterministic6g
make makefiles
make -j$(nproc)
```

### Working in the OMNeT++ IDE
1. Open/Create the workspace located at `[path_to_your_workspace]`.
2. Open the INET project (File > Open Projects from File System... > Directory... > `[path_to_your_workspace]/inet` > Finish)
3. Open the D6G project (File > Open Projects from File System... > Directory... > `[path_to_your_workspace]/deterministic6g` > Finish)

## Run simulations

### Using the IDE
1. Open the `omnetpp.ini` file of the simulation you want to run.
2. Click on the green run button in the top toolbar.
