
#include <iostream>
#include <sstream>
#include <fc/io/raw.hpp>

namespace graphene { namespace chain {
struct C
{
   uint32_t x = 0;
};

template< typename T >
struct E
{
   T e;
};

struct A
{
   E<C> y;
};

template< typename Stream, typename T >
void operator<<( Stream& s, const graphene::chain::E<T>& e )
{
   std::cout << "here!\n";
   return;
}

} }

FC_REFLECT( graphene::chain::C, (x) )
FC_REFLECT( graphene::chain::A, (y) )

namespace fc { namespace raw {

} }

int main( int argc, char** argv, char** envp )
{
   std::stringstream ss;
   graphene::chain::A obj;

   fc::raw::pack( ss, obj );
   return 0;
}

