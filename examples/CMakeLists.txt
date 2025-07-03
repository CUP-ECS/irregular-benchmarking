blt_add_executable( NAME main
        SOURCES main.cpp
        OUTPUT_DIR ${EXAMPLE_OUTPUT_DIRECTORY})






#
##CALIPER_start
#find_path(CALIPER_INCLUDE_DIR caliper/cali.h
#        PATHS
#        ${SPACK_PREFIX}/include
#)
#
#find_library(CALIPER_LIBRARY caliper
#        PATHS
#        ${SPACK_PREFIX}/lib
#)
#
## Check if we found Caliper
#if (CALIPER_INCLUDE_DIR AND CALIPER_LIBRARY)
#    message(STATUS "Found Caliper: ${CALIPER_LIBRARY}")
#    set(CALIPER_FOUND TRUE)
#else ()
#    message(FATAL_ERROR "Caliper not found!")
#endif ()
#
#set(CALIPER_INCLUDE_DIRS ${CALIPER_INCLUDE_DIR})
#set(CALIPER_LIBRARIES ${CALIPER_LIBRARY})
#
#include_directories(${CALIPER_INCLUDE_DIR})
##CALIPER_end
#



#TCLAP_start
find_path(TCLAP_INCLUDE_DIR tclap/CmdLine.h
        PATHS
        ${SPACK_PREFIX}/include
)

# Check if we found TCLAP
if (TCLAP_INCLUDE_DIR)
    message(STATUS "Found TCLAP: ${TCLAP_INCLUDE_DIR}")
    set(TCLAP_FOUND TRUE)
else ()
    message(FATAL_ERROR "TCLAP not found!")
endif ()
set(TCLAP_INCLUDE_DIRS ${TCLAP_INCLUDE_DIR})


include_directories(${TCLAP_INCLUDE_DIR})
#TCLAP_end



# nlohmann_json_start
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
        PATHS
        ${SPACK_PREFIX}/include
)

# Check if we found nlohmann/json
if (NLOHMANN_JSON_INCLUDE_DIR)
    message(STATUS "Found nlohmann/json: ${NLOHMANN_JSON_INCLUDE_DIR}")
    set(NLOHMANN_JSON_FOUND TRUE)
else ()
    message(FATAL_ERROR "nlohmann/json not found!")
endif ()
set(NLOHMANN_JSON_INCLUDE_DIRS ${NLOHMANN_JSON_INCLUDE_DIR})

include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
# nlohmann_json_end


#
#find_package(MPI REQUIRED)
#target_link_libraries(main PRIVATE MPI::MPI_CXX)
find_package(vernier REQUIRED)


target_link_libraries( main vernier::vernier PRIVATE myproject )

#target_link_libraries( main PRIVATE ${CALIPER_LIBRARY} )

install(TARGETS main DESTINATION ${CMAKE_INSTALL_BINDIR})
