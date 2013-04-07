IF (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        SET(PCHSupport_FOUND TRUE)
ENDIF()

MACRO(ADD_PCH _pch _header)
    # Get source and destination file
    GET_FILENAME_COMPONENT(_name ${_header} NAME_WE)
    SET(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_header}")
    SET(_output "${CMAKE_CURRENT_BINARY_DIR}/${_name}.pch")

    # Get compiler flags
    STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _cxxflags_var)
    SET(_compiler_flags "${CMAKE_CXX_FLAGS} ${${_cxxflags_var}}")
    
    # Get include directories
    GET_DIRECTORY_PROPERTY(_include_directories INCLUDE_DIRECTORIES)
    FOREACH(_directory ${_include_directories})
        LIST(APPEND _compiler_flags "-I${_directory}")
    ENDFOREACH(_directory)

    # Get definitions
    GET_DIRECTORY_PROPERTY(_definitions DEFINITIONS)
    LIST(APPEND _compiler_flags ${_definitions})

    # Generate appropriate command
    SEPARATE_ARGUMENTS(_compiler_flags)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${_output}
        COMMAND ${CMAKE_CXX_COMPILER}
                                ${_compiler_flags}
                                -Xclang -emit-pch
                                -o ${_output} ${_source}
        DEPENDS ${_source} )
    ADD_CUSTOM_TARGET(${_pch} DEPENDS ${_output})
    SET("${_pch}_output" ${_output})
ENDMACRO(ADD_PCH)

MACRO(TARGET_USE_PCH _target _pch)
    IF (${ARGC} GREATER 2})
        MESSAGE(FATAL_ERROR "Clang only supports a single precompiled header")
    ENDIF (${ARGC} GREATER 2})

    IF (DEFINED ${_pch}_output)
        ADD_DEPENDENCIES(${_target} ${_pch})
        SET_TARGET_PROPERTIES(${_target} PROPERTIES
            COMPILE_FLAGS "-Xclang -include-pch -Xclang ${${_pch}_output} -Winvalid-pch"
        )
    ENDIF (DEFINED ${_pch}_output)
ENDMACRO(TARGET_USE_PCH)
