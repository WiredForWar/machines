/*
 * R E G I S T R Y . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "system/registry.hpp"

#include "recorder/recorder.hpp"
#include "recorder/private/recpriv.hpp"

#include "utility/rapidxml_utils.hpp"
#include "utility/rapidxml_ext.hpp"

#include "system/pathname.hpp"

#include <fstream>
#include <string>

class SysRegistryImpl
{
public:
    // Default template is char
    using XmlFileData = rapidxml::file<>;

    SysRegistryImpl(std::string fileName = "config.xml");

    ~SysRegistryImpl();

    void init();
    bool readFromFile();

    void store();

    friend class SysRegistry;
    std::string currentStubKey_;

    rapidxml::xml_document<> doc_;
    std::unique_ptr<XmlFileData> xmlFile_{};
    std::string regFile_;
};

SysRegistryImpl::SysRegistryImpl(std::string fileName)
    : regFile_(fileName)
{
    init();
};

SysRegistryImpl::~SysRegistryImpl()
{
    doc_.clear();
};

void SysRegistryImpl::init()
{
    bool create = true;

    if (SysPathName::existsAsFile(regFile_))
    {
        if (readFromFile())
        {
            create = false;
        }
    }

    if (create)
    {
        doc_.clear();
        char* node_name = doc_.allocate_string("keys");
        rapidxml::xml_node<>* child = doc_.allocate_node(rapidxml::node_element, node_name);
        doc_.append_node(child);
    }
}

bool SysRegistryImpl::readFromFile()
{
    bool result = true;

    try
    {
        xmlFile_ = std::make_unique<XmlFileData>(regFile_.c_str());
        doc_.parse<0>(xmlFile_->data());
    }
    catch (const rapidxml::parse_error& e)
    {
        std::cerr << e.what() << " here: " << e.where<char>() << std::endl;
        result = false;
    }
    catch (...)
    {
        result = false;
    }

    std::cout << "Failed to parse config file, new empty one is created." << std::endl;

    return result;
}

void SysRegistryImpl::store()
{
    // Save to file
    std::ofstream file_stored(regFile_.c_str());
    file_stored << doc_;
    file_stored.close();
}

#define CB_SysRegistry_DEPIMPL()                                                                                       \
    PRE(pImpl_)                                                                                                        \
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
}

SysRegistry::~SysRegistry() = default;

void SysRegistry::reload()
{
    pImpl_ = std::make_unique<SysRegistryImpl>();
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

/* End REGISTRY.CPP *************************************************/
