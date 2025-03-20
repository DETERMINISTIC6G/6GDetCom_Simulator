# 6GDetCom Simulator Framework
A simulator framework for validating the concepts for a wireless-friendly design for end-to-end deterministic communication.

## Acknowledgments

This software is part of the Deliverable D4.1 "DetCom Simulator Framework (Release 1)" of the DETERMINISTIC6G project, which has received funding from the European Union's Horizon Europe research and innovation programme under grant agreement No. 101096504.

[DETERMINISTIC6G Project Website](https://deterministic6g.eu/).

DETERMINISTIC6G e-mail: coordinator@deterministic6g.eu

## License

The software is licensed under the [GNU LESSER GENERAL PUBLIC LICENSE Version 3](LICENSE.md).

If you use this software, please cite the following paper:

G. P. Sharma, D. Patel, J. Sachs, M. De Andrade, J. Farkas, J. Harmatos, B. Varga, H. -P., Bernhard, R. Muzaffar, M. Ahmed, F. Dürr, D. Bruckner, E.M. De Oca, D. Houatra, H. Zhang and J. Gross: Toward Deterministic Communications in 6G Networks: State of the Art, Open Challenges and the Way Forward. IEEE Access, vol. 11, pp. 106898-106923, 2023, doi: 10.1109/ACCESS.2023.3316605


## Compatibility
This version of the D6G framework is compatible with OMNeT++ 6.0 and INET 4.5.2.

## Getting Started
There are two methods to use this project.

1. Run the [install.sh](install.sh) script to automatically install the framework and its dependencies (Linux only).
2. Follow the [manual installation](#manual-installation) instructions below.


## Manual Installation
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

1. Clone the INET repository with the correct version (make sure to follow the correct [Workspace Setup](#workspace-setup)):
```shell
git clone --branch v4.5.2 https://github.com/inet-framework/inet.git
```

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
