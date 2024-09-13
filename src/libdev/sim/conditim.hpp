
#ifndef _SIM_CONDITIONS_MANAGER
#define _SIM_CONDITIONS_MANAGER

#include "base/base.hpp"
#include "stdlib/string.hpp"

#include <memory>
#include <vector>

class SimManager;
class SimCondition;
class SimAction;
class SimConditionsManagerImpl;

class SimConditionsManager
{
public:
    friend class SimManager;
    friend class Simcondition;

    ~SimConditionsManager();
    using Actions = std::vector<std::unique_ptr<SimAction>>;
    const Actions& actions() const;

    // Ensure the action conditions are checked on next update call, as opposed to the
    // cached earliest required time. Called automatically when an action is enabled.
    void forceCheckOnNextUpdate();

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const SimConditionsManager& t);

private:
    SimConditionsManager();

    // add action.
    void addAction(std::unique_ptr<SimAction> pNewAction);

    // enable action.
    void enableAction(const std::string& keyName);

    // remove condition.
    void remove(const std::string& keyName);

    void update();

    void loadGame();
    void unloadGame();

    SimConditionsManagerImpl* pImpl_;
};

#endif
