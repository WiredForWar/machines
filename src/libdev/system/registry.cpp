/*
 * R E G I S T R Y . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "system/registry.hpp"
#include "profiler/profiler.hpp"

#include "recorder/recorder.hpp"
#include "recorder/private/recpriv.hpp"

#include "utility/rapidxml_utils.hpp"
#include "utility/rapidxml_ext.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

class SysRegistryImpl
{
public:
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
    std::string currentStubKey_;

    rapidxml::xml_document<> doc_;
    rapidxml::file<>* xmlFile_; // Default template is char
    std::string regFile_;
};

#define CB_SysRegistry_DEPIMPL()                                                                                       \
    PRE(pImpl_)                                                                                                        \
    CB_DEPIMPL(std::string, currentStubKey_)                                                                           \
    CB_DEPIMPL(rapidxml::xml_document<>, doc_)

// static
SysRegistry& SysRegistry::instance()
{
    static SysRegistry instance_;
    return instance_;
}

SysRegistry::SysRegistry()
    : pImpl_(std::make_unique<SysRegistryImpl>())
{

    TEST_INVARIANT;
}

SysRegistry::~SysRegistry()
{
    CB_SysRegistry_DEPIMPL();
    TEST_INVARIANT;
}

void SysRegistry::reload()
{
    pImpl_ = std::make_unique<SysRegistryImpl>();
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
        result = queryValueNoRecord(valueName, value);
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
SysRegistry::queryValueNoRecord(const std::string& valueName, std::string& target)
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
SysRegistry::setValue(const std::string& valueName, const std::string& value)
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
                if (real_thing == nullptr)
                {
                    a->value(node_value);
                }
                else if (
                    // these checks just demonstrate that
                    real_thing->next_sibling() == nullptr // it is there and how it is located
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

SysRegistry::ReturnValue SysRegistry::deleteValue(const std::string& valueName)
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

        std::string actualKeyName = keyName;
        actualKeyName += "\\";
        actualKeyName += valueName;

        queryValueNoRecord(actualKeyName, result);

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

        std::string actualKeyName = keyName;
        actualKeyName += "\\";
        actualKeyName += valueName;
        std::string value;
        if (SUCCESS == queryValueNoRecord(actualKeyName, value))
            result = atoi(value.c_str());

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
    std::string actualKeyName = keyName;
    actualKeyName += "\\";
    actualKeyName += valueName;

    setValue(actualKeyName, value);
}

void SysRegistry::setIntegerValue(const std::string& keyName, const std::string& valueName, int value)
{
    std::string actualKeyName = keyName;
    actualKeyName += "\\";
    actualKeyName += valueName;

    std::string valueStr = std::to_string(value);
    setValue(actualKeyName, valueStr);
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
