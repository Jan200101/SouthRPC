
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(RapidJSON REQUIRED)

plugin_manifest(SouthRPC name "SouthRPC")
plugin_manifest(SouthRPC displayname "SouthRPC")
plugin_manifest(SouthRPC description "Implements JSON-RPC 2.0 Bridge for Northstar over HTTP 1.1")
plugin_manifest(SouthRPC run_on_server ON) # Insecure but sure
plugin_manifest(SouthRPC run_on_client ON)

add_library(SouthRPC SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/init.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/handler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/handler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/helper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/helper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/http_server.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/http_server.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rpc_server.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/rpc_server.h"
)

target_include_directories(SouthRPC PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(SouthRPC rapidjson_header)
target_link_libraries(SouthRPC ws2_32)
target_precompile_headers(SouthRPC PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ns_plugin.h")

plugin_link(SouthRPC)
plugin_thunderstore(SouthRPC
    "https://github.com/Jan200101/SouthRPC"
    "${PROJECT_SOURCE_DIR}/README.md"
    "${PROJECT_SOURCE_DIR}/icon.png"
)

if (BUILD_LOCAL)
    add_subdirectory(local)
endif()
