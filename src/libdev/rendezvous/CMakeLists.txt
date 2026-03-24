add_library(rendezvous STATIC)
add_library(LibDev::rendezvous ALIAS rendezvous)

target_sources(rendezvous
    PRIVATE
        HttpStatus.hpp
        RendezvousClient.cpp
        RendezvousClient.hpp
        RendezvousTypes.hpp
)

target_link_libraries(rendezvous
    PUBLIC
        httplib::httplib
    PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)
