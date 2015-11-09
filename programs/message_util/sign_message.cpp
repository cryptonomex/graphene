/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Any modified source or binaries are used only with the BitShares network.
 *
 * 2. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 3. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>

#include <fc/crypto/hex.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>
#include <fc/io/stdio.hpp>
#include <fc/smart_ref_impl.hpp>

#include <graphene/app/api.hpp>
#include <graphene/chain/protocol/address.hpp>
#include <graphene/chain/protocol/protocol.hpp>
#include <graphene/egenesis/egenesis.hpp>
#include <graphene/utilities/key_conversion.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#ifndef WIN32
#include <csignal>
#endif

using namespace graphene::app;
using namespace graphene::chain;
using namespace graphene::utilities;
using namespace std;
namespace bpo = boost::program_options;

#define BUFSIZE 131072

std::string from_hex( const std::string& hex )
{
   std::string result;
   FC_ASSERT( (hex.size() & 1) == 0 );
   result.reserve( hex.size() >> 1 );
   for( size_t i=0; i<hex.size(); i+=2 )
   {
      uint8_t c;
      switch( hex[i] )
      {
         case '0': c = 0x00; break;
         case '1': c = 0x10; break;
         case '2': c = 0x20; break;
         case '3': c = 0x30; break;
         case '4': c = 0x40; break;
         case '5': c = 0x50; break;
         case '6': c = 0x60; break;
         case '7': c = 0x70; break;
         case '8': c = 0x80; break;
         case '9': c = 0x90; break;
         case 'a': c = 0xa0; break;
         case 'b': c = 0xb0; break;
         case 'c': c = 0xc0; break;
         case 'd': c = 0xd0; break;
         case 'e': c = 0xe0; break;
         case 'f': c = 0xf0; break;
         case 'A': c = 0xA0; break;
         case 'B': c = 0xB0; break;
         case 'C': c = 0xC0; break;
         case 'D': c = 0xD0; break;
         case 'E': c = 0xE0; break;
         case 'F': c = 0xF0; break;
         default:
            FC_ASSERT( false );   // illegal hex char
      }

      switch( hex[i+1] )
      {
         case '0': c += 0x00; break;
         case '1': c += 0x01; break;
         case '2': c += 0x02; break;
         case '3': c += 0x03; break;
         case '4': c += 0x04; break;
         case '5': c += 0x05; break;
         case '6': c += 0x06; break;
         case '7': c += 0x07; break;
         case '8': c += 0x08; break;
         case '9': c += 0x09; break;
         case 'a': c += 0x0a; break;
         case 'b': c += 0x0b; break;
         case 'c': c += 0x0c; break;
         case 'd': c += 0x0d; break;
         case 'e': c += 0x0e; break;
         case 'f': c += 0x0f; break;
         case 'A': c += 0x0A; break;
         case 'B': c += 0x0B; break;
         case 'C': c += 0x0C; break;
         case 'D': c += 0x0D; break;
         case 'E': c += 0x0E; break;
         case 'F': c += 0x0F; break;
         default:
            FC_ASSERT( false );   // illegal hex char
      }
      result += char(c);
   }
   return result;
}

int verify_single_sig( const digest_type& sig_digest, const std::string& verify_str )
{
   size_t split_pos = verify_str.find(':');
   std::string verify_sig_hex, verify_pubkey_str;
   if( split_pos == std::string::npos )
   {
      verify_sig_hex = verify_str;
      verify_pubkey_str = "";
   }
   else
   {
      verify_sig_hex = verify_str.substr( 0, split_pos );
      verify_pubkey_str = verify_str.substr( split_pos+1 );
   }

   std::string verify_sig_str;
   try
   {
      verify_sig_str = from_hex( verify_sig_hex );
   }
   catch( const fc::exception& e )
   {
      std::cout << "PARSE ERROR" << std::endl;
      return 1;
   }

   fc::ecc::compact_signature sig;

   if( verify_sig_str.size() != sig.size() )
   {
      std::cout << "PARSE ERROR" << std::endl;
      return 1;
   }

   for( size_t i=0; i<sig.size(); i++ )
      sig.at(i) = verify_sig_str[i];

   public_key_type pubkey_from_sig = fc::ecc::public_key( sig, sig_digest );

   std::cout << verify_str << " -> " << std::string( pubkey_from_sig );

   if( verify_pubkey_str != "" )
   {
      public_key_type pubkey_from_arg( verify_pubkey_str );
      if( pubkey_from_sig == pubkey_from_arg )
      {
         std::cout << " OK" << std::endl;
         return 0;
      }
      else
      {
         std::cout << " FAIL" << std::endl;
         return 1;
      }
   }

   std::cout << std::endl;
   return 0;
}

int main( int argc, char** argv )
{
   try
   {
      bpo::options_description cli_options("Graphene empty blocks");

      std::cerr << "WARNING:  This utility is experimental\n"
         "Message signature algorithm is not guaranteed to stay identical in future releases\n";

      cli_options.add_options()
            ("help,h", "Print this help message and exit.")
            ("chain-id,c", bpo::value<std::string>()->default_value("0000000000000000000000000000000000000000"), "The chain ID to sign with")
            ("role,r", bpo::value<std::string>(), "The message role to sign with")
            ("sign,s", bpo::value<std::string>(), "The WIF private key to sign with")
            ("verify,v", bpo::value<std::string>(), "Signature or sig:pubkey pair to verify")
            ("inhash,i", bpo::value<std::string>(), "The input to sign (hash of input file / tx)")
            ("hash", bpo::value<std::string>(), "The hash algorithm to use (sha256 | none)")
            ;

      // the effective chain ID is H(chain_id + role)
      // empty string -> message is potentially a transaction on the given chain

      bpo::variables_map options;
      try
      {
         boost::program_options::store( boost::program_options::parse_command_line(argc, argv, cli_options), options );
      }
      catch (const boost::program_options::error& e)
      {
         std::cerr << "empty_blocks:  error parsing command line: " << e.what() << "\n";
         return 1;
      }

      if( options.count("help") )
      {
         std::cout << cli_options << "\n";
         return 1;
      }

      if( !options.count( "chain-id" ) )
      {
         std::cerr << "--chain-id option is required\n";
         return 1;
      }

      if( !options.count( "role" ) )
      {
         std::cerr << "--role option is required\n";
         return 1;
      }

      /*
      // old API which hashed input file
      if( !options.count( "infile" ) )
      {
         std::cerr << "--infile option is required\n";
         return 1;
      }

      // TODO:  allow disable hashing, or different hash algorithms
      fc::path infilename = options["infile"].as<boost::filesystem::path>();
      boost::filesystem::ifstream f( infilename, std::ios::in | std::ios::binary );
      char* data = new char[ BUFSIZE ];
      if( data == nullptr )
      {
         std::cerr << "couldn't allocate buffer";
         exit(1);
      }
      fc::sha256::encoder infile_enc;
      while( !f.eof() )
      {
         f.read( data, BUFSIZE );
         infile_enc.write( data, f.gcount() );
      }
      fc::sha256 infile_hash = infile_enc.result();
      delete[] data;
      */

      if( !options.count( "inhash" ) )
      {
         std::cerr << "--inhash option is required\n";
         return 1;
      }

      std::string inhash = from_hex( options["inhash"].as<std::string>() );
      chain_id_type chain_id = chain_id_type( options["chain-id"].as<std::string>() );

      digest_type::encoder enc;
      if( (!options.count("role")) || (options["role"].as<string>() == "") )
      {
         fc::raw::pack( enc, chain_id );
         fc::raw::pack( enc, inhash );
      }
      else
      {
         digest_type::encoder enc_echain_id;
         fc::raw::pack( enc, chain_id );
         fc::raw::pack( enc, options["role"].as<string>() );
         fc::raw::pack( enc, enc_echain_id.result() );
         fc::raw::pack( enc, inhash );
      }
      digest_type sig_digest = enc.result();

      if( options.count("sign") )
      {
         fc::optional< fc::ecc::private_key > sign_key = graphene::utilities::wif_to_key( options["sign"].as<std::string>() );
         fc::ecc::compact_signature sig = sign_key->sign_compact( sig_digest );
         std::string hex_sig = fc::to_hex( (const char*) sig.data, sig.size() );
         std::cout << hex_sig << std::endl;
      }

      if( options.count("verify") )
      {
         return verify_single_sig( sig_digest, options["verify"].as<std::string>() );
      }
      return 0;
   }
   catch ( const fc::exception& e )
   {
      std::cout << e.to_detail_string() << "\n";
      return 1;
   }
   return 0;
}
