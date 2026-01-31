#include "machgui/Network/NetworkVariables.hpp"

#include "system/Variable_p.hpp"

namespace Config
{

Variable<std::string> netSelectedProtocol("Misc/Chosen Protocol", {});
Variable<std::string> netPlayerName("Misc/Players Name", {});

} // namespace Config
