/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <graphene/chain/committee_member_evaluator.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/vote.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>
#include <graphene/chain/hardfork.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {

void_result committee_member_create_evaluator::do_evaluate( const committee_member_create_operation& op )
{ try {
   FC_ASSERT(db().get(op.committee_member_account).is_lifetime_member());
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type committee_member_create_evaluator::do_apply( const committee_member_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::committee);
   });

   const auto& new_del_object = db().create<committee_member_object>( [&]( committee_member_object& obj ){
         obj.committee_member_account   = op.committee_member_account;
         obj.vote_id            = vote_id;
         obj.url                = op.url;
   });
   return new_del_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_evaluator::do_evaluate( const committee_member_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.committee_member).committee_member_account == op.committee_member_account);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_evaluator::do_apply( const committee_member_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.committee_member),
      [&]( committee_member_object& com )
      {
         if( op.new_url.valid() )
            com.url = *op.new_url;
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_global_parameters_evaluator::do_evaluate(const committee_member_update_global_parameters_operation& o)
{ try {
   FC_ASSERT(trx_state->_is_proposed_trx);

   database& d = db();

   // #603 hard fork check
   if( d.head_block_time() <= HARDFORK_603_TIME )
   {
      for( const auto& e : o.new_parameters.extensions )
      {
         FC_ASSERT( e.which() != chain_parameters::parameter_extension::tag<chain_parameters::ext::coin_seconds_as_fees_options>::value,
                    "New parameters contain an extension which requires hardfork #603." );
      }
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_global_parameters_evaluator::do_apply(const committee_member_update_global_parameters_operation& o)
{ try {
   db().modify(db().get_global_properties(), [&o](global_property_object& p) {
      p.pending_parameters = o.new_parameters;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_core_asset_evaluator::do_evaluate(const committee_member_update_core_asset_operation& o)
{ try {
   FC_ASSERT(trx_state->_is_proposed_trx);

   database& d = db();

   // #583 BSIP10 hard fork check
   if( d.head_block_time() <= HARDFORK_583_TIME )
      FC_THROW( "Operation requires hardfork #583" );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_core_asset_evaluator::do_apply(const committee_member_update_core_asset_operation& o)
{ try {
   database& d = db();

   const asset_object& a = d.get_core_asset();

   auto a_copy = a;
   a_copy.options = o.new_options;
   auto new_mode = a_copy.get_transfer_fee_mode();

   a_copy = a;
   a_copy.options.market_fee_percent = o.new_options.market_fee_percent;
   a_copy.options.max_market_fee = o.new_options.max_market_fee;
   a_copy.set_transfer_fee_mode( new_mode );

   d.modify(a, [&](asset_object& ao) {
      ao.options = a_copy.options;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

} } // graphene::chain
