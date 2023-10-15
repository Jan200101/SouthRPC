
find_package(spdlog REQUIRED)
find_package(NorthstarPluginABI REQUIRED)

add_executable(LocalRPC
	$<TARGET_OBJECTS:SouthRPC>
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_include_directories(LocalRPC PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(LocalRPC rapidjson_header)
target_link_libraries(LocalRPC ws2_32)
target_precompile_headers(LocalRPC PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ns_plugin.h")

target_link_libraries(LocalRPC spdlog::spdlog_header_only)
target_include_directories(LocalRPC PRIVATE ${NS_DLL_DIR})
target_include_directories(LocalRPC PRIVATE ${NS_PLUG_DIR})