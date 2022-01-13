cmake_minimum_required(VERSION 3.12.0)

project(grpc_daq_cpp_demo C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif(MSVC)

add_subdirectory(third_party/grpc ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_REFLECTION grpc++_reflection)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
set(_GRPC_GRPCPP grpc++)
set(_PROTOBUF_LIBPROTOBUF libprotobuf)

set(proto_dir "${CMAKE_SOURCE_DIR}/source/proto")
set(nidaqmx_proto "${proto_dir}/nidaqmx.proto")
set(session_proto "${proto_dir}/session.proto")
set(grpc_generated_source_dir "${CMAKE_CURRENT_BINARY_DIR}/proto")
file(MAKE_DIRECTORY ${grpc_generated_source_dir})
set(generated_implementation_files 
    "${grpc_generated_source_dir}/session.pb.cc"
    "${grpc_generated_source_dir}/session.grpc.pb.cc"
    "${grpc_generated_source_dir}/nidaqmx.pb.cc"
    "${grpc_generated_source_dir}/nidaqmx.grpc.pb.cc"
)

set(generated_header_files 
    "${grpc_generated_source_dir}/session.pb.h"
    "${grpc_generated_source_dir}/session.grpc.pb.h"
    "${grpc_generated_source_dir}/nidaqmx.pb.h"
    "${grpc_generated_source_dir}/nidaqmx.grpc.pb.h"
)


add_custom_command(
    OUTPUT ${generated_implementation_files} ${generated_header_files}
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out ${grpc_generated_source_dir}
        --cpp_out ${grpc_generated_source_dir}
        -I ${CMAKE_SOURCE_DIR}/third_party/grpc/third_party/protobuf/src/
        -I ${proto_dir}
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        session.proto nidaqmx.proto
    DEPENDS "${nidaqmx_proto}" "${session_proto}"
)


add_executable(grpc_daq_cpp_demo
    ${CMAKE_SOURCE_DIR}/source/grpc_daq_cpp_demo.cpp
    ${generated_implementation_files}
)

target_link_libraries(grpc_daq_cpp_demo
   ${_GRPC_GRPCPP}
   ${_PROTOBUF_LIBPROTOBUF}
)

target_include_directories(grpc_daq_cpp_demo
    PRIVATE
        ${grpc_generated_source_dir}
)