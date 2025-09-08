
find_package(Protobuf REQUIRED)

set(PROTOBUF_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/proto")
file(GLOB PROTO_FILES "${PROTOBUF_SOURCE_DIR}/*.proto")

set(PROTOBUF_GENERATE_CPP_APPEND_PATH TRUE)
include_directories(${PROTOBUF_SOURCE_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp")

add_library(common STATIC ${COMMON_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(common PUBLIC ${Protobuf_LIBRARIES})