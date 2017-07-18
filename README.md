Sailingrobot
==========

Åland Sailing Robot is a research project at Åland University of Applied Sciences that develops autonomous sailing boats.
This repository contains the embedded software  executed on the different autonomous boats of the project and various useful tools for its development (simulator, unit-tests, UI...).

The following Autonomous sailboats are equipped with this software :
- Janet
- ASPire

## Overview

### Directory Structure

The  *sailingrobot* repository is divided into different directories :
- **ArduinoSketches** contains  the code executed on the arduino boards of the autonomous sailboats (Janet, ASPire,...).
- **CBoat** is tool that displays the current boat state.
-  **Mission** regroups predefined missions.
- **Navigation System** contains the source code of the control system running on the Raspberry Pi of the autonomous sailboats.
-  **sailing_simulator** contains the simulation environment used to test and develop the control system. (Submodule)
- **Xbee remote** contains the code used to establish a radio communication between the embedded system and a laptop by using XBee modules.

##  Setup guide

### Requirements

Additional packages are required for building the Navigation System.
```shell
$ sudo apt-get install libncurses5-dev libncursesw5-dev
```

### Installation

If you clone the repository, either use:
```shell
$ git clone --recursive https://github.com/AlandSailingRobots/sailingrobot.git
```
or if you have already cloned it, use:
```shell
$ cd sailingrobot  
$ git submodule init # Initialises all the submodules (Only need to be done once)  
$ git submodule update # Updates the submodule to the latest version 
```
Install the wiringPi library:
```shell 
$ cd libs/wiringPi
$ ./build #Install wiringPi as a dynamic library
```
Install the database:
```shell 
$ ./installdb.sh
```

### Build Instructions

To build the Control system in *NavigationSystem*, type:
```shell 
make [target] [ext variable 1] [ext variable 2]

Targets:
	all: Default, same as make ASPire
	ASPire: Build the control system for ASPire
	Janet: Build the control system for Janet
	unit_tests: Build the unit tests
	integration_tests_ASPire: Build the integration test for ASPire

External Variables (Only for building a control system):
	USE_SIM:    1 To run with the sailing_simulator
				0 to run on the Pi (default)
	USE_LNM: Chooses which navigation algorithm you want to use. 
				1 for the Local Navigation Module (Voter System)
				0 for the line-follow algorithm (default)
``` 

Example :
```
# Build the ASPire control system with the line-follow algorithm, to be executed with the simulator.
make ASPire USE_SIM=1
```

### Usage

The database has two functionalities:
- Logging navigation datas
- Storing the sailboat parameters and mission

#### Configuration

To update the configuration tables in the database, you run the update_config.py script with one of the configuration json files as a parameter.

```shell
./update_config.py config_ASPire.json
```

To change the waypoints you use the waypoints.py together with one of the json in the Mission Folder.

```shell
./update_waypoints.py Mission/ASS.json
```

#### Deployment

To run the control system previously build, use (no parameters needed):
```shell
./NavigationSystem/sr
```

#### Run Tests

## License
GNU GPL v2
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

## Links

* [Web site](www.sailingrobots.ax)