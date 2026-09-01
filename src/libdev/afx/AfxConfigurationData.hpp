#pragma once

struct AfxConfigurationData
{
    // Anti aliasing
    int multisampleBuffers;
    int multisampleSamples;

    // Seconds without progress before a hang report is written; zero disables.
    int watchdogTimeout;
};
