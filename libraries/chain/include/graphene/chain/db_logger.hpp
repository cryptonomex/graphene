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
#pragma once

#include <graphene/chain/protocol/block.hpp>
#include <graphene/chain/protocol/transaction.hpp>
#include <graphene/chain/protocol/types.hpp>

#include <fstream>
#include <ios>

/*
 * This file provides with() functions which modify the database
 * temporarily, then restore it.  These functions are mostly internal
 * implementation detail of the database.
 *
 * Essentially, we want to be able to use "finally" to restore the
 * database regardless of whether an exception is thrown or not, but there
 * is no "finally" in C++.  Instead, C++ requires us to create a struct
 * and put the finally block in a destructor.  Aagh!
 */

namespace graphene { namespace chain {

class database;

namespace detail {

typedef uint64_t dblog_hash_type;

struct push_block_item
{
   signed_block new_block;
   uint32_t skip;
};

struct push_transaction_item
{
   signed_transaction new_tx;
   uint32_t skip;
};

struct generate_block_item
{
   fc::time_point_sec when;
   witness_id_type witness_id;
   public_key_type witness_key;
   uint32_t skip;
};

struct vbo_hash_item
{
   dblog_hash_type hash_value;
};

typedef static_variant<
   push_block_item,
   push_transaction_item,
   generate_block_item,
   vbo_hash_item
   > log_item;

dblog_hash_type compute_vbo_hash( const database& db );

class database_logger
{
   public:
      database_logger( const string& filename );
      virtual ~database_logger();

      void log_push_block( const signed_block& new_block, uint32_t skip );
      void log_push_transaction( const signed_transaction& new_tx, uint32_t skip );
      void log_generate_block( fc::time_point_sec when, witness_id_type witness_id, public_key_type witness_key, uint32_t skip );
      void log_vbo_hash( const database& db );

      std::ofstream logfile;
};

void replay_log( std::string filename, database& db, const vector< private_key_type >& block_signing_keys );

} } } // graphene::chain::detail

FC_REFLECT( graphene::chain::detail::push_block_item,
    (new_block)
    (skip)
   )

FC_REFLECT( graphene::chain::detail::push_transaction_item,
    (new_tx)
    (skip)
   )

FC_REFLECT( graphene::chain::detail::generate_block_item,
    (when)
    (witness_id)
    (witness_key)
    (skip)
   )

FC_REFLECT( graphene::chain::detail::vbo_hash_item,
    (hash_value)
   )

FC_REFLECT_TYPENAME( graphene::chain::detail::log_item )
