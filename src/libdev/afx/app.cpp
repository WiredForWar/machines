#include "afx/app.hpp"

AfxApp::AfxApp()
{
}

// virtual
AfxApp::~AfxApp()
{
}

AfxApp::ExitStatus AfxApp::run()
{
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

const std::string& AfxApp::version() const
{
    return version_;
}

const std::string& AfxApp::buildVersion() const
{
    return buildVersion_;
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
