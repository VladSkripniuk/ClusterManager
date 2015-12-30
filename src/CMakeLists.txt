cmake_minimum_required(VERSION 2.8)

set(SERVER_EXE Server.cpp)
set(CLIENT_EXE Client.cpp)

set(CMAKE_CXX_FLAGS "-std=c++11")

set(tinyxml2_LIB tinyxml2.cpp)
set(ClusterManager_LIB ClusterManager.cpp)
set(ConnectionManager_LIB ConnectionManager.cpp)
set(ClientManager_LIB ClientManager.cpp)
set(Connection_LIB Connection.cpp)
set(ClusterShell_LIB ClusterShell.cpp)
set(Listener_LIB Listener.cpp)
set(ShellParser_LIB ShellParser.cpp)

add_library(ClusterManager STATIC ${ClusterManager_LIB})
add_library(tinyxml2 STATIC ${tinyxml2_LIB})
add_library(ConnectionManager STATIC ${ConnectionManager_LIB})
add_library(ClientManager STATIC ${ClientManager_LIB})
add_library(ClusterShell STATIC ${ClusterShell_LIB})
add_library(Connection STATIC ${Connection_LIB})
add_library(Listener STATIC ${Listener_LIB})
add_library(ShellParser STATIC ${ShellParser_LIB})

target_link_libraries(Connection tinyxml2)

add_executable(server ${SERVER_EXE})
add_executable(client ${CLIENT_EXE})

target_link_libraries(server ClusterManager tinyxml2 ConnectionManager Connection ClusterShell Listener pthread ShellParser)
target_link_libraries(client ClientManager tinyxml2 Connection pthread)
