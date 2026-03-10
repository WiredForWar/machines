/*
 * R E G I S T R Y . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    SysRegistry

    Allows access to the windows registry.
*/

#ifndef _SYSTEM_REGISTRY_HPP
#define _SYSTEM_REGISTRY_HPP

#include "base/base.hpp"

#include <memory>

class SysRegistryImpl;

class SysRegistry
// Canonical form revoked
{
public:
    //  Singleton class
    static SysRegistry& instance();
    ~SysRegistry();

    void reload();

    // complex interface for accessing the registry properly but with no windows dependancies
    enum ReturnValue
    {
        SUCCESS,
        FAILED
    };

    // This is the same as above but will create the key if necessary - this is normal use
    ReturnValue deleteKey(const std::string& keyName);
    ReturnValue setValue(const std::string& valueName, const std::string& value);
    ReturnValue deleteValue(const std::string& valueName);

    // more simple interface but less efficient as it will open and close keys and each call
    std::string queryStringValue(
        const std::string& keyName,
        const std::string& valueName,
        const std::string& defaultValue = std::string());
    int queryIntegerValue(const std::string& keyName, const std::string& valueName, int defaultValue = 0);
    bool queryBooleanValue(const std::string& keyName, const std::string& valueName, bool defaultValue = false);
    // The set functions will create the key if it isn't present
    void setStringValue(const std::string& keyName, const std::string& valueName, const std::string& value);
    void setIntegerValue(const std::string& keyName, const std::string& valueName, int value);

    ReturnValue queryValueNoRecord(const std::string& valueName, std::string&);

private:
    friend std::ostream& operator<<(std::ostream& o, const SysRegistry& t);

    SysRegistry(const SysRegistry&);
    SysRegistry& operator=(const SysRegistry&);

    SysRegistry();
    std::unique_ptr<SysRegistryImpl> pImpl_;
};

#endif

/* End REGISTRY.HPP *************************************************/
