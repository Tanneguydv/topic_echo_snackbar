cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(TopicEchoSnackBar)

cmake_policy(SET CMP0100 NEW)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(ignition-cmake2 REQUIRED)
find_package(ignition-gazebo6 REQUIRED COMPONENTS gui)
find_package(ignition-msgs8 REQUIRED)
find_package(ignition-transport11 REQUIRED)
find_package(ignition-gui6 REQUIRED)

# Find ignition-plugin
ign_find_package(ignition-plugin1 REQUIRED COMPONENTS register)
set(IGN_PLUGIN_VER ${ignition-plugin1_VERSION_MAJOR})

# Add sources
add_library(TopicEchoSnackBar SHARED
  TopicEchoSnackBar.cc
  TopicEchoSnackBar.hh
  TopicEchoSnackBar.qrc
)

# Include directories
target_include_directories(TopicEchoSnackBar PRIVATE
  ${IGNITION-TRANSPORT_INCLUDE_DIRS}
  ${IGNITION-MSGS_INCLUDE_DIRS}
  ${IGNITION-GUI_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(TopicEchoSnackBar
  ignition-common4::ignition-common4
  ignition-plugin${IGN_PLUGIN_VER}::ignition-plugin${IGN_PLUGIN_VER}
  ignition-msgs8::ignition-msgs8
  ignition-transport11::ignition-transport11
  ignition-gui6::ignition-gui6
)

### INSTALLATION ###
install(TARGETS TopicEchoSnackBar
  LIBRARY DESTINATION lib/x86_64-linux-gnu/ign-gui-6/custom_plugins
)

install(FILES TopicEchoSnackBar.qml
  DESTINATION share/${PROJECT_NAME}
)

install(FILES TopicEchoSnackBar.qrc
  DESTINATION share/${PROJECT_NAME}
)


