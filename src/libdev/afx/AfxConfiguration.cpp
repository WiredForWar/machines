#include <cmath>
#include "afx/AfxConfiguration.hpp"

AfxConfiguration::AfxConfiguration()
    : AfxConfiguration(&defaultLoader_)
{
}

AfxConfiguration::AfxConfiguration(IAfxConfigurationLoader* loader)
{
    pLoader_ = loader;
    configValidated_ = false;
    pConfigData_ = nullptr;
}

AfxConfiguration::~AfxConfiguration()
{
    if (pConfigData_ != nullptr)
    {
        delete pConfigData_;
    }
}

const AfxConfigurationData& AfxConfiguration::getConfig()
{
    if (pConfigData_ == nullptr)
    {
        pConfigData_ = pLoader_->load();
    }

    if (not configValidated_)
    {
        // Add validations here:
        validateAntiAliasingSettings(*pConfigData_);

        configValidated_ = true;
    }

    return *pConfigData_;
}

void AfxConfiguration::validateAntiAliasingSettings(AfxConfigurationData& config)
{
    if (config.multisampleSamples <= 0)
    {
        config.multisampleBuffers = 0;
        config.multisampleSamples = 0;
        return;
    }

    config.multisampleBuffers = config.multisampleBuffers > 0 ? 1 : 0;
    if (config.multisampleSamples == 1)
    {
        config.multisampleSamples = 2;
        return;
    }

    constexpr int ValidValues[] = { 16, 8, 4, 2 };
    for (int Value : ValidValues)
    {
        if (config.multisampleSamples >= Value)
        {
            config.multisampleSamples = Value;
            break;
        }
    }
}
