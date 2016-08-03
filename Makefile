#######################################################
#
#    Aland Sailing Robot
#    ===========================================
#    sailingrobot
#    -------------------------------------------
#
#######################################################


#######################################################
# TOOLCHAINS
#######################################################


# Options include:
#		linux_local = For a local linux machine (MOCK objects used)
#		raspi_cc = For cross compiling to the PI on a linux machine
#		raspi_local = For compiling on the PI itself
export TOOLCHAIN = linux-local
C_TOOLCHAIN = 0
USE_SIM = 0

#######################################################
# FILES
#######################################################

# Directories
export BUILD_DIR = build
SRC_DIR = ./
OUTPUT_DIR = ./

UNIT_TEST = ./unit-tests.run
HARDWARE_TEST = ./hardware-tests.run

# External Libraries

JSON = 					libs/json

# Sources

CORE =					MessageBus.cpp ActiveNode.cpp

ifeq ($(USE_SIM),1)
NODES =					Nodes/MessageLoggerNode.cpp  Nodes/WaypointNode.cpp Nodes/HTTPSyncNode.cpp Nodes/xBeeSyncNode.cpp \
							Nodes/VesselStateNode.cpp  Nodes/RoutingNode.cpp Nodes/LineFollowNode.cpp \
							Nodes/SimulationNode.cpp Nodes/obstacledetection/colorDetectionNode.cpp \
							Nodes/obstacledetection/colorDetectionUtility.cpp Nodes/lidarLite/lidarLite.cpp \
							Nodes/lidarLite/lidarLiteNode.cpp
SYSTEM_SERVICES =	SystemServices/Logger.cpp
else
NODES =					Nodes/MessageLoggerNode.cpp Nodes/CV7Node.cpp Nodes/HMC6343Node.cpp Nodes/GPSDNode.cpp Nodes/ActuatorNode.cpp  Nodes/ArduinoNode.cpp \
														Nodes/VesselStateNode.cpp Nodes/WaypointNode.cpp Nodes/HTTPSyncNode.cpp Nodes/xBeeSyncNode.cpp Nodes/RoutingNode.cpp Nodes/LineFollowNode.cpp \
														Nodes/SimulationNode.cpp Nodes/obstacledetection/colorDetectionNode.cpp Nodes/obstacledetection/colorDetectionUtility.cpp \
														Nodes/lidarLite/lidarLite.cpp Nodes/lidarLite/lidarLiteNode.cpp

SYSTEM_SERVICES =		SystemServices/MaestroController.cpp SystemServices/Logger.cpp
endif

XBEE = 					xBee/xBeeSync.cpp xBee/xBee.cpp

BEHAVIOURCLASS = 		behaviourclass/RoutingBehaviour.cpp  behaviourclass/WaypointBehaviour.cpp behaviourclass/LineFollowBehaviour.cpp

I2CCONTROLLER = 		i2ccontroller/I2CController.cpp

POSITION = 				utility/Position.cpp utility/MockPosition.cpp utility/RealPosition.cpp

COURSE = 				coursecalculation/CourseCalculation.cpp coursecalculation/CourseMath.cpp

DB = 					dbhandler/DBHandler.cpp dbhandler/DBLogger.cpp

COMMAND = 				ruddercommand/RudderCommand.cpp sailcommand/SailCommand.cpp

XML_LOG = 				xmlparser/pugi/pugixml.cpp xmlparser/src/xml_log.cpp

THREAD = 				thread/SystemState.cpp thread/ExternalCommand.cpp thread/ThreadRAII.cpp

WAYPOINTROUTING = 		waypointrouting/WaypointRouting.cpp waypointrouting/Commands.cpp waypointrouting/TackAngle.cpp

WINDVANECONTROLLER = 	windvanecontroller/WindVaneController.cpp


SRC_MAIN = main.cpp

SRC = 	utility/Utility.cpp utility/Timer.cpp utility/SysClock.cpp $(SYSTEM_SERVICES) $(XBEE) \
		$(CORE) $(NODES) $(I2CCONTROLLER) $(POSITION) $(COURSE) $(DB) $(COMMAND) $(GPS) \
		$(XML_LOG) $(THREAD) $(WAYPOINTROUTING) $(WINDVANECONTROLLER)


#SOURCES = $(addprefix src/, $(SRC))

# Includes

export INC = -I./ -I./libs

INC = -I./ -I./libs -I./libs/wiringPi/wiringPi

WIRING_PI = libwiringPi.so
WIRING_PI_PATH = ./libs/wiringPi/wiringPi/
WIRING_PI_STATIC = ./libs/wiringPi/wiringPi/libwiringPi.so.2.32

# Object files
OBJECTS = $(addprefix $(BUILD_DIR)/, $(SRC:.cpp=.o))
OBJECT_MAIN = $(addprefix $(BUILD_DIR)/, $(SRC_MAIN:.cpp=.o))

# Target Output
EXECUTABLE = sr
export OBJECT_FILE = $(BUILD_DIR)/objects.tmp


#######################################################
# TOOLS
#######################################################


export CFLAGS = -Wall -g -o2 `pkg-config --cflags opencv`
export CPPFLAGS = -g -Wall -pedantic -Werror -std=c++11

export LIBS = -lsqlite3 -lgps -lrt -lcurl -lpthread `pkg-config --libs opencv`

ifeq ($(TOOLCHAIN),raspi_cc)
C_TOOLCHAIN = 0
CC = arm-linux-gnueabihf-gcc
CXX = arm-linux-gnueabihf-g++
SIZE = arm-linux-gnueabihf-size
else
C_TOOLCHAIN = 1
CC = gcc
CXX = g++
SIZE = size
endif

export CC
export CXX

export MKDIR_P = mkdir -p


#######################################################
# Rules
#######################################################

.PHONY: clean

all: $(EXECUTABLE) stats


simulation:
	make USE_SIM=1 -j4

# Builds the intergration test, requires the whole system to be built before
build_tests: $(OBJECTS) $(EXECUTABLE)
	@echo Building tests...
	$(MAKE) -C tests
	$(CXX) $(CPPFLAGS) tests/runner.o @$(OBJECT_FILE) -Wl,-rpath=./ ./libwiringPi.so -o $(UNIT_TEST) $(LIBS)
	$(CXX) $(CPPFLAGS) tests/runnerHardware.o @$(OBJECT_FILE) -Wl,-rpath=./ ./libwiringPi.so -o $(HARDWARE_TEST) $(LIBS)

#  Create the directories needed
$(BUILD_DIR):
	@$(MKDIR_P) $(BUILD_DIR)

$(WIRING_PI):
	$(MAKE) -C $(WIRING_PI_PATH)
	@mv $(WIRING_PI_STATIC) ./libwiringPi.so

# Link and build
$(EXECUTABLE) : $(BUILD_DIR) $(OBJECTS) $(WIRING_PI) $(OBJECT_MAIN)
	rm -f $(OBJECT_FILE)
	@echo Linking object files
	@echo -n " " $(OBJECTS) >> $(OBJECT_FILE)
	$(CXX) $(LDFLAGS) @$(OBJECT_FILE) ./libwiringPi.so $(OBJECT_MAIN) -Wl,-rpath=./ -o $@ $(LIBS) $(LIBS_BOOST)
	@echo Built using toolchain: $(TOOLCHAIN)

# Compile CPP files into the build folder
$(BUILD_DIR)/%.o:$(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo Compiling CPP File: $@

	@$(CXX) -c $(CPPFLAGS) $(INC) -o ./$@ $< -DTOOLCHAIN=$(TOOLCHAIN) -DSIMULATION=$(USE_SIM) $(LIBS) $(LIBS_BOOST)

 # Compile C files into the build folder
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo Compiling C File: $@
	@$(C) -c $(CFLAGS) $(INC) -o $@ $ -DTOOLCHAIN=$(C_TOOLCHAIN)


#####################################################################
# Tool Rules

stats:$(EXECUTABLE)
	@echo Final executable size:
	$(SIZE) $(EXECUTABLE)

clean:
	@echo Removing existing object files and executable
	@rm -f -r $(BUILD_DIR)
	@rm -f $(EXECUTABLE)
	@echo DONE
