
if (NorthstarPluginABI_FOUND)
	return()
endif()

set(NS_LAUNCHER_DIR "${PROJECT_SOURCE_DIR}/deps/NorthstarLauncher" CACHE STRING "Path to NorthstarLauncher Dependency")
check_init_submodule(${NS_LAUNCHER_DIR})

set(NS_DLL_DIR "${NS_LAUNCHER_DIR}/NorthstarDLL")
set(NS_PLUG_DIR "${NS_DLL_DIR}/plugins")

set(NorthstarPluginABI_FOUND 1)
