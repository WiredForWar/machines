#include "service/RendezvousConfig.hpp"
#include "service/RendezvousServer.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <charconv>
#include <iostream>
#include <string>

namespace
{

bool parseUint16(const char* text, uint16_t* outValue)
{
    const char* end = text;
    while (*end != '\0')
    {
        ++end;
    }

    unsigned long parsedValue = 0;
    const std::errc ec = std::from_chars(text, end, parsedValue).ec;
    if (ec != std::errc{} || parsedValue > std::numeric_limits<uint16_t>::max())
    {
        return false;
    }

    *outValue = static_cast<uint16_t>(parsedValue);
    return true;
}

bool parseUint32(const char* text, uint32_t* outValue)
{
    const char* end = text;
    while (*end != '\0')
    {
        ++end;
    }

    unsigned long parsedValue = 0;
    const std::errc ec = std::from_chars(text, end, parsedValue).ec;
    if (ec != std::errc{} || parsedValue > std::numeric_limits<uint32_t>::max())
    {
        return false;
    }

    *outValue = static_cast<uint32_t>(parsedValue);
    return true;
}

void printUsage()
{
    std::cout << "Usage: mek [--bind-address addr] [--port N] [--session-timeout seconds] [--no-metrics]" << std::endl;
}

void initLogging()
{
    try
    {
        std::shared_ptr<spdlog::logger> logger = spdlog::get("mek");
        if (!logger)
        {
            logger = spdlog::stderr_color_mt("mek");
        }

        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cerr << "Failed to initialize logging: " << ex.what() << std::endl;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    initLogging();

    RendezvousConfig config{
        .bindAddress = "0.0.0.0",
        .port = 8080,
        .metricsEnabled = true,
        .sessionTimeout = std::chrono::minutes(2),
    };

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument = argv[i];
        if (argument == "--bind-address" && i + 1 < argc)
        {
            config.bindAddress = argv[++i];
        }
        else if (argument == "--port" && i + 1 < argc)
        {
            uint16_t portValue = 0;
            if (!parseUint16(argv[++i], &portValue) || portValue == 0)
            {
                spdlog::error("Invalid port value");
                return 1;
            }
            config.port = portValue;
        }
        else if (argument == "--session-timeout" && i + 1 < argc)
        {
            uint32_t timeoutValue = 0;
            if (!parseUint32(argv[++i], &timeoutValue) || timeoutValue == 0)
            {
                spdlog::error("Invalid session timeout value");
                return 1;
            }
            config.sessionTimeout = std::chrono::seconds(timeoutValue);
        }
        else if (argument == "--no-metrics")
        {
            config.metricsEnabled = false;
        }
        else if (argument == "--help")
        {
            printUsage();
            return 0;
        }
        else
        {
            spdlog::error("Unknown argument: {}", argument);
            printUsage();
            return 1;
        }
    }

    RendezvousServer server(config);
    const bool ok = server.run();
    return ok ? 0 : 1;
}
