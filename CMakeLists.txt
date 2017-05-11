# Minimum CMake required
cmake_minimum_required(VERSION 2.8.12)

# Project
project(protobuf-udp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(protobuf_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required protobuf package
find_package(protobuf CONFIG REQUIRED)

if(protobuf_VERBOSE)
  message(STATUS "Using Protocol Buffers ${Protobuf_VERSION}")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

# http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
if(MSVC AND protobuf_MSVC_STATIC_RUNTIME)
  foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
  endforeach()
endif()


set(PROTOS addressbook.proto)
set(PROTO_SRCS "")
foreach(proto_file ${PROTOS})
  get_filename_component(proto_file_abs ${proto_file} ABSOLUTE)
  get_filename_component(basename ${proto_file} NAME_WE)
  set(generated_files ${basename}.pb.cc ${basename}.pb.h)
  list(APPEND PROTO_SRCS ${generated_files})

  add_custom_command(
    OUTPUT ${generated_files}
    COMMAND protoc
    ARGS --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR} ${proto_file_abs}
    COMMENT "Generating ${generated_files} from ${proto_file}"
    VERBATIM
  )
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_library(udp src/udp.cc)

#Executable setup
set(executable_name add_person_cpp)
add_executable(${executable_name} src/add_person.cc ${PROTO_SRCS})
target_link_libraries(${executable_name} ${PROTOBUF_LIBRARIES})

set(executable_name list_people_cpp)
add_executable(${executable_name} src/list_people.cc ${PROTO_SRCS})
target_link_libraries(${executable_name} ${PROTOBUF_LIBRARIES})

#Executable setup
set(executable_name send_addressbook_cpp)
add_executable(${executable_name} src/send_addressbook.cc ${PROTO_SRCS})
target_link_libraries(${executable_name} ${PROTOBUF_LIBRARIES} udp)

set(executable_name recv_addressbook_cpp)
add_executable(${executable_name} src/recv_addressbook.cc ${PROTO_SRCS})
target_link_libraries(${executable_name} ${PROTOBUF_LIBRARIES} udp)