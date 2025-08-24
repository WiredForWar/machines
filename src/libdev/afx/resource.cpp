/*
 * R E S O U R C E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "afx/resource.hpp"

#include "utility/rapidxml_utils.hpp"

bool AfxResourceLib::addStringsFromFile(const SysPathName& path)
{
    PRE(path.existsAsFile());

    bool parsed = true;
    rapidxml::file<> xmlFile(path.c_str()); // Default template is char
    rapidxml::xml_document<> doc;
    try
    {
        doc.parse<0>(xmlFile.data());
    }
    catch (const rapidxml::parse_error& e)
    {
        parsed = false;
        std::cerr << e.what() << " here: " << e.where<char>() << std::endl;
    }

    if (parsed)
    {
        rapidxml::xml_node<>* node = doc.first_node();

        for (rapidxml::xml_node<>* a = node->first_node(); a; a = a->next_sibling())
        {
            resourceStrings_[atoi(a->first_attribute()->value())] = a->value();
        }
    }

    return parsed;
}

void AfxResourceLib::clear()
{
    resourceStrings_.clear();
}

std::string AfxResourceLib::getString(const uint id) const
{
    if (resourceStrings_.count(id) > 0)
        return resourceStrings_.at(id);
    else
        return "";
}
