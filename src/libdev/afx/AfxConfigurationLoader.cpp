#include "afx/AfxConfigurationLoader.hpp"

#include "system/ConfigVariables.hpp"

IAfxConfigurationLoader::~IAfxConfigurationLoader() = default;

AfxConfigurationLoader::AfxConfigurationLoader()
{
}

// virtual
AfxConfigurationLoader::~AfxConfigurationLoader()
{
}

// virtual
AfxConfigurationData* AfxConfigurationLoader::load()
{
    // Caller will be responsible for cleanup
    AfxConfigurationData* config = new AfxConfigurationData();
    config->multisampleBuffers = Config::gfxMsaaBuffers.get();
    config->multisampleSamples = Config::gfxMsaaSamples.get();
    config->watchdogTimeout = Config::watchdogTimeout.get();

    return config;
}
