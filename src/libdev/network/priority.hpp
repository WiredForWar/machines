#ifndef _NETPRIORITY_HPP
#define _NETPRIORITY_HPP

class NetPriority
// memberwise cannonical
{
public:

	typedef unsigned char 		Urgency;
	typedef unsigned char		RedeliveryAttempts;

	NetPriority( Urgency = 0, RedeliveryAttempts = 0 );

	Urgency				urgency() const;
	RedeliveryAttempts	redeliveryAttempts() const;

private:
	Urgency					urgency_;
	RedeliveryAttempts		redeliveryAttempts_;
};

#endif //_NETPRIORITY_HPP