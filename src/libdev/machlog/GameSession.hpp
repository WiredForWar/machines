#pragma once

// The lifetime of one game being played.
//
// A session begins when the simulation starts running a loaded game and ends when that
// game is over, whether it was won, lost, quit or dropped. Anything that has to be set up
// per game and torn down with it belongs here, so that there is one place to do it from
// and one answer to whether a game is running.
class MachLogGameSession
{
public:
    //  Singleton class
    static MachLogGameSession& instance();
    ~MachLogGameSession();

    void begin();
    void end();

    bool isActive() const;

private:
    MachLogGameSession();

    MachLogGameSession(const MachLogGameSession&);
    MachLogGameSession& operator=(const MachLogGameSession&);
    bool operator==(const MachLogGameSession&);

    bool isActive_{};
};
