
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(json-c REQUIRED)

plugin_manifest(SouthRPC name "SouthRPC")
plugin_manifest(SouthRPC displayname "SouthRPC")
plugin_manifest(SouthRPC description "Implements JSON-RPC Bridge for Northstar over TCP/IP")
plugin_manifest(SouthRPC run_on_server OFF)
plugin_manifest(SouthRPC run_on_client ON)

add_library(SouthRPC SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/init.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/plugin.h
	${CMAKE_CURRENT_SOURCE_DIR}/server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/server.h
)

target_link_libraries(SouthRPC json-c::json-c)
target_precompile_headers(SouthRPC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ns_plugin.h)

plugin_link(SouthRPC)
