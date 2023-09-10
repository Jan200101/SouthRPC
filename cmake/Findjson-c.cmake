#
# Tries to find json-c through the config
# before trying to query for it
#
#

if (json-c_FOUND)
  return()
endif()

find_package(json-c CONFIG)

if (JSONC_FOUND)
    return()
endif()

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(_JSONC json-c)

    if (BUILD_STATIC AND NOT _JSONC_FOUND)
         message(FATAL_ERROR "Cannot find static build information")
    endif()
    set(json-c_FOUND 1)
endif()

if (json-c_FOUND) # we can rely on pkg-config
    set(json-c_LINK_LIBRARIES ${_JSONC_LINK_LIBRARIES})
    if (NOT BUILD_STATIC)
        set(json-c_INCLUDE_DIRS ${_JSONC_INCLUDE_DIRS})
        set(json-c_CFLAGS ${_JSONC_CFLAGS_OTHER})
    else()
        set(json-c_INCLUDE_DIRS ${_JSONC_STATIC_INCLUDE_DIRS})
        set(json-c_CFLAGS ${_JSONC_STATIC_CFLAGS_OTHER})
    endif()
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
        PATHS
            /usr/include/json-c /usr/local/include/json-c)

    find_library(JSONC_LIB
        NAMES ${_JSONC_LIBRARIES} jsonc json-c
        HINTS
            ENV jsoncPath${_lib_suffix}
            ENV jsoncPath
            ${_JSONC_LIBRARY_DIRS}
            ${_JSONC_STATIC_LIBRARY_DIRS}
        PATHS
            /usr/lib{_lib_suffix} /usr/local/lib{_lib_suffix}
            /usr/lib /usr/local/lib)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(json-c DEFAULT_MSG JSONC_LIB JSONC_INC)
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
    add_library(json-c::json-c UNKNOWN IMPORTED)
    set_target_properties(json-c::json-c PROPERTIES
        IMPORTED_LOCATION "${json-c_LINK_LIBRARIES}"
    )
    target_compile_definitions(json-c::json-c INTERFACE ${json-c_CFLAGS})
    target_include_directories(json-c::json-c INTERFACE ${json-c_INCLUDE_DIRS})
endif()
