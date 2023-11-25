#pragma once

#include <cstddef>

// void * operator new( size_t, void * ) noexcept;

// the following template function is replaced by the following two
// functions due to the fact that the Borland compiler doesn't change
// ptrdiff_t to type long when compile with -ml or -mh:
// template < class T > T * allocate( ptrdiff_t size, T * );

template <class T> T* allocate(int size, T*);

template <class T> T* allocate(long size, T*);

//////////////////////////////////////////////////////////////////////

template <class T> void deallocate(T* buffer);

//////////////////////////////////////////////////////////////////////

template <class ForwardIter> void destroy(ForwardIter first, ForwardIter last);

template <class T> void destroy(T* pointer);

void destroy(char*);
void destroy(unsigned char*);
void destroy(short*);
void destroy(unsigned short*);
void destroy(int*);
void destroy(unsigned int*);
void destroy(long*);
void destroy(unsigned long*);
void destroy(float*);
void destroy(double*);

void destroy(char**);
void destroy(unsigned char**);
void destroy(short**);
void destroy(unsigned short**);
void destroy(int**);
void destroy(unsigned int**);
void destroy(long**);
void destroy(unsigned long**);
void destroy(float**);
void destroy(double**);

void destroy(char*, char*);
void destroy(unsigned char*, unsigned char*);
void destroy(short*, short*);
void destroy(unsigned short*, unsigned short*);
void destroy(int*, int*);
void destroy(unsigned int*, unsigned int*);
void destroy(long*, long*);
void destroy(unsigned long*, unsigned long*);
void destroy(float*, float*);
void destroy(double*, double*);

void destroy(char**, char**);
void destroy(unsigned char**, unsigned char**);
void destroy(short**, short**);
void destroy(unsigned short**, unsigned short**);
void destroy(int**, int**);
void destroy(unsigned int**, unsigned int**);
void destroy(long**, long**);
void destroy(unsigned long**, unsigned long**);
void destroy(float**, float**);
void destroy(double**, double**);
