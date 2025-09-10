# common/CMakeLists.txt
find_package(protobuf CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "proto/chat.proto")
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp")

add_library(common STATIC 
    ${COMMON_SOURCES}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(common INTERFACE protobuf::libprotobuf)
