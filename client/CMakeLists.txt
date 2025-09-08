# client/CMakeLists.txt

find_package(asio REQUIRED)
find_package(Protobuf REQUIRED)

file(GLOB_RECURSE CLIENT_SOURCES "src/*.cpp")

add_executable(client ${CLIENT_SOURCES})

target_include_directories(client PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(client PUBLIC
    common
    asio::asio
    ${Protobuf_LIBRARIES}
)
if(WIN32)
    target_link_libraries(client PRIVATE ws2_32 wsock32)
endif()
