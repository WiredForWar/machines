/*
 * R E G I S T R Y . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "system/registry.hpp"
#include "profiler/profiler.hpp"

#include "recorder/recorder.hpp"
#include "recorder/private/recpriv.hpp"
#include "ctl/map.hpp"
#include "ctl/list.hpp"

#include "utility/rapidxml_utils.hpp"
#include "utility/rapidxml_ext.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

class SysRegistryImpl
{
private:
    SysRegistryImpl(std::string fileName = "config.xml")
        : regFile_(fileName)
        , xmlFile_(nullptr)
    {
        bool create = false;
        try
        {
            xmlFile_ = new rapidxml::file<>(regFile_.c_str());
            doc_.parse<0>(xmlFile_->data());
        }
        catch (const rapidxml::parse_error& e)
        {
            std::cerr << e.what() << " here: " << e.where<char>() << std::endl;
            create = true;
        }
        catch (...)
        {
            create = true;
        }
        if (create)
        {
            std::cout << "Failed to parse config file, new empty one is created." << std::endl;
            doc_.clear();
            char* node_name = doc_.allocate_string("keys");
            rapidxml::xml_node<>* child = doc_.allocate_node(rapidxml::node_element, node_name);
            doc_.append_node(child);
        }
    };
    ~SysRegistryImpl()
    {
        doc_.clear();
        delete xmlFile_;
    };
    void store()
    {
        // Save to file
        std::ofstream file_stored(regFile_.c_str());
        file_stored << doc_;
        file_stored.close();
    }

    friend class SysRegistry;
    using KeyHandleMap = ctl_map<std::string, SysRegistry::KeyHandle, std::less<std::string>>;
    KeyHandleMap keyHandleMap_;
    using OpenKeys = ctl_list<SysRegistry::KeyHandle>;
    OpenKeys openKeys_;
    std::string currentStubKey_;

    rapidxml::xml_document<> doc_;
    rapidxml::file<>* xmlFile_; // Default template is char
    std::string regFile_;
};

#define CB_SysRegistry_DEPIMPL()                                                                                       \
    PRE(pImpl_)                                                                                                        \
    CB_DEPIMPL(SysRegistryImpl::KeyHandleMap, keyHandleMap_)                                                           \
    CB_DEPIMPL(SysRegistryImpl::OpenKeys, openKeys_)                                                                   \
    CB_DEPIMPL(std::string, currentStubKey_)                                                                           \
    CB_DEPIMPL(rapidxml::xml_document<>, doc_)

// static
SysRegistry& SysRegistry::instance()
{
    static SysRegistry instance_;
    return instance_;
}

SysRegistry::SysRegistry()
    : pImpl_(new SysRegistryImpl)
{

    TEST_INVARIANT;
}

SysRegistry::~SysRegistry()
{
    CB_SysRegistry_DEPIMPL();
    TEST_INVARIANT;
    while (openKeys_.size())
        closeKey(openKeys_.front());
    delete pImpl_;
}

void SysRegistry::reload()
{
    while (pImpl_->openKeys_.size())
        closeKey(pImpl_->openKeys_.front());
    delete pImpl_;

    pImpl_ = new SysRegistryImpl;
}

void SysRegistry::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const SysRegistry& t)
{

    o << "SysRegistry " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "SysRegistry " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

SysRegistry::ReturnValue SysRegistry::onlyOpenKey(const std::string& keyName, SysRegistry::KeyHandle* pOpenedKey)
{
    ReturnValue result = SUCCESS;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryReturnValue();
        *pOpenedKey = RecRecorderPrivate::instance().playbackRegistryKey();
    }
    else
    {
        result = onlyOpenKeyNoRecord(keyName, pOpenedKey);

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            RecRecorderPrivate::instance().recordRegistryReturnValue(result);
            RecRecorderPrivate::instance().recordRegistryKey(*pOpenedKey);
        }
    }

    return result;
}

SysRegistry::ReturnValue
SysRegistry::onlyOpenKeyNoRecord(const std::string& keyName, SysRegistry::KeyHandle* pOpenedKey)
{
    ReturnValue result = SUCCESS;

    CB_SysRegistry_DEPIMPL();
    std::string actualKeyName = currentStubKey_;
    actualKeyName += "\\";
    actualKeyName += keyName;

    rapidxml::xml_node<>* root_node = doc_.first_node();
    for (rapidxml::xml_node<>* a = root_node->first_node(); a; a = a->next_sibling())
    {
        std::string currentVal(a->first_attribute()->value());
        if (currentVal.find(keyName) != std::string::npos)
            return SUCCESS;
    }
    return FAILED;
}

SysRegistry::ReturnValue SysRegistry::openKey(const std::string& keyName, SysRegistry::KeyHandle* pOpenedKey)
{
    ReturnValue result = SUCCESS;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryReturnValue();
        *pOpenedKey = RecRecorderPrivate::instance().playbackRegistryKey();
    }
    else
    {
        CB_SysRegistry_DEPIMPL();
        std::string actualKeyName = currentStubKey_;
        actualKeyName += "\\";
        actualKeyName += keyName;

        result = FAILED;
        rapidxml::xml_node<>* root = doc_.first_node();
        for (rapidxml::xml_node<>* a = root->first_node(); a; a = a->next_sibling())
        {
            std::string currentVal(a->first_attribute()->value());
            if (currentVal.find(keyName) != std::string::npos)
            {
                result = SUCCESS;
                break;
            }
        }

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            RecRecorderPrivate::instance().recordRegistryReturnValue(result);
            RecRecorderPrivate::instance().recordRegistryKey(*pOpenedKey);
        }
    }

    return result;
}

SysRegistry::ReturnValue SysRegistry::deleteKey(const std::string& keyName)
{
    ReturnValue result = SUCCESS;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryReturnValue();
    }
    else
    {
        CB_SysRegistry_DEPIMPL();
        result = FAILED;
        rapidxml::xml_node<>* root = doc_.first_node();
        for (rapidxml::xml_node<>* a = root->first_node(); a; a = a->next_sibling())
        {
            std::string currentVal(a->first_attribute()->value());
            if (currentVal.find(keyName) != std::string::npos)
            {
                root->remove_node(a);
                result = SUCCESS;
            }
        }
        if (result == SUCCESS)
            pImpl_->store();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordRegistryReturnValue(result);
    }

    return result;
}

SysRegistry::ReturnValue SysRegistry::queryValue(
    SysRegistry::KeyHandle key,
    const std::string& valueName,
    SysRegistry::DataType dataType,
    void* pBuffer,
    int* pBufferSize)
{
    ReturnValue result = SUCCESS;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryReturnValue();
        *pBufferSize = RecRecorderPrivate::instance().playbackRegistryBuffer(pBuffer);
    }
    else
    {
        std::string value;
        result = queryValueNoRecord(key, valueName, value);
        strncpy((char*)pBuffer, value.c_str(), value.length());

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            RecRecorderPrivate::instance().recordRegistryReturnValue(result);
            RecRecorderPrivate::instance().recordRegistryBuffer(pBuffer, *pBufferSize);
        }
    }

    return result;
}

SysRegistry::ReturnValue
SysRegistry::queryValueNoRecord(SysRegistry::KeyHandle key, const std::string& valueName, std::string& target)
{
    ReturnValue result = SUCCESS;

    CB_SysRegistry_DEPIMPL();
    result = FAILED;
    rapidxml::xml_node<>* root = doc_.first_node();
    for (rapidxml::xml_node<>* a = root->first_node(); a; a = a->next_sibling())
    {
        if (a->first_attribute()->value() == valueName)
        {
            target = a->value();
            result = SUCCESS;
        }
    }
    return result;
}

SysRegistry::ReturnValue
SysRegistry::setValue(SysRegistry::KeyHandle key, const std::string& valueName, const std::string& value)
{
    ReturnValue result = SUCCESS;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryReturnValue();
    }
    else
    {
        CB_SysRegistry_DEPIMPL();
        bool create = true;
        rapidxml::xml_node<>* root = doc_.first_node();
        char* node_value = doc_.allocate_string(value.c_str());
        for (rapidxml::xml_node<>* a = root->first_node(); a; a = a->next_sibling())
        {
            if (a->first_attribute()->value() == valueName)
            {
                rapidxml::xml_node<>* real_thing = a->first_node();
                if (real_thing != nullptr // these checks just demonstrate that
                    && real_thing->next_sibling() == nullptr // it is there and how it is located
                    && real_thing->type() == rapidxml::node_data) // when element does contain text data
                {
                    real_thing->value(node_value);
                }
                create = false;
                break;
            }
        }
        // Value does not exist add a new node
        if (create)
        {
            static const char key[] = "key";
            static const char name[] = "name";
            rapidxml::xml_node<>* child = doc_.allocate_node(rapidxml::node_element, key);
            char* node_name = doc_.allocate_string(valueName.c_str());
            child->append_attribute(doc_.allocate_attribute(name, node_name));
            child->value(node_value);
            root->append_node(child);
        }
        pImpl_->store();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordRegistryReturnValue(result);
    }

    return result;
}

SysRegistry::ReturnValue SysRegistry::deleteValue(SysRegistry::KeyHandle key, const std::string& valueName)
{
    ReturnValue result = SUCCESS;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryReturnValue();
    }
    else
    {
        CB_SysRegistry_DEPIMPL();
        result = FAILED;
        rapidxml::xml_node<>* root = doc_.first_node();
        for (rapidxml::xml_node<>* a = root->first_node(); a; a = a->next_sibling())
        {
            if (a->first_attribute()->value() == valueName)
            {
                root->remove_node(a);
                pImpl_->store();
                result = SUCCESS;
                break;
            }
        }

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordRegistryReturnValue(result);
    }

    return result;
}

SysRegistry::ReturnValue SysRegistry::closeKey(KeyHandle key)
{
    ReturnValue result = SUCCESS;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryReturnValue();
    }
    else
    {
        result = closeKeyNoRecord(key);

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordRegistryReturnValue(result);
    }

    return result;
}

SysRegistry::ReturnValue SysRegistry::closeKeyNoRecord(KeyHandle key)
{
    ReturnValue result = SUCCESS;

    //  If we're playing back it is entirely possible we might fail the pre condition
    //  because the openKeys_ map is not properly maintained. We therefore only run
    //  this section of code if we're not playing back.

    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        CB_SysRegistry_DEPIMPL();
    }

    return result;
}

std::string
SysRegistry::queryStringValue(const std::string& keyName, const std::string& valueName, const std::string& defaultValue)
{
    std::string result = defaultValue;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryStringValue();
    }
    else
    {
        CB_SysRegistry_DEPIMPL();
        SysRegistry::KeyHandle handle = 0;

        bool doCloseKey = true;
        std::string actualKeyName = keyName;
        actualKeyName += "\\";
        actualKeyName += valueName;
        if (keyHandleMap_.end() != keyHandleMap_.find(actualKeyName))
        {
            doCloseKey = false;
        }

        //      if( SUCCESS == onlyOpenKeyNoRecord( keyName, &handle, root ) )
        {
            //          if( SUCCESS == queryValueNoRecord( handle, valueName, result) )
            //              result = std::string ( array );
            queryValueNoRecord(handle, actualKeyName, result);

            if (doCloseKey)
                closeKeyNoRecord(handle);
        }

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordRegistryStringValue(result);
    }

    return result;
}

int SysRegistry::queryIntegerValue(const std::string& keyName, const std::string& valueName, int defaultValue)
{
    int result = defaultValue;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        result = RecRecorderPrivate::instance().playbackRegistryIntegerValue();
    }
    else
    {
        CB_SysRegistry_DEPIMPL();
        SysRegistry::KeyHandle handle;

        bool doCloseKey = true;
        std::string actualKeyName = keyName;
        actualKeyName += "\\";
        actualKeyName += valueName;
        if (keyHandleMap_.end() != keyHandleMap_.find(actualKeyName))
        {
            doCloseKey = false;
        }

        //      if( SUCCESS == onlyOpenKeyNoRecord( keyName, &handle ) )
        {
            std::string value;
            if (SUCCESS == queryValueNoRecord(handle, actualKeyName, value))
                result = atoi(value.c_str());
            if (doCloseKey)
                closeKeyNoRecord(handle);
        }

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            RecRecorderPrivate::instance().recordRegistryIntegerValue(result);
        }
    }

    return result;
}

bool SysRegistry::queryBooleanValue(const std::string& keyName, const std::string& valueName, bool defaultValue)
{
    return queryIntegerValue(keyName, valueName, defaultValue ? 1 : 0) == 1;
}

// The set functions will create the key if it isn't present
void SysRegistry::setStringValue(const std::string& keyName, const std::string& valueName, const std::string& value)
{
    CB_SysRegistry_DEPIMPL();
    SysRegistry::KeyHandle handle;
    bool doCloseKey = true;

    std::string actualKeyName = keyName;
    actualKeyName += "\\";
    actualKeyName += valueName;
    if (keyHandleMap_.end() != keyHandleMap_.find(actualKeyName))
    {
        doCloseKey = false;
    }

    //  if( SUCCESS == openKey( keyName, &handle, root ) )
    {
        setValue(handle, actualKeyName, value);
        if (doCloseKey)
            closeKeyNoRecord(handle);
    }
}

void SysRegistry::setIntegerValue(const std::string& keyName, const std::string& valueName, int value)
{
    CB_SysRegistry_DEPIMPL();
    SysRegistry::KeyHandle handle;

    bool doCloseKey = true;
    std::string actualKeyName = keyName;
    actualKeyName += "\\";
    actualKeyName += valueName;
    if (keyHandleMap_.end() != keyHandleMap_.find(actualKeyName))
    {
        doCloseKey = false;
    }

    //  if( SUCCESS == openKey( keyName, &handle, root ) )
    {
        std::string valueStr = std::to_string(value);
        setValue(handle, actualKeyName, valueStr);
        if (doCloseKey)
            closeKeyNoRecord(handle);
    }
}

const std::string& SysRegistry::currentStubKey() const
{
    CB_SysRegistry_DEPIMPL();
    return currentStubKey_;
}

void SysRegistry::currentStubKey(const std::string& newStubKey)
{
    CB_SysRegistry_DEPIMPL();
    currentStubKey_ = newStubKey;
}

/* End REGISTRY.CPP *************************************************/
