cmake_minimum_required(VERSION 3.1.3)

project (CLAMR CXX C)
# The version number.
set (CLAMR_VERSION_MAJOR 2)
set (CLAMR_VERSION_MINOR 0.7)
set (CLAMR_VERSION "${CLAMR_VERSION_MAJOR}.${CLAMR_VERSION_MINOR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
# --- custom targets: ---
INCLUDE( cmake/Modules/TargetDistclean.cmake OPTIONAL)

# enable @rpath in the install name for any shared library being built
set(CMAKE_MACOSX_RPATH 1)

if (NOT CMAKE_BUILD_TYPE)
   #set(CMAKE_BUILD_TYPE RelWithDebInfo)
   set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

if (CMAKE_PRECISION_CHECK)
   add_definitions(-DPRECISION_CHECK_GRAPHICS=1)
   add_definitions(-DPRECISION_CHECK_STATS=1)
   add_definitions(-DPRECISION_CHECK=1.0e-7)
endif (CMAKE_PRECISION_CHECK)

if (CMAKE_PRECISION_CHECK_APPLIED)
   if (CMAKE_PRECISION_CHECK_APPLIED STREQUAL "best")
      add_definitions(-DPRECISION_CHECK_BEST_PARENTHESIS=1)
   else (CMAKE_PRECISION_CHECK_APPLIED STREQUAL "best")
      add_definitions(-DPRECISION_CHECK_WITH_PARENTHESIS=1)
   endif(CMAKE_PRECISION_CHECK_APPLIED STREQUAL "best")
endif (CMAKE_PRECISION_CHECK_APPLIED)

enable_testing()
# uncomment to build debug
#set(CMAKE_BUILD_TYPE Debug)

#set (CMAKE_CXX_EXTENSIONS off)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_C_STANDARD 99)

#message("Release CFLAGS are ${CMAKE_C_FLAGS_RELEASE}")
#message("RelWithDebInfo CFLAGS are ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
#message("Debug CFLAGS are ${CMAKE_C_FLAGS_DEBUG}")
#message("CFLAGS are ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
string(REPLACE -O2 -O3 CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
string(REPLACE -O2 -O3 CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
#message("Release CFLAGS are ${CMAKE_C_FLAGS_RELEASE}")
#message("RelWithDebInfo CFLAGS are ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
#message("Debug CFLAGS are ${CMAKE_C_FLAGS_DEBUG}")
#message("CFLAGS are ${CMAKE_C_FLAGS}")

if (CMAKE_BUILD_TYPE MATCHES "DEBUG")
    set( CMAKE_VECTOR_NOVEC TRUE)
endif (CMAKE_BUILD_TYPE MATCHES "DEBUG")


if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
   # using Clang
   set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
   # using GCC
   set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
   # using Intel C
   set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
   # using Visual Studio C++
elseif (CMAKE_C_COMPILER_ID MATCHES "PGI")
elseif (CMAKE_C_COMPILER_ID MATCHES "Cray")
elseif (CMAKE_C_COMPILER_ID MATCHES "XL")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   # using Clang
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   # using GCC
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
   # using Intel C++
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
   # using Visual Studio C++
elseif (CMAKE_CXX_COMPILER_ID MATCHES "PGI")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Cray")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "XL")
endif()

option (KOKKOS_ENABLE "Enable KOKKOS support" OFF)
mark_as_advanced (KOKKOS_ENABLE)

#set (Kokkos_ENABLE_CUDA_LAMBDA on)

if(NOT KOKKOS_ENABLE)
   find_package(Vector)
   if (CMAKE_VECTOR_NOVEC)
       set(VECTOR_C_FLAGS "${VECTOR_NOVEC_C_FLAGS}")
       set(VECTOR_CXX_FLAGS "${VECTOR_NOVEC_CXX_FLAGS}")
       set(VECTOR_Fortran_FLAGS "${VECTOR_NOVEC_Fortran_FLAGS}")
   endif (CMAKE_VECTOR_NOVEC)
   if (CMAKE_VECTOR_VERBOSE)
       set(VECTOR_C_FLAGS "${VECTOR_C_FLAGS} ${VECTOR_C_VERBOSE}")
       set(VECTOR_CXX_FLAGS "${VECTOR_CXX_FLAGS} ${VECTOR_CXX_VERBOSE}")
       set(VECTOR_Fortran_FLAGS "${VECTOR_Fortran_FLAGS} ${VECTOR_Fortran_VERBOSE}")
   endif (CMAKE_VECTOR_VERBOSE)
endif(NOT KOKKOS_ENABLE)

set_source_files_properties(state.cpp PROPERTIES COMPILE_FLAGS "${VECTOR_CXX_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR})

if (NOT APPLE OR "${CMAKE_SYSTEM_VERSION}" LESS 18)
   find_package(OpenCL)
   #  OpenCL_FOUND          - True if OpenCL was found
   #  OpenCL_INCLUDE_DIRS   - include directories for OpenCL
   #  OpenCL_LIBRARIES      - link against this library to use OpenCL
   #  OpenCL_VERSION_STRING - Highest supported OpenCL version (eg. 1.2)
   #  OpenCL_VERSION_MAJOR  - The major version of the OpenCL implementation
   #  OpenCL_VERSION_MINOR  - The minor version of the OpenCL implementation
   if (OpenCL_FOUND)
      set(HAVE_CL_DOUBLE ON CACHE BOOL "Have OpenCL Double")
      set(NO_CL_DOUBLE OFF)
      include_directories(${OpenCL_INCLUDE_DIRS})
   endif (OpenCL_FOUND)
endif (NOT APPLE OR "${CMAKE_SYSTEM_VERSION}" LESS 18)

find_package(MPI)
if (MPI_FOUND)
   include_directories(${MPI_INCLUDE_PATH})
endif (MPI_FOUND)

if(NOT KOKKOS_ENABLE)
   find_package( OpenMP)
   if(OPENMP_FOUND)
      message("OPENMP FOUND ${OpenMP_C_FLAGS}")
   else (OPENMP_FOUND)
      message("OPENMP NOT FOUND")
   endif(OPENMP_FOUND)
endif(NOT KOKKOS_ENABLE)

#-----------------------------------------------------------------------------
# Option to use Shared/Static libs, default is static
#-----------------------------------------------------------------------------
set (LIB_TYPE STATIC)
if (BUILD_SHARED_LIBS)
  set (LIB_TYPE SHARED)
endif (BUILD_SHARED_LIBS)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT INTEGER_TYPE)
    set (INTEGER_TYPE reg_integer)
endif (NOT INTEGER_TYPE)

if (INTEGER_TYPE STREQUAL "reg_integer")
    set (REG_INTEGER on)
    set (INTEGER_TYPE reg_integer)
    add_definitions(-DREG_INTEGER)
endif (INTEGER_TYPE STREQUAL "reg_integer")

if (INTEGER_TYPE STREQUAL "short_integer")
    set (SHORT_INTEGER on)
    set (INTEGER_TYPE short_integer)
    add_definitions(-DSHORT_INTEGER)
endif (INTEGER_TYPE STREQUAL "short_integer")

if (INTEGER_TYPE STREQUAL "min_integer")
    set (MIN_INTEGER on)
    set (INTEGER_TYPE min_integer)
    add_definitions(-DMIN_INTEGER)
endif (INTEGER_TYPE STREQUAL "min_integer")

if (NOT PRECISION_TYPE)
   set (PRECISION_TYPE full_precision)
endif (NOT PRECISION_TYPE)

if (PRECISION_TYPE STREQUAL "full_precision")
   set (FULL_PRECISION on)
   set (PRECISION_TYPE full_precision)
   add_definitions(-DFULL_PRECISION)
endif (PRECISION_TYPE STREQUAL "full_precision")

if (PRECISION_TYPE STREQUAL "mixed_precision")
   set (MIXED_PRECISION on)
   set (PRECISION_TYPE mixed_precision)
   add_definitions(-DMIXED_PRECISION)
endif (PRECISION_TYPE STREQUAL "mixed_precision")

if (PRECISION_TYPE STREQUAL "minimum_precision")
   set (MINIMUM_PRECISION on)
   set (PRECISION_TYPE minimum_precision)
   add_definitions(-DMINIMUM_PRECISION)
endif (PRECISION_TYPE STREQUAL "minimum_precision")

if (PRECISION_TYPE STREQUAL "half_precision")
   set (HALF_PRECISION on)
   set (PRECISION_TYPE half_precision)
   add_definitions(-DHALF_PRECISION)
   include_directories(${CMAKE_SOURCE_DIR}/half/include)
endif (PRECISION_TYPE STREQUAL "half_precision")

if (NOT GRAPHICS_TYPE)
   set (GRAPHICS_TYPE none)
endif (NOT GRAPHICS_TYPE)

if (GRAPHICS_TYPE STREQUAL "OpenGL")
   find_package(OpenGL)
   if (OPENGL_FOUND)
      find_package(GLUT)
   endif (OPENGL_FOUND)
endif (GRAPHICS_TYPE STREQUAL "OpenGL")

# If autodetect does not work try setting MPEHOME or
# MPE_INCLUDE_DIR and MPE_LIBRARIES
if (GRAPHICS_TYPE STREQUAL "MPE")
   find_package(MPE)
   find_package(X11)
endif (GRAPHICS_TYPE STREQUAL "MPE")

message("MPI_LIBRARIES ${MPI_LIBRARIES}")
message("MPI_INCLUDE_PATH ${MPI_INCLUDE_PATH}")
#message("GRAPHICS_TYPE is ${GRAPHICS_TYPE}")
#message("MPE_INCLUDE is ${MPE_INCLUDE_DIR}")

if (OPENGL_FOUND AND GLUT_FOUND)
   set (HAVE_GRAPHICS on)
   set (HAVE_OPENGL on)
   include_directories(${OPENGL_INCLUDE_DIR})
   include_directories(${GLUT_INCLUDE_DIR})
endif (OPENGL_FOUND AND GLUT_FOUND)

#if(DEFINED ENV{QUO_HOME})
#   if(EXISTS "$ENV{QUO_HOME}/include/")
#      message(STATUS "QUO Detected")
#      set (QUO_FOUND on)
#      include_directories($ENV{QUO_HOME}/include)
#   endif (EXISTS "$ENV{QUO_HOME}/include/")
#endif (DEFINED ENV{QUO_HOME})

#if(DEFINED ENV{REPROBLAS_HOME})
#   if(EXISTS "$ENV{REPROBLAS_HOME}/include/rblas.h")
#      message(STATUS "reproBlas Detected")
#      set (REPROBLAS_FOUND on)
#      set (HAVE_REPROBLAS ON)
#      include_directories($ENV{REPROBLAS_HOME}/include)
#      set (REPROBLAS_LIBRARIES -L$ENV{REPROBLAS_HOME}/libs -lreproblas_mpi -lreproblas_seq -liblas -lmpindexedfp -lindexedfp)
#   endif (EXISTS "$ENV{REPROBLAS_HOME}/include/rblas.h")
#endif (DEFINED ENV{REPROBLAS_HOME})

#if(DEFINED ENV{LTTRACE_HOME})
#   if(EXISTS "$ENV{LTTRACE_HOME}/lib/liblttrace.a")
#      message(STATUS "LTTRACE Detected")
#      include_directories($ENV{LTTRACE_HOME}/include)
#      set (LTTRACE_FOUND on)
#      set (HAVE_LTTRACE ON)
#      set (LTTRACE_LIBRARIES -L$ENV{LTTRACE_HOME}/lib -llttrace -lz -luuid -Wl,-rpath -Wl,$ENV{LTTRACE_HOME}/lib)
#   endif (EXISTS "$ENV{LTTRACE_HOME}/lib/liblttrace.a")
#endif (DEFINED ENV{LTTRACE_HOME})

#if (DEFINED ENV{LD_LIBRARY_PATH})
#   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,$ENV{LD_LIBRARY_PATH}")
#   message("CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}")
#   #string (REPLACE ":" ";" LDPATH_TMP $ENV{LD_LIBRARY_PATH})
#   #foreach(LDPATH ${LDPATH_TMP})
#   #   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,${LDPATH}")
#   #   message("CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}")
#   #endforeach(LDPATH)
#endif (DEFINED ENV{LD_LIBRARY_PATH})

if (NOT OPENGL_FOUND)
   if (MPE_FOUND)
      set (HAVE_GRAPHICS on)
      set (HAVE_MPE on)
      include_directories(${MPE_INCLUDE_DIR})
      if (X11_FOUND)
         include_directories(${X11_INCLUDE_DIR})
      endif (X11_FOUND)
      set (MPE_LIBS ${MPE_LIBRARIES})
      set (MPE_NOMPI_LIBS ${MPE_NOMPI_LIBRARIES})
      set (X11_LIBS ${X11_LIBRARIES})
   endif (MPE_FOUND)
endif (NOT OPENGL_FOUND)

# search for Boost
# this min version isn't correct -- FIXME
#set (BOOST_MIN_VER "1.45.0")
#find_package(Boost ${BOOST_MIN_VER})

find_library(POWER_GADGET IntelPowerGadget)
if (POWER_GADGET)
   set (POWER_GADGET_FOUND on)
   add_definitions(-DHAVE_POWER_GADGET)
   message("-- Detected Intel Power Gadget")
   include_directories(/Library/Frameworks/IntelPowerGadget.framework/Headers)
endif (POWER_GADGET)


#message("MPE_LIBRARIES ${MPE_LIBRARIES}")
#message("MPE_LIBS ${MPE_LIBS}")
#message("MPE_NOMPI_LIBS ${MPE_NOMPI_LIBS}")
#message("OPENGL_LIBRARIES ${OPENGL_LIBRARIES}")
#message("GLUT_LIBRARIES ${GLUT_LIBRARIES}")
#message("X11_LIBRARIES ${X11_LIBRARIES}")
#message("X11_LIBS ${X11_LIBS}")

add_definitions(-DHAVE_CONFIG_H)

# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

#ADDING TESTS CODE####

if (EXISTS "${CMAKE_SOURCE_DIR}/tests" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/tests")
   add_subdirectory(tests)
endif (EXISTS "${CMAKE_SOURCE_DIR}/tests" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/tests")

if (EXISTS "${CMAKE_SOURCE_DIR}/mesh" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/mesh")
   add_subdirectory(mesh)
endif (EXISTS "${CMAKE_SOURCE_DIR}/mesh" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/mesh")

if (EXISTS "${CMAKE_SOURCE_DIR}/s7" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/s7")
   add_subdirectory(s7)
endif (EXISTS "${CMAKE_SOURCE_DIR}/s7" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/s7")

if (EXISTS "${CMAKE_SOURCE_DIR}/l7" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/l7")
   add_subdirectory(l7)
endif (EXISTS "${CMAKE_SOURCE_DIR}/l7" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/l7")

if (EXISTS "${CMAKE_SOURCE_DIR}/graphics" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/graphics")
   add_subdirectory(graphics)
endif (EXISTS "${CMAKE_SOURCE_DIR}/graphics" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/graphics")

if (EXISTS "${CMAKE_SOURCE_DIR}/timer" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/timer")
   add_subdirectory(timer)
endif (EXISTS "${CMAKE_SOURCE_DIR}/timer" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/timer")

if (EXISTS "${CMAKE_SOURCE_DIR}/memstats" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/memstats")
   add_subdirectory(memstats)
endif (EXISTS "${CMAKE_SOURCE_DIR}/memstats" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/memstats")

if (EXISTS "${CMAKE_SOURCE_DIR}/powerstats" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/powerstats")
   add_subdirectory(powerstats)
endif (EXISTS "${CMAKE_SOURCE_DIR}/powerstats" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/powerstats")

if (OpenCL_FOUND)
   if (EXISTS "${CMAKE_SOURCE_DIR}/ezcl" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/ezcl")
      add_subdirectory(ezcl)
   endif (EXISTS "${CMAKE_SOURCE_DIR}/ezcl" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/ezcl")
endif (OpenCL_FOUND)

if (EXISTS "${CMAKE_SOURCE_DIR}/genmalloc" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/genmalloc")
   add_subdirectory(genmalloc)
endif (EXISTS "${CMAKE_SOURCE_DIR}/genmalloc" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/genmalloc")

if (EXISTS "${CMAKE_SOURCE_DIR}/MallocPlus" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/MallocPlus")
   add_subdirectory(MallocPlus)
endif (EXISTS "${CMAKE_SOURCE_DIR}/MallocPlus" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/MallocPlus")

if (EXISTS "${CMAKE_SOURCE_DIR}/hash" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/hash")
   add_subdirectory(hash)
endif (EXISTS "${CMAKE_SOURCE_DIR}/hash" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/hash")

if (EXISTS "${CMAKE_SOURCE_DIR}/crux" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/crux")
   add_subdirectory(crux)
endif (EXISTS "${CMAKE_SOURCE_DIR}/crux" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/crux")

if (EXISTS "${CMAKE_SOURCE_DIR}/PowerParser" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/PowerParser")
   add_subdirectory(PowerParser)
endif (EXISTS "${CMAKE_SOURCE_DIR}/PowerParser" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/PowerParser")

########### benchmark target ###############
add_executable(benchmark benchmark/benchmark.c benchmark/l7_update_host.c)

# Add source files we need for different accelerators
if(L7_CUDA_OFFLOAD)
   enable_language(CUDA)
   if(CMAKE_CUDA_COMPILER)
      message("Compiling benchmark with CUDA accelerator support")
      add_definitions(-DL7_CUDA_OFFLOAD)
      target_sources(benchmark PRIVATE benchmark/l7_update_cuda.cu)
      set_property(TARGET benchmark PROPERTY CUDA_ARCHITECTURES 35 70)
      set_target_properties(benchmark PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
      set_target_properties(benchmark PROPERTIES COMPILE_DEFINITIONS HAVE_CUDA)
   endif(CMAKE_CUDA_COMPILER)
endif(L7_CUDA_OFFLOAD)

if(OpenCL_FOUND)
   message("Compiling benchmark with OpenCL accelerator support")
   add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/l7_update_kern.inc
                  COMMAND ${CMAKE_SOURCE_DIR}/benchmark/embed_source.pl ${CMAKE_SOURCE_DIR}/benchmark/l7_update_kern.cl > l7_update_kern.inc
                  DEPENDS benchmark/state_kern.cl ${CMAKE_SOURCE_DIR}/benchmark/embed_source.pl)
   add_custom_target(l7_update_kern_source ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/l7_update_kern.inc)

   set_target_properties(benchmark PROPERTIES COMPILE_DEFINITIONS HAVE_OPENCL)

   target_sources(benchmark PRIVATE benchmark/l7_update_opencl.c)
   add_custom_target(benchmark/l7_update_opencl.c DEPENDS l7_update_kern_source)

   target_link_libraries(benchmark PRIVATE ${OpenCL_LIBRARIES})
endif(OpenCL_FOUND)

if(OpenMP_FOUND)
   # We have to set this since its not cached otherwise.
   set(OpenMP_C_VERSION ${OpenMP_C_VERSION} CACHE STRING "OpenMP C version")
   target_link_libraries(benchmark PRIVATE OpenMP::OpenMP_C)

  if(L7_OPENMP_OFFLOAD)
      if(OpenMP_C_VERSION VERSION_GREATER_EQUAL "4.5")
         add_definitions(-DL7_OPENMP_OFFLOAD)
         message("Compiling benchmark with OpenMP accelerator support")
         target_sources(benchmark PRIVATE benchmark/l7_update_openmp.c)
         set_property(TARGET benchmark PROPERTY CUDA_ARCHITECTURES 35 70)
         target_link_libraries(benchmark PRIVATE OpenMP::OpenMP_C)
         if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -foffload=nvptx-none=-misa=sm_35")
         endif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
      else (OpenMP_C_VERSION VERSION_GREATER_EQUAL "4.5")
         message("benchmark not using OpenMP accelerator support (VERSION=${OpenMP_C_VERSION})")
      endif(OpenMP_C_VERSION VERSION_GREATER_EQUAL "4.5")
   endif(L7_OPENMP_OFFLOAD)
endif(OpenMP_FOUND)

target_link_libraries(benchmark PRIVATE m)
target_link_libraries(benchmark PRIVATE ${MPI_LIBRARIES})
if(OpenCL_FOUND)
   target_link_libraries(benchmark PRIVATE dl7 ezcl)
elseif (OpenMP_FOUND)
   target_link_libraries(benchmark PRIVATE mpl7)
else(OpenCL_FOUND)
   target_link_libraries(benchmark PRIVATE l7)
endif(OpenCL_FOUND)

########### configure config.h ################
# configure a header file to pass some of the CMake settings
# to the source code
# at bottom because we set things that change config above
configure_file ( "${PROJECT_SOURCE_DIR}/benchmark/config.h.in" "${PROJECT_BINARY_DIR}/config.h")

########### clean files ################
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "state_kernel.inc;clamr;clamr_cpuonly;clamr_gpuonly;clamr_mpionly;clamr_gpucheck;clamr_mpicheck;clamr_checkall;clamr_quo;total_execution_time.log;docs/_build;htmldoc.out;pdfdoc.out;singlehtmldoc.out")

