cmake_minimum_required(VERSION 3.10)
project(boost-runtime-test CXX)

set(CMAKE_CXX_STANDARD 14)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Names of the boost libraries to test
set(BOOST_RUNTIME_TEST_LIBS "" CACHE STRING "Boost libraries to test")
# Boost source tree, the upstream tests are built straight from it
set(BOOST_SOURCE_DIR "" CACHE PATH "Boost source tree")
# Set to build a boost-python extension module
set(BOOST_RUNTIME_TEST_PYTHON_INCLUDE_DIR "" CACHE PATH "Python include directory")
set(BOOST_RUNTIME_TEST_PYTHON_SITE_DIR "" CACHE STRING "Python module install directory")

# Upstream test of each library, relative to the boost source tree.
# Libraries without an entry are built from src/boost-<lib>-test.cpp.
set(upstream_atomic libs/atomic/test/atomic_api.cpp)

set(upstream_chrono libs/chrono/test/test_11012.cpp)

set(upstream_container libs/container/test/synchronized_pool_resource_test.cpp)

set(upstream_context libs/context/test/test_fcontext.cpp)

set(upstream_contract libs/contract/test/check/audit_disabled.cpp)

set(upstream_date_time libs/date_time/test/testgregorian_calendar.cpp)

set(upstream_exception libs/exception/test/exception_test.cpp)
set(definitions_exception BOOST_TEST_DYN_LINK)

set(upstream_filesystem libs/filesystem/test/quick.cpp)

set(upstream_graph libs/graph/test/copy.cpp)

set(upstream_json libs/json/test/string.cpp libs/json/test/main.cpp)
set(includes_json libs/json/test)

set(upstream_log libs/log/test/run/attr_attribute_set.cpp)
set(component_log log_setup)
set(components_log unit_test_framework)
set(definitions_log BOOST_TEST_DYN_LINK BOOST_LOG_DYN_LINK BOOST_LOG_SETUP_DYN_LINK)
set(includes_log libs/log/test/common)

set(upstream_math libs/math/test/test_tr1.cpp)
set(component_math math_c99 math_c99f math_tr1 math_tr1f)
set(components_math unit_test_framework)
set(definitions_math BOOST_TEST_DYN_LINK)

set(upstream_nowide libs/nowide/test/test_convert.cpp)

set(upstream_program_options libs/program_options/test/variable_map_test.cpp)

set(upstream_random libs/random/test/test_ranlux24.cpp)
set(components_random unit_test_framework)
set(definitions_random BOOST_TEST_DYN_LINK)

set(upstream_regex libs/regex/test/quick.cpp)

set(upstream_serialization libs/serialization/test/test_inclusion2.cpp)

set(upstream_stacktrace libs/stacktrace/test/test_trivial.cpp)
set(component_stacktrace stacktrace_basic)

set(upstream_system libs/system/test/error_condition_test.cpp)

set(upstream_test libs/test/test/framework-ts/decorators-datatestcase-test.cpp)
set(component_test unit_test_framework)
set(definitions_test BOOST_TEST_DYN_LINK)
set(includes_test libs/test/test/framework-ts)

set(upstream_thread libs/thread/test/test_2501.cpp)

set(upstream_timer libs/timer/test/cpu_timer_test.cpp)

set(upstream_type_erasure libs/type_erasure/test/test_dynamic_any_cast.cpp)
set(components_type_erasure unit_test_framework)
set(definitions_type_erasure BOOST_TEST_DYN_LINK)

function(boost_components out)
  set(components)
  foreach(lib IN LISTS ARGN)
    if(DEFINED component_${lib})
      list(APPEND components ${component_${lib}})
    else()
      list(APPEND components ${lib})
    endif()
    list(APPEND components ${components_${lib}})
  endforeach()
  list(REMOVE_DUPLICATES components)
  set(${out} ${components} PARENT_SCOPE)
endfunction()

boost_components(all_components ${BOOST_RUNTIME_TEST_LIBS})
if(BOOST_RUNTIME_TEST_PYTHON_INCLUDE_DIR)
  list(APPEND all_components python)
endif()

find_package(Boost REQUIRED CONFIG COMPONENTS ${all_components})

foreach(lib IN LISTS BOOST_RUNTIME_TEST_LIBS)
  string(REPLACE "_" "-" name ${lib})
  set(target boost-${name}-test)

  boost_components(components ${lib})
  set(boost_targets)
  foreach(component IN LISTS components)
    list(APPEND boost_targets Boost::${component})
  endforeach()

  if(DEFINED upstream_${lib})
    set(sources)
    foreach(source IN LISTS upstream_${lib})
      list(APPEND sources ${BOOST_SOURCE_DIR}/${source})
    endforeach()
  else()
    set(sources src/${target}.cpp)
  endif()

  add_executable(${target} ${sources})
  target_link_libraries(${target} Boost::headers ${boost_targets}
                        Threads::Threads ${CMAKE_DL_LIBS})
  target_compile_definitions(${target} PRIVATE ${definitions_${lib}})
  foreach(include IN LISTS includes_${lib})
    target_include_directories(${target} PRIVATE ${BOOST_SOURCE_DIR}/${include})
  endforeach()
  install(TARGETS ${target} RUNTIME DESTINATION bin)
endforeach()

if(BOOST_RUNTIME_TEST_PYTHON_INCLUDE_DIR)
  add_library(boost_python_test MODULE src/boost-python-test.cpp)
  set_target_properties(boost_python_test PROPERTIES PREFIX "")
  target_include_directories(boost_python_test PRIVATE ${BOOST_RUNTIME_TEST_PYTHON_INCLUDE_DIR})
  target_link_libraries(boost_python_test Boost::headers Boost::python)
  install(TARGETS boost_python_test LIBRARY DESTINATION ${BOOST_RUNTIME_TEST_PYTHON_SITE_DIR})
endif()
