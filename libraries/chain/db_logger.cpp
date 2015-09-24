/*
 * Copyright (c) 2015, Cryptonomex, Inc.
 * All rights reserved.
 *
 * This source code is provided for evaluation in private test networks only, until September 8, 2015. After this date, this license expires and
 * the code may not be used, modified or distributed for any purpose. Redistribution and use in source and binary forms, with or without modification,
 * are permitted until September 8, 2015, provided that the following conditions are met:
 *
 * 1. The code and/or derivative works are used only for private test networks consisting of no more than 10 P2P nodes.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <graphene/chain/db_logger.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/vesting_balance_object.hpp>

#include <fstream>
#include <iostream>

namespace graphene { namespace chain { namespace detail {

dblog_hash_type compute_vbo_hash( const database& db )
{
   const auto& idx = db.get_index_type<vesting_balance_index>().indices().get<by_id>();
   auto itr = idx.begin();
   uint64_t total = 0;
   while( itr != idx.end() )
   {
      const vesting_balance_object& vbo = (*itr);
      vector< char > data = fc::raw::pack( vbo );
      uint64_t h = fc::city_hash64( &(data[0]), data.size() );
      total += h;
   }
   return total;
}

database_logger::database_logger( const std::string& filename )
{
   logfile.open(filename, std::ios::out | std::ios::app | std::ios::binary);
   return;
}

database_logger::~database_logger()
{
   logfile.close();
   return;
}

template< typename ItemType, typename Stream, typename F >
void write_item( Stream& stream, F func )
{
   log_item generic_item;
   generic_item.set_which( log_item::tag< ItemType >::value );
   ItemType& item = generic_item.get< ItemType >();
   func( item );
   fc::raw::pack( stream, item );
   stream.flush();
}

void database_logger::log_push_block( const signed_block& new_block, uint32_t skip )
{
   write_item< push_block_item >( logfile, [&]( push_block_item& item )
   {
      item.new_block = new_block;
      item.skip = skip;
   } );
}

void database_logger::log_push_transaction( const signed_transaction& new_tx, uint32_t skip )
{
   write_item< push_transaction_item >( logfile, [&]( push_transaction_item& item )
   {
      item.new_tx = new_tx;
      item.skip = skip;
   } );
}
      
void database_logger::log_generate_block( fc::time_point_sec when, witness_id_type witness_id, public_key_type witness_key, uint32_t skip )
{
   write_item< generate_block_item >( logfile, [&]( generate_block_item& item )
   {
      item.when = when;
      item.witness_id = witness_id;
      item.witness_key = witness_key;
      item.skip = skip;
   } );
}

void database_logger::log_vbo_hash( const database& db )
{
   write_item< vbo_hash_item >( logfile, [&]( vbo_hash_item& item )
   {
      item.hash_value = compute_vbo_hash( db );
   } );
   return;
}

struct replay_log_visitor
{
   typedef void result_type;

   replay_log_visitor( database& _db, const vector< private_key_type >& block_signing_keys )
      : db(_db)
   {
      signing_key_map.reserve( block_signing_keys.size() );
      for( private_key_type priv : block_signing_keys )
         signing_key_map[priv.get_public_key()] = priv;
   }

   void operator()( const push_block_item& item )
   {
      db.push_block( item.new_block, item.skip );
   }
   
   void operator()( const push_transaction_item& item )
   {
      db.push_transaction( item.new_tx, item.skip );
   }

   void operator()( const generate_block_item& item )
   {
      auto it = signing_key_map.find( item.witness_key );
      FC_ASSERT( it == signing_key_map.end() );

      db.generate_block( item.when, item.witness_id, it->second, item.skip );
   }

   void operator()( const vbo_hash_item& item )
   {
      // TODO:  Process item
   }

   database& db;
   flat_map< public_key_type, private_key_type > signing_key_map;
};

void replay_log( std::string filename, database& db, const vector< private_key_type >& block_signing_keys )
{
   std::ifstream infile;
   infile.open(filename, std::ios::in | std::ios::binary);
   replay_log_visitor vtor( db, block_signing_keys );
   while( !infile.eof() )
   {
      log_item item;
      fc::raw::unpack( infile, item );
      item.visit( vtor );
   }
}

} } } // graphene::chain::detail
