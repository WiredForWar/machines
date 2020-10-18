#include "ctl/pvector.hpp"
#include "ctl/vector.hpp"

#include "device/timer.hpp"

void    testvectorP2( void );

void	speedTest( void );

int main( void )
{
	testvectorP2();

	speedTest();

	return 0;
}

void    testvectorP2( void )
{
    ctl_vector< int* > v1;
	int	i1;
	v1.push_back( &i1 );

    ctl_vector< double* >  v2;
    double  d1;
    v2.push_back( &d1 );

    ctl_vector< float* >  v3;
    float  f1;
    v3.push_back( &f1 );

    ctl_vector< char* >  v4;
    char  c1;
    v4.push_back( &c1 );

//     ctl_pvector< int > v1;
// 	int	i1;
// 	v1.push_back( &i1 );
// 
//     ctl_pvector< double* >  v2;
//     double  d1;
//     v2.push_back( &d1 );
// 
//     ctl_pvector< float* >  v3;
//     float	f1;
//     v3.push_back( &f1 );
// 
//     ctl_pvector< char* >  v4;
//     char	c1;
//     v4.push_back( &c1 );
}


const size_t	N_ITERATIONS = 1000000;

void speedTest( void )
{
	ctl_vector< int* >	v1;
	ctl_pvector< int >	v2( 5 );

	int*	pI;

	v1.push_back( pI );
	v2.push_back( pI );

	cout << endl;
	v1.push_back( pI );
	cout << endl;
	v2.push_back( pI );
	cout << endl;

	DevTimer	t1;
	t1.time( 0.0 );

	for( size_t i = 0; i < N_ITERATIONS; ++i )
	{
//		v1.push_back( pI );
// 		v1.push_back( pI );
// 		v1.pop_back();
// 
// 		for( ctl_vector< int* >::iterator j = v1.begin(); j != v1.end(); ++j )
// 		{
// 			int*	pI2 = *j;
// 		}
	}

	cout << "Time for vector " << t1.time() << endl;

	t1.time( 0.0 );

	for( size_t i = 0; i < N_ITERATIONS; ++i )
	{
//		v2.push_back( pI );
// 		v2.push_back( pI );
// 		v2.pop_back();
// 
// 		for( ctl_pvector< int >::iterator j = v2.begin(); j != v2.end(); ++j )
// 		{
// 			int*	pI2 = *j;
// 		}
	}

	cout << "Time for vector_p " << t1.time() << endl;
}

void t1( void )
{
	ctl_vector< int* >	v1;

// 	int*	pI;
// 
// 	v1.push_back( pI );
}


void t2( void )
{
	ctl_pvector< int >	v1;

// 	int*	pI;
// 
// 	v1.push_back( pI );
}

class TestClass
{
public:
    ctl_vector< void* > v1_;
    void push_back( int* pI ) { v1_.push_back( pI ); }
};

void t3( void )
{
    TestClass   v1;

// 	int*	pI;
// 
// 	v1.push_back( pI );
}

template< class T >
class TestClass2 : private vector< void* >
{
public:
//    vector< void* > v1_;
    TestClass2() {}
    void push_back( int* pI ) { vector< void* >::push_back( pI ); }
};

void t4( void )
{
    TestClass2< int >   v1;

//    cout << "a" << endl;
	int*	pI;

	v1.push_back( pI );
}


void t5( void )
{
    vector< int* >   v1;

//    cout << "a" << endl;
	int*	pI;

	v1.push_back( pI );
}


