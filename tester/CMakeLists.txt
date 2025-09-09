# tester/CMakeLists.txt

find_package(asio CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE TESTER_SOURCES "src/*.cpp")

add_executable(tester "src/main.cpp")


target_link_libraries(tester PRIVATE
    common 
    client_lib 
)

if(WIN32)
    target_link_libraries(tester PRIVATE ws2_32)
endif()