
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(spdlog REQUIRED)

plugin_manifest(SouthRPC name "South-RPC")
plugin_manifest(SouthRPC displayname "South RPC")
plugin_manifest(SouthRPC description "Implements JSON-RPC Bridge for Northstar over TCP/IP")
plugin_manifest(SouthRPC run_on_server OFF)
plugin_manifest(SouthRPC run_on_client ON)

add_library(SouthRPC SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/init.cpp
)

target_precompile_headers(SouthRPC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/plugin.h)

plugin_link(SouthRPC)
