# server/CMakeLists.txt 

find_package(asio CONFIG REQUIRED)
find_package(SOCI CONFIG REQUIRED COMPONENTS mysql)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")
add_executable(server ${SERVER_SOURCES})

target_include_directories(server PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
     ${CMAKE_BINARY_DIR}
    "${CMAKE_BINARY_DIR}/common"
)

target_link_libraries(server PRIVATE
    common     
    asio::asio
    SOCI::soci_core
    SOCI::soci_mysql
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()
