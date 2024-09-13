#pragma once

#include "base/persist.hpp"

#include <string>

void writeAllocatedStringFromPointer(PerOstream& ostr, const std::string* pOb);
void readAllocatedStringFromPointer(PerIstream& istr, std::string* pOb);

void perWrite(PerOstream& ostr, const std::string& ob);
void perRead(PerIstream& istr, std::string& ob);

PerOstream& operator<<(PerOstream&, const std::string&);
PerIstream& operator>>(PerIstream&, std::string&);
