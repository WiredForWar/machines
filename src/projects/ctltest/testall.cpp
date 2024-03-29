#include <string>
// #include <function.hpp>

#define _INSTANTIATE_TEMPLATE_CLASSES

#include <algorithm>

#include "ctl/fixedvec.hpp"
#include "ctl/queue.hpp"
#include "ctl/list.hpp"
#include "ctl/deque.hpp"
#include "ctl/vector.hpp"
#include "ctl/nbvector.hpp"
#include "ctl/pvector.hpp"
#include "ctl/set.hpp"
#include "ctl/map.hpp"
#include "ctl/mmvector.hpp"

#include "ctl/testh/dummy.hpp"

#include "mathex/double.hpp"

void testDeque();
void testQueue();
void testList();
void testMap();
void testMultiMap();
void testSet();
void testMultiSet();
void testFixedVector();
void testVector();
void testVectorP();
void testMinMemoryVector();
void testSort();

using namespace std;

int main()
{
    testDeque();
    testQueue();
    testMap();
    testMultiMap();
    testSet();
    testMultiSet();
    testFixedVector();
    testVector();
    // testMinMemoryVector();
    // testSort();
    testList();

    //    testVectorP();

    return 0;
}

void testDeque()
{
    ctl_deque<int> d;

    d.push_back(1);
    d.push_front(2);

    cout << d.front() << endl;
    d.pop_front();

    cout << d.back() << endl;
    d.pop_back();
}

void testQueue()
{
    // ctl_queue< ctl_deque< int > >  q;
    ctl_queue<int> q;

    q.push(2);
    q.push(3);
    q.push(1);
    q.push(-50);

    cout << q.front() << endl;
    q.pop();
    cout << q.front() << endl;
    q.pop();
    cout << q.front() << endl;
    q.pop();
    cout << q.front() << endl;
    q.pop();
}

void testMap()
{
    ctl_map<int, string, less<int>> intToString;

    //     intToString[ 0 ] = "zero";
    //     intToString[ 2 ] = "two";
    //     intToString[ 1 ] = "one";
    intToString.insert(ctl_map<int, string, less<int>>::value_type(3, "three"));
    intToString.insert(4, "four");
    intToString.insert(4, "four again");

    intToString[4] = "four yet again";

    ctl_map<int, string, less<int>>::iterator i;

    cout << endl;

    for (i = intToString.begin(); i != intToString.end(); ++i)
        cout << (*i).first << "  " << (*i).second << endl;

    cout << endl;

    intToString.erase(intToString.begin());

    for (i = intToString.begin(); i != intToString.end(); ++i)
        cout << (*i).first << "  " << (*i).second << endl;

    cout << endl;
}

void testMultiMap()
{
    ctl_multimap<int, string, less<int>> intToString;

    //     intToString[ 0 ] = "zero";
    //     intToString[ 2 ] = "two";
    //     intToString[ 1 ] = "one";
    intToString.insert(ctl_map<int, string, less<int>>::value_type(3, "three"));
    intToString.insert(ctl_map<int, string, less<int>>::value_type(4, "four"));
    intToString.insert(ctl_map<int, string, less<int>>::value_type(4, "four again"));
    intToString.insert(ctl_map<int, string, less<int>>::value_type(4, "four yet again"));

    //    intToString[ 4 ] = "four yet again";

    ctl_map<int, string, less<int>>::iterator i;

    for (i = intToString.begin(); i != intToString.end(); ++i)
        cout << (*i).first << "  " << (*i).second << endl;

    cout << endl;

    intToString.erase(intToString.begin());

    for (i = intToString.begin(); i != intToString.end(); ++i)
        cout << (*i).first << "  " << (*i).second << endl;

    cout << endl;
}

void testSet()
{
    ctl_set<int, less<int>> intSet;

    intSet.insert(4);
    intSet.insert(1);
    intSet.insert(-50);
    intSet.insert(3063764);
    intSet.insert(2);
    intSet.insert(4);
    intSet.insert(4);
    intSet.insert(4);

    ctl_set<int, less<int>>::iterator i;

    for (i = intSet.begin(); i != intSet.end(); ++i)
        cout << (*i) << endl;

    cout << endl;
}

void testMultiSet()
{
    ctl_multiset<int, less<int>> intSet;

    intSet.insert(4);
    intSet.insert(1);
    intSet.insert(-50);
    intSet.insert(3063764);
    intSet.insert(2);
    intSet.insert(4);
    intSet.insert(4);
    intSet.insert(4);

    ctl_multiset<int, less<int>>::iterator i;

    for (i = intSet.begin(); i != intSet.end(); ++i)
        cout << (*i) << endl;
}

void testFixedVector()
{

    ctl_fixed_vector<int> v1(10);

    v1[0] = 10;
    v1[1] = 20;
}

using std::cout;
using std::endl;
void testVector()
{
    ctl_vector<int>* v1 = new ctl_vector<int>();

    cout << v1->size() << endl;
    cout << v1->end() - v1->begin() << endl;

    v1->push_back(10);

    cout << v1->size() << endl;
    cout << v1->end() - v1->begin() << endl;

    v1->push_back(20);

    cout << v1->size() << endl;
    cout << v1->end() - v1->begin() << endl;

    typedef int RenMeshInstance;

    struct MeshData
    {
        RenMeshInstance* mesh;
        MexDouble distance;
    };

    typedef ctl_vector<MeshData> Meshes;

    Meshes m1;
    m1.push_back(MeshData());
    WHERE_STREAM("meshes " << m1.size() << endl);
    WHERE_STREAM("end - begin " << m1.end() - m1.begin() << endl);

    ASSERT(m1.size() == m1.end() - m1.begin(), "");

    //     for( ctl_vector< int >::reverse_iterator i = v1.rbegin(); i != v1.rend(); ++i )
    //         cout << *i << endl;
    //
    //     const ctl_vector< int > v2( v1 );
    //
    //     for( ctl_vector< int >::const_reverse_iterator i = v2.rbegin(); i != v2.rend(); ++i )
    //         cout << *i << endl;
}
//
// void    testMinMemoryVector()
// {
//     ctl_min_memory_vector< int >    v1;
//
//     v1.push_back( 10 );
//     v1.push_back( 20 );
// }
//
// void    testSort()
// {
//     ctl_vector< int >   vec;
//
//     vec.push_back( 10 );
//     vec.push_back( -10 );
//     vec.push_back( 5 );
//     vec.push_back( 1000 );
//     vec.push_back( 15 );
//
//     sort( vec.begin(), vec.end(), less< int >() );
// }
//
void testList()
{
    MEMORY_STREAM("a" << endl);
    ctl_list<int> l1;
    MEMORY_STREAM("b" << endl);
    ctl_list<int> l2;
    MEMORY_STREAM("c" << endl);

    l1.push_back(1);
    l2.push_back(2);
}

void testvectorP()
{
    ctl_pvector<int> v1;

    const int N_ENTRIES = 100;
    int iArray[N_ENTRIES];

    for (size_t i = 0; i < N_ENTRIES; ++i)
    {
        iArray[i] = i;
        v1.push_back(&iArray[i]);
    }

    cout << "testvectorP" << endl;

    cout << *v1[0] << endl;
    cout << *v1[1] << endl;
    cout << *v1.front() << endl;
    cout << *v1.back() << endl;

    for (size_t i = 0; i < N_ENTRIES; ++i)
    {
        ASSERT(i == *v1[i], "");
        int* pJ = v1[i];

        ctl_pvector<int>::iterator k = v1.begin();

        int* pK = k[i];

        int* pI = ((v1.begin())[i]);
        ASSERT(i == *pI, "");
        ASSERT(i == *((v1.begin())[i]), "");
    }

    for (ctl_pvector<int>::iterator i = v1.begin(); i != v1.end(); ++i)
    {
        cout << (*(*i)) << " ";

        ctl_pvector<int>::iterator j = i;
        ctl_pvector<int>::iterator k = j++;
        ctl_pvector<int>::iterator l = j;
        l--;

        ASSERT(l == i, "");

        if (j != v1.end())
        {
            ASSERT(*i == *k, "");
            ASSERT(*i + 1 == *j, "");
        }
    }

    cout << endl;

    for (ctl_pvector<int>::reverse_iterator i = v1.rbegin(); i != v1.rend(); ++i)
    {
        cout << (*(*i)) << " ";

        ctl_pvector<int>::reverse_iterator j = i;
        ctl_pvector<int>::reverse_iterator k = j++;
        ctl_pvector<int>::reverse_iterator l = j;
        l--;

        ASSERT(l == i, "");

        if (j != v1.rend())
        {
            ASSERT(*i == *k, "");
            ASSERT_INFO(*(*i));
            ASSERT_INFO(*(*j));
            ASSERT_INFO(*(*(i + 1)));
            ASSERT(*(*(i + 1)) == *(*j), "");
        }
    }

    cout << endl;

    ctl_nb_vector<double*> v2;
    ctl_nb_vector<double*> v3;
    double d1;
    v2.push_back(&d1);

    //     if( v2 == v3 )
    //         cout << "Vectors are equal" << endl;
    /*
    ctl_vector< ctl_nb_helper< sizeof( int ) > >   vi1;
    ctl_vector< ctl_nb_helper< sizeof( int ) > >   vi2;

//    vi1.push_back( 7 );

    if( vi1 == vi2 )
        cout << "Vectors are equal" << endl;
    else
        cout << "Vectors are not equal" << endl;*/
}

void test2()
{
    ctl_nb_vector<float*> fv;
    float f;

    fv.push_back(&f);
}
