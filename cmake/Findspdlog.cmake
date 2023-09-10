### Get same spdlog as Northstar

if (spdlog_FOUND)
	return()
endif()

find_package(NorthstarPluginABI REQUIRED)
find_package(Threads REQUIRED)

check_init_submodule(${NS_LAUNCHER_DIR}/thirdparty/spdlog)

add_library(spdlog_header_only INTERFACE)
add_library(spdlog::spdlog_header_only ALIAS spdlog_header_only)
target_include_directories(spdlog_header_only INTERFACE "${NS_LAUNCHER_DIR}/thirdparty")
target_include_directories(spdlog_header_only INTERFACE "${NS_LAUNCHER_DIR}/thirdparty/spdlog")
target_link_libraries(spdlog_header_only INTERFACE Threads::Threads)

set(spdlog_FOUND 1)

