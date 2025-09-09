# Installing OMNeT++ and INET

The following descriptions have been tested with Debian 12, but should also work with other Linux distributions.

Information for other Linux distributions (Ubuntu) and platforms (Windows, macOS) as well as a more detailed description
can be found in the official [OMNeT++ Installation Guide](https://doc.omnetpp.org/omnetpp/InstallGuide.pdf).

## Installing OMNeT++

We are using OMNeT++ 6.0.1, although other versions of OMNeT++ might also work.

Install required packages using the package manager:

```
$ sudo apt-get install build-essential clang lld gdb bison flex perl python3 python3-pip qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5opengl5-dev libxml2-dev zlib1g-dev doxygen graphviz libwebkit2gtk-4.0-37
```

OMNeT++ requires a number of Python packages. Some of these packages are not available through Debian packages (apt),
therefore, we will use pip to install these Python packages. Since Debian 12, installing Python packages with pip by
users requires virtual Python environments. Therefore, we first create a virtual environment `venv-omnetpp`, activate
it, and then from within the environment install all Python packages using pip:

```
$ python3 -m venv ~/venv-omnetpp
$ source ~/venv-omnetpp/bin/activate
(venv-omnetpp) $ pip install numpy pandas matplotlib scipy seaborn posix-ipc
```

Download `omnetpp-6.0.1-linux-x86_64.tgz` from the [OMNeT++ web page](https://omnetpp.org/).

Extract archive to a folder of your choice. In the following, we install OMNeT++ directly to the home directory `~/` of
the user and refer to the OMNeT++ folder as `~/omnetpp-6.0.1`:

```
$ cd ~/
$ tar xzf omnetpp-6.0.1-linux-x86_64.tgz
$ cd ~/omnetpp-6.0.1
```

Edit `configure.user`:

* Exclude Open Scene Graph: `WITH_OSG=no`

Compile OMNeT++. These steps must be executed from within the virtual Python environment `venv-omnetpp` that you created
and activated above:

```
(venv-omnetpp) $ source setenv
(venv-omnetpp) $ ./configure
(venv-omnetpp) $ make
```

Start the OMNeT++ IDE:

```
(venv-omnetpp) $ cd bin 
(venv-omnetpp) $ ./omnetpp
```

For your convenience, you can also create a start script, say `omnetpp.sh`, to activate the virtual environment and
start OMNeT++ from within this environment:

```
#!/usr/bin/bash

source ~/venv-omnetpp/bin/activate
cd ~/omnetpp-6.0.1
source setenv
./bin/omnetpp
```

Select a directory for your OMNeT++ workspace. In the following, we refer to this directory as `~/workspace`.

OMNeT++ will offer to import INET into your workspace. Do not choose this option. We will import our own INET version
next as described below.

## Installing INET

Our implementations are based on INET v4.5.2.

Change to your OMNeT++ workspace directory:

```
$ cd ~/workspace
```

To checkout the official INET v4.5.2 release (without the DETERMINISTIC6G extensions) from the official INET GitHub
repository, execute:

```
$ git clone https://github.com/inet-framework/inet.git
$ cd inet
$ git checkout tags/v4.5.2 -b mybranch
```

You can change the branch name `mybranch` as you like. You could even leave it out, but it is good style not to work
with a so-called detached head if you plan to make changes.

Import the project into the workspace in the OMNeT++ IDE using the menu:

`File | Import | General | Existing projects into Workspace`

Select as root directory the directory to which you have checked out INET above: `~/workspace/inet`. Do not check the
option "Copy Project into Workspace". Click Finish. Note that OMNet++ might take a while to check the project showing some
warnings, which you can ignore.

Select the INET project from the OMNeT++ Project Explorer (tree-view on the left side in the IDE). Then build the INET
project by selecting the menu item:

Project | Build All

When the build is finished, you can test your INET installation, for instance, using the Time Aware Shaper TSN showcase
shipped with INET. From the INET project in the Project Explorer select:

showcases / tsn / trafficshaping / timeawareshaper / omnetpp.ini

Right-click the file `omnetpp.ini` and select:

Run As | OMNeT++ Simulation
