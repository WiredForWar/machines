/*
 * C L A R G S . H P P 
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */


/*
    CommandLineArguments

    Parse and store the command line arguments
*/

#ifndef _CLARGS_HPP
#define _CLARGS_HPP

#include "base/base.hpp"

class CommandLineArguments
{
public:
    //  Singleton class
    static CommandLineArguments& instance( void );
    ~CommandLineArguments( void );

    void parseArguments( int argc, char** argv );
    
    size_t  seed( void ) const;
    bool    displayLogInfo( void ) const;
    
    void CLASS_INVARIANT;

    friend ostream& operator <<( ostream& o, const CommandLineArguments& t );

private:
    // Operation deliberately revoked
    CommandLineArguments( const CommandLineArguments& );

    // Operation deliberately revoked
    CommandLineArguments& operator =( const CommandLineArguments& );

    // Operation deliberately revoked
    bool operator ==( const CommandLineArguments& );

    CommandLineArguments( void );
    
    size_t  seed_;
    bool    displayLogInfo_;
};

CommandLineArguments& ARGS( void );

#define LOG_DATA( var )   ARGS().displayLogInfo() ?     \
( cout << __FILE__ << ", " << __LINE__ << "   " << #var " :\t" << (var) << endl ) : \
 cout << ""

#endif

/* End CLARGS.HPP ***************************************************/
