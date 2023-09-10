#
# Tries to find json-c through the config
# before trying to query for it
#

if (json-c_FOUND)
  return()
endif()

#find_package(json-c CONFIG)

if (json-c_FOUND)
    return()
endif()

if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(_JSONC json-c)
    endif()
endif()

if (_JSONC_FOUND) # we can rely on pkg-config
    set(json-c_LINK_LIBRARIES ${_JSONC_LINK_LIBRARIES})
    if (NOT BUILD_STATIC)
        set(json-c_INCLUDE_DIRS ${_JSONC_INCLUDE_DIRS})
        set(json-c_CFLAGS ${_JSONC_CFLAGS_OTHER})
    else()
        set(json-c_INCLUDE_DIRS ${_JSONC_STATIC_INCLUDE_DIRS})
        set(json-c_CFLAGS ${_JSONC_STATIC_CFLAGS_OTHER})
    endif()
    set(json-c_FOUND 1)
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_lib_suffix 64)
    else()
        set(_lib_suffix 32)
    endif()

    find_path(JSONC_INC
        NAMES json.h
        HINTS
            ENV jsoncPath${_lib_suffix}
            ENV jsoncPath
            ${_JSONC_INCLUDE_DIRS}
    )

    find_library(JSONC_LIB
        NAMES ${_JSONC_LIBRARIES} jsonc json-c
        HINTS
            ENV jsoncPath${_lib_suffix}
            ENV jsoncPath
            ${_JSONC_LIBRARY_DIRS}
            ${_JSONC_STATIC_LIBRARY_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    #find_package_handle_standard_args(json-c DEFAULT_MSG JSONC_LIB JSONC_INC)
    mark_as_advanced(JSONC_INC JSONC_LIB)

    if(json-c_FOUND)
        set(json-c_INCLUDE_DIRS ${JSONC_INC})
        set(json-c_LINK_LIBRARIES ${JSONC_LIB})
        if (BUILD_STATIC)
            set(json-c_LINK_LIBRARIES ${json-c_LINK_LIBRARIES} ${_JSONC_STATIC_LIBRARIES})
        endif()
    endif()
endif()


if (json-c_FOUND)
    # Reconstruct the official interface
    add_library(json-c::json-c UNKNOWN IMPORTED)
    set_target_properties(json-c::json-c PROPERTIES
        IMPORTED_LOCATION "${json-c_LINK_LIBRARIES}"
    )
    target_compile_definitions(json-c::json-c INTERFACE ${json-c_CFLAGS})
    target_include_directories(json-c::json-c INTERFACE ${json-c_INCLUDE_DIRS})
else()
    include(FetchContent)
    cmake_policy(SET CMP0077 NEW)

    message(STATUS "Downloading json-c...")
    FetchContent_Declare(
        jsonc
        GIT_REPOSITORY https://github.com/json-c/json-c
        GIT_TAG json-c-0.17
        GIT_SHALLOW TRUE
    )

    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
    FetchContent_MakeAvailable(jsonc)

    # Only the config file includes the namespace
    add_library(json-c::json-c ALIAS json-c)

    set(json-c_FOUND 1)
endif()
