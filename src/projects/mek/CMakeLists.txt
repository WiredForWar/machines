cmake_minimum_required(VERSION 3.19)

project(mek)

find_package(Threads REQUIRED)

if(mek_IS_TOP_LEVEL)
    add_subdirectory(../../../external ${CMAKE_CURRENT_BINARY_DIR}/external)
    add_subdirectory(../../libdev/rendezvous ${CMAKE_CURRENT_BINARY_DIR}/rendezvous)
    target_include_directories(rendezvous PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../libdev)
endif()

add_executable(mek
    mek_main.cpp
    service/RendezvousConfig.hpp
    service/RendezvousServer.cpp
    service/RendezvousServer.hpp
    service/SessionStore.cpp
    service/SessionStore.hpp
)

target_include_directories(mek PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mek
    PRIVATE
        LibDev::rendezvous
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Threads::Threads
)

add_executable(mek_client
    client/TestClient.cpp
)

target_link_libraries(mek_client
    PRIVATE
        LibDev::rendezvous
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)
