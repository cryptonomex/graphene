
#include <graphene/chain/protocol/address.hpp>
#include <graphene/chain/protocol/types.hpp>

#include <fc/crypto/elliptic.hpp>
#include <fc/io/json.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

#define NUM_KEYS 100

int main( int argc, char** argv, char** envp )
{
   std::vector< graphene::chain::public_key_type > pubkeys;
   std::vector< graphene::chain::address > addresses;

   for( int i=0; i<100; i++ )
   {
      std::string text = fc::to_string(i);
      fc::ecc::private_key k = fc::ecc::private_key::regenerate( fc::sha256::hash( text ) );
      pubkeys.emplace_back( k.get_public_key() );
      addresses.emplace_back( pubkeys.back() );
   }

   std::sort( pubkeys.begin(), pubkeys.end() );
   std::sort( addresses.begin(), addresses.end() );
   std::cout << "list of sorted keys:" << std::endl << std::endl;
   for( const auto& pub : pubkeys )
      std::cout << std::string( pub ) << std::endl;
   std::cout << std::endl << "list of sorted addresses:" << std::endl << std::endl;
   for( const auto& addr : addresses )
      std::cout << std::string( addr ) << std::endl;
   std::cout << std::endl << "list of sorted keys with corresponding address / ripemd160:" << std::endl << std::endl;
   for( const auto& pub : pubkeys )
   {
      fc::mutable_variant_object mvo;
      graphene::chain::address addr = graphene::chain::address( pub );
      mvo( "public_key", std::string( pub ) )
         ( "address", std::string( addr ) )
         ( "hex_address", addr.addr.str() )
         ;
      std::cout << fc::json::to_string( mvo ) << std::endl;
   }
   return 0;
}
