#include "afx/app.hpp"

#include <iostream>

AfxApp::AfxApp()
{
}

// virtual
AfxApp::~AfxApp()
{
}

AfxApp::ExitStatus AfxApp::run()
{
    if (args_.size() == 1)
    {
        if (args_.at(0) == "--version")
        {
            std::cout << name() << " " << version() << " (build " << buildVersion() << ")" << std::endl;
            if (!buildInfo().empty())
            {
                std::cout << "Build: " << buildInfo() << std::endl;
            }
            return EXIT_OK;
        }
    }

    if (startup())
    {
        coreLoop();
        shutdown();
    }

    return exitStatus();
}

// virtual
bool AfxApp::startup()
{
    if (! OSStartup())
        return false;

    if (! clientStartup())
    {
        OSShutdown();
        return false;
    }

    clientStarted();

    return true;
}

// virtual
void AfxApp::shutdown()
{
    clientShutdown();
    OSShutdown();
}

AfxApp::ExitStatus AfxApp::exitStatus() const
{
    return exitStatus_;
}

void AfxApp::exitStatus(AfxApp::ExitStatus es)
{
    exitStatus_ = es;
}

void AfxApp::setArgs(const AfxInvokeArgs& a)
{
    args_ = a;
}

// virtual
const std::string& AfxApp::name() const
{
    return appName_;
}

std::string_view AfxApp::version() const
{
    return version_;
}

std::string_view AfxApp::buildVersion() const
{
    return buildVersion_.empty() ? std::string_view("unknown") : buildVersion_;
}

void AfxApp::setAppName(const std::string& name)
{
    appName_ = name;
}

void AfxApp::setVersion(const std::string& version)
{
    version_ = version;
}

void AfxApp::setBuildVersion(const std::string& buildVersion)
{
    buildVersion_ = buildVersion;
}

std::string_view AfxApp::buildInfo() const
{
    return buildInfo_;
}

void AfxApp::setBuildInfo(const std::string& buildInfo)
{
    buildInfo_ = buildInfo;
}
