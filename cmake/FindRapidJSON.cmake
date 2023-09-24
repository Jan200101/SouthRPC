### Get same spdlog as Northstar

if (RapidJSON_FOUND)
	return()
endif()

find_package(NorthstarPluginABI REQUIRED)

check_init_submodule(${NS_LAUNCHER_DIR}/thirdparty/rapidjson)

add_library(rapidjson_header INTERFACE)
target_include_directories(rapidjson_header INTERFACE "${NS_LAUNCHER_DIR}/thirdparty")
target_include_directories(rapidjson_header INTERFACE "${NS_LAUNCHER_DIR}/thirdparty/rapidjson")

set(RapidJSON_FOUND 1)

