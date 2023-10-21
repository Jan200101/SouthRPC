## CMake extension for creating R2Northstar V2 Plugins

cmake_minimum_required( VERSION 3.21.1)

if(CMAKE_POLICY_DEFAULT_CMP0017 OR CMAKE_POLICY_DEFAULT_CMP0020)
    # touch these to remove warnings
endif()
cmake_policy(SET CMP0057 NEW)

project(R2plugin)

if (NOT WIN32)
    message(FATAL_ERROR "Northstar Plugins can only be compiled for Windows")
elseif ("${CMAKE_SYSTEM_PROCESSOR}" AND NOT "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
    message(FATAL_ERROR "Northstar Plugins can only be build for x86_64")
elseif (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    message(WARNING "Titanfall and Northstar are built using MSVC, other compilers may not produce a working plugin.")
endif ()

if (__R2PLUGIN_CMAKE_INCLUDED)
    return()
endif()
set(__R2PLUGIN_CMAKE_INCLUDED TRUE)

find_package(spdlog REQUIRED)
find_package(NorthstarPluginABI REQUIRED)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

macro(check_compiler_flags LANG FLAGS OUTPUT)
    foreach(flag ${${FLAGS}})
        string(REPLACE "+" "P" VAR_NAME ${flag})
        string(TOUPPER ${VAR_NAME} VAR_NAME)
        set(flag "-${flag}")
        check_compiler_flag("${LANG}" "${flag}" "${VAR_NAME}")

        if(${${VAR_NAME}})
            list(APPEND ${OUTPUT} "${flag}")
        endif()
    endforeach()
endmacro()

macro(check_linker_flags LANG FLAGS OUTPUT)
    foreach(flag ${${FLAGS}})
        string(REPLACE "+" "P" VAR_NAME ${flag})
        string(TOUPPER ${VAR_NAME} VAR_NAME)
        set(flag "-${flag}")
        check_linker_flag("${LANG}" "${flag}" "${VAR_NAME}")

        if(${${VAR_NAME}})
            list(APPEND ${OUTPUT} "${flag}")
        endif()
    endforeach()
endmacro()


list(APPEND
    C_FLAGS
)

list(APPEND
    C_LINK_FLAGS
    static
    static-libgcc
)

list(APPEND
    CXX_FLAGS
    ${C_FLAGS}
)

list(APPEND
    CXX_LINK_FLAGS
    ${C_LINK_FLAGS}
    static-libstdc++
)


get_property(languages GLOBAL PROPERTY ENABLED_LANGUAGES)

if("C" IN_LIST languages)
    check_compiler_flags("C" C_FLAGS C_FLAGS_LIST)
    list(JOIN C_FLAGS_LIST " " C_FLAGS)
    set(PLUGIN_C_FLAGS "${PLUGIN_C_FLAGS} ${C_FLAGS}")

    check_linker_flags("C" C_LINK_FLAGS C_LINK_FLAGS_LIST)
    list(JOIN C_LINK_FLAGS_LIST " " PLUGIN_C_LINK_FLAGS)
endif()
if("CXX" IN_LIST languages)
    check_compiler_flags("CXX" CXX_FLAGS CXX_FLAGS_LIST)
    list(JOIN CXX_FLAGS_LIST " " CXX_FLAGS)
    set(PLUGIN_CXX_FLAGS "${PLUGIN_CXX_FLAGS} ${CXX_FLAGS}")

    check_linker_flags("CXX" CXX_LINK_FLAGS CXX_LINK_FLAGS_LIST)
    list(JOIN CXX_LINK_FLAGS_LIST " " PLUGIN_CXX_LINK_FLAGS)
endif()

set(RESOURCE_TEMPLATE
"
#define IDR_RCDATA1 101
IDR_RCDATA1             RCDATA                  \"manifest.json\"

VS_VERSION_INFO VERSIONINFO
 FILEVERSION 1,0,0,0
 PRODUCTVERSION 1,0,0,0
 FILEFLAGSMASK 0x3fL
 FILEFLAGS 0x0L
 FILEOS 0x40004L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK \"StringFileInfo\"
    BEGIN
        BLOCK \"040904b0\"
        BEGIN
            VALUE \"CompanyName\", \"Northstar\"
            VALUE \"FileDescription\", \"Northstar Plugin\"
            VALUE \"FileVersion\", \"1.0.0.0\"
            VALUE \"ProductName\", \"Northstar Plugin\"
            VALUE \"ProductVersion\", \"1.0.0.0\"
        END
    END
    BLOCK \"VarFileInfo\"
    BEGIN
        VALUE \"Translation\", 0x409, 1200
    END
END
")

set(MANIFEST_TEMPLATE "
    {
        \"name\": \"NSPlugin\",
        \"displayname\": \"Northstar Plugin\",
        \"description\": \"\",
        \"api_version\": \"3\",
        \"version\": \"0\",
        \"run_on_server\": false,
        \"run_on_client\": false
    }\n"
)

macro(plugin_manifest TARGET KEY VALUE)
    if (NOT ${TARGET}_MANIFEST)
        set(${TARGET}_MANIFEST "${MANIFEST_TEMPLATE}")

        # set default name to target name
        string(JSON ${TARGET}_MANIFEST SET "${${TARGET}_MANIFEST}" name "\"${TARGET}\"")

        # set default version to project version
        string(JSON ${TARGET}_MANIFEST SET "${${TARGET}_MANIFEST}" version "\"${CMAKE_PROJECT_VERSION}\"")
    endif()

    if ("${VALUE}" STREQUAL ON)
        set(JSON_VALUE "true")
    elseif ("${VALUE}" STREQUAL OFF)
        set(JSON_VALUE "false")
    else()
        set(JSON_VALUE "\"${VALUE}\"")
    endif()

    string(JSON ${TARGET}_MANIFEST SET "${${TARGET}_MANIFEST}" "${KEY}" "${JSON_VALUE}")
endmacro()

macro(plugin_link TARGET)
    if (NOT ${TARGET}_MANIFEST)
        message(FATAL_ERROR "No Plugin manifest found")
    endif()

    if("C" IN_LIST languages)
        target_link_libraries(${TARGET} ${PLUGIN_C_LINK_FLAGS})
    endif()
    if("CXX" IN_LIST languages)
        target_link_libraries(${TARGET} ${PLUGIN_CXX_LINK_FLAGS})
    endif()
    target_link_libraries(${TARGET} spdlog::spdlog_header_only)
    target_include_directories(${TARGET} PRIVATE ${NS_DLL_DIR})
    target_include_directories(${TARGET} PRIVATE ${NS_PLUG_DIR})

    string(JSON PLUGIN_NAME GET "${${TARGET}_MANIFEST}" name)
    target_compile_definitions(${TARGET} PRIVATE PLUGIN_NAME=\"${PLUGIN_NAME}\")

    set(MANIFEST_DIR "${CMAKE_BINARY_DIR}/${TARGET}_plugin/")
    file(MAKE_DIRECTORY "${MANIFEST_DIR}")
    file(WRITE "${MANIFEST_DIR}/manifest.json" "${${TARGET}_MANIFEST}")
    file(WRITE "${MANIFEST_DIR}/manifest.rc" "${RESOURCE_TEMPLATE}")
    target_sources(${TARGET} PUBLIC "${MANIFEST_DIR}/manifest.rc")
endmacro()

macro(plugin_thunderstore TARGET WEBSITE_URL README ICON)
    if (NOT ${TARGET}_MANIFEST)
        message(FATAL_ERROR "No Plugin manifest found")
    endif()

    string(JSON PLUGIN_NAME GET "${${TARGET}_MANIFEST}" name)
    string(JSON PLUGIN_DESCRIPTION GET "${${TARGET}_MANIFEST}" description)
    string(JSON PLUGIN_VERSION GET "${${TARGET}_MANIFEST}" version)

    set(THUNDERSTORE_TEMPLATE "
        {
            \"name\": \"kyurid\",
            \"version_number\": \"\",
            \"website_url\": \"${WEBSITE_URL}\",
            \"description\": \"\",
            \"dependencies\": []
        }\n"
    )
    # do this to reformat the template
    string(JSON THUNDERSTORE_TEMPLATE SET "${THUNDERSTORE_TEMPLATE}" name "\"${PLUGIN_NAME}\"")
    string(JSON THUNDERSTORE_TEMPLATE SET "${THUNDERSTORE_TEMPLATE}" version_number "\"${PLUGIN_VERSION}\"")
    string(JSON THUNDERSTORE_TEMPLATE SET "${THUNDERSTORE_TEMPLATE}" description "\"${PLUGIN_DESCRIPTION}\"")

    set(MOD_DIR "${CMAKE_BINARY_DIR}/${TARGET}_mod/")
    set(MOD_PLUG_DIR "${MOD_DIR}/plugins")

    file(MAKE_DIRECTORY "${MOD_DIR}")
    file(MAKE_DIRECTORY "${MOD_PLUG_DIR}")
    file(WRITE "${MOD_DIR}/manifest.json" "${THUNDERSTORE_TEMPLATE}")
    file(COPY_FILE ${README} "${MOD_DIR}/README.md")
    file(COPY_FILE ${ICON} "${MOD_DIR}/icon.png")

    install(TARGETS ${TARGET} RUNTIME DESTINATION "${MOD_PLUG_DIR}")
endmacro()