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

#include <fc/smart_ref_impl.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/internal_exceptions.hpp>

#include <algorithm>

namespace graphene { namespace chain {

void verify_authority_accounts( const database& db, const authority& a )
{
   const auto& chain_params = db.get_global_properties().parameters;
   GRAPHENE_ASSERT(
      a.num_auths() <= chain_params.maximum_authority_membership,
      internal_verify_auth_max_auth_exceeded,
      "Maximum authority membership exceeded" );
   for( const auto& acnt : a.account_auths )
   {
      GRAPHENE_ASSERT( db.find_object( acnt.first ) != nullptr,
         internal_verify_auth_account_not_found,
         "Account ${a} specified in authority does not exist",
         ("a", acnt.first) );
   }
}

void_result account_create_evaluator::do_evaluate( const account_create_operation& op )
{ try {
   database& d = db();
   FC_ASSERT( d.find_object(op.options.voting_account), "Invalid proxy account specified." );
   FC_ASSERT( fee_paying_account->is_lifetime_member(), "Only Lifetime members may register an account." );
   FC_ASSERT( op.referrer(d).is_member(d.head_block_time()), "The referrer must be either a lifetime or annual subscriber." );

   const auto& global_props = d.get_global_properties();
   const auto& chain_params = global_props.parameters;

   try
   {
      verify_authority_accounts( d, op.owner );
      verify_authority_accounts( d, op.active );
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_create_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_create_auth_account_not_found )

   uint32_t max_vote_id = global_props.next_available_vote_id;

   FC_ASSERT( op.options.num_witness <= chain_params.maximum_witness_count,
              "Voted for more witnesses than currently allowed (${c})", ("c", chain_params.maximum_witness_count) );

   FC_ASSERT( op.options.num_committee <= chain_params.maximum_committee_count,
              "Voted for more committee members than currently allowed (${c})", ("c", chain_params.maximum_committee_count) );

   safe<uint32_t> counts[vote_id_type::VOTE_TYPE_COUNT];
   for( auto id : op.options.votes )
   {
      FC_ASSERT( id < max_vote_id );
      counts[id.type()]++;
   }
   FC_ASSERT(counts[vote_id_type::witness] <= op.options.num_witness,
             "",
             ("count", counts[vote_id_type::witness])("num", op.options.num_witness));
   FC_ASSERT(counts[vote_id_type::committee] <= op.options.num_committee,
             "",
             ("count", counts[vote_id_type::committee])("num", op.options.num_committee));

   auto& acnt_indx = d.get_index_type<account_index>();
   if( op.name.size() )
   {
      auto current_account_itr = acnt_indx.indices().get<by_name>().find( op.name );
      FC_ASSERT( current_account_itr == acnt_indx.indices().get<by_name>().end() );
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type account_create_evaluator::do_apply( const account_create_operation& o )
{ try {

   uint16_t referrer_percent = o.referrer_percent;
   bool has_small_percent = (
         (db().head_block_time() <= HARDFORK_453_TIME)
      && (o.referrer != o.registrar  )
      && (o.referrer_percent != 0    )
      && (o.referrer_percent <= 0x100)
      );

   if( has_small_percent )
   {
      if( referrer_percent >= 100 )
      {
         wlog( "between 100% and 0x100%:  ${o}", ("o", o) );
      }
      referrer_percent = referrer_percent*100;
      if( referrer_percent > GRAPHENE_100_PERCENT )
         referrer_percent = GRAPHENE_100_PERCENT;
   }

   const auto& new_acnt_object = db().create<account_object>( [&]( account_object& obj ){
         obj.registrar = o.registrar;
         obj.referrer = o.referrer;
         obj.lifetime_referrer = o.referrer(db()).lifetime_referrer;

         auto& params = db().get_global_properties().parameters;
         obj.network_fee_percentage = params.network_percent_of_fee;
         obj.lifetime_referrer_fee_percentage = params.lifetime_referrer_percent_of_fee;
         obj.referrer_rewards_percentage = referrer_percent;

         obj.name             = o.name;
         obj.owner            = o.owner;
         obj.active           = o.active;
         obj.options          = o.options;
         obj.statistics = db().create<account_statistics_object>([&](account_statistics_object& s){s.owner = obj.id;}).id;
   });

   if( has_small_percent )
   {
      wlog( "Account affected by #453 registered in block ${n}:  ${na} reg=${reg} ref=${ref}:${refp} ltr=${ltr}:${ltrp}",
         ("n", db().head_block_num()) ("na", new_acnt_object.id)
         ("reg", o.registrar) ("ref", o.referrer) ("ltr", new_acnt_object.lifetime_referrer)
         ("refp", new_acnt_object.referrer_rewards_percentage) ("ltrp", new_acnt_object.lifetime_referrer_fee_percentage) );
      wlog( "Affected account object is ${o}", ("o", new_acnt_object) );
   }

   const auto& dynamic_properties = db().get_dynamic_global_properties();
   db().modify(dynamic_properties, [](dynamic_global_property_object& p) {
      ++p.accounts_registered_this_interval;
   });

   const auto& global_properties = db().get_global_properties();
   if( dynamic_properties.accounts_registered_this_interval %
       global_properties.parameters.accounts_per_fee_scale == 0 )
      db().modify(global_properties, [&dynamic_properties](global_property_object& p) {
         p.parameters.current_fees->get<account_create_operation>().basic_fee <<= p.parameters.account_fee_scale_bitshifts;
      });

   return new_acnt_object.id;
} FC_CAPTURE_AND_RETHROW((o)) }


struct account_update_operation_ext_evaluate_visitor
{
   account_update_operation_ext_evaluate_visitor( const database& d, const account_update_operation& o )
      : db(d), op(o), acct( d.get(o.acct) )
   {}

   void operator()( const void_t& e ) {}

   void operator()( const account_update_operation::ext::create_committee& e )
   {
      // Note, it is a non-obvious property that you cannot include both create_committee
      // and update_committee in the same operation.  The reason is that the create (update) ext checks
      // the committee is absent (present) during the operation's evaluate, thus one of these checks will
      // fail.

      // TODO invalid before HF time
      // TODO also invalid in proposals!!!

      FC_ASSERT( !acct.committee.valid() );
      FC_ASSERT( acct.is_lifetime_member() );
      FC_ASSERT( e.review_period_sec <= get_global_properties().maximum_proposal_lifetime );
      db.get( e.committee_asset );    // throw if asset doesn't exist
   }

   void operator()( const account_update_operation::ext::update_committee& e )
   {
      // TODO invalid before HF time

      FC_ASSERT( acct.committee.valid() );
      committee_object& c = (*account.committee)(db);
      uint16_t new_min_size = c.min_committee_size;
      uint16_t new_max_size = c.max_committee_size;
      if( e.new_min_committee_size.valid() )
         new_min_size = *(e.new_min_committee_size);
      if( e.new_max_committee_size.valid() )
         new_max_size = *(e.new_max_committee_size);
      FC_ASSERT( new_min_size <= new_max_size );
      if( e.new_review_period_sec.valid() )
      {
         FC_ASSERT( *(e.new_review_period_sec) <= get_global_properties().maximum_proposal_lifetime );
      }
   }

   const database& db;
   const account_update_operation& op;
   const account_object& acct;
};

struct account_update_operation_options_ext_evaluate_visitor
{
   account_update_operation_options_ext_evaluate_visitor( const database& d, const account_update_operation& o )
      : db(d), op(o)
   {
      assert( o.new_options.valid() );
   }

   void operator()( const void_t& e ) {}

   void operator()( const account_options::ext::vote_committee_size& e )
   {
      flat_map< account_id_type, uint16_t > ca2votes;
      const auto& committee_idx = _db.get_index_type<committee_member_index>().indices().get<by_vote_id>();

      for( const vote_id_type& vote_id : o.new_options->votes )
      {
         if( id.type() == vote_id_type::committee )
         {
            auto it = committee_idx.find( id );
            FC_ASSERT( it != committee_idx.end() );
            auto it2 = ca2votes.find( it->committee_account );
            if( it2 == ca2votes.end() )
               ca2votes[it->committee_account] = 1;
            else
               ++(it2->second);
         }
      }

      for( auto it=e.committee_size.begin(); it != e.committee_size.end(); ++it )
      {
         // can only vote for committee size that exists
         db.get( it->first );
         if( it->second == 0 )
            continue;
         auto it2 = id2votes.find( it->first );
         // can vote for size==n only if you vote for at least n members
         FC_ASSERT( it2 && (it2->second >= it->second) );
      }

      // TODO:  Same check with num_committee against the (legacy) default committee
      // Or should we enforce num_committee == 0 after hardfork so wallets must use new
      // format to vote for committee?
   }

   const database& db;
   const account_update_operation& op;
};

void_result account_update_evaluator::do_evaluate( const account_update_operation& o )
{ try {
   database& d = db();

   const auto& chain_params = d.get_global_properties().parameters;

   try
   {
      if( o.owner )  verify_authority_accounts( d, *o.owner );
      if( o.active ) verify_authority_accounts( d, *o.active );
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_update_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_update_auth_account_not_found )

   acnt = &o.account(d);

   if( o.new_options )
   {
      FC_ASSERT( o.new_options->num_witness <= chain_params.maximum_witness_count );
      FC_ASSERT( o.new_options->num_committee <= chain_params.maximum_committee_count );
      uint32_t max_vote_id = d.get_global_properties().next_available_vote_id;
      for( auto id : o.new_options->votes )
      {
         FC_ASSERT( id < max_vote_id );
      }
      if( o.new_options->extensions.size() > 0 )
      {
         account_update_operation_options_ext_evaluate_visitor vtor;
         for( const extensions_type& ext : o.new_options->extensions )
         {
            ext.visit( vtor );
         }
      }
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

struct account_update_operation_ext_apply_visitor
{
   account_update_operation_ext_apply_visitor( const database& d, const account_update_operation& o )
      : db(d), op(o), acct( d.get(o.acct) )
   {}

   void operation()( const void_t& e ) {}

   void operation()( const account_update_operation::ext::create_committee& e )
   {
      committee_id_type committee_id = db.create< committee_object >( [&]( committee_object& committee )
      {
         committee.committee_asset = e.committee_asset;
         committee.min_committee_size = e.min_committee_size;
         committee.max_committee_size = e.max_committee_size;
         committee.review_period_seconds = e.review_period_seconds;
      } ).id;
      db.modify( acct, [&]( const account_object& a )
      {
         a.committee = committee_id;
      } );
   }

   void operation()( const account_update_operation::ext::update_committee& e )
   {
      db.modify( (*acct.committee)(db), [&]( committee_object& c )
      {
         if( e.new_min_committee_size.valid() )
            c.min_committee_size = *(e.new_min_committee_size);
         if( e.new_max_committee_size.valid() )
            c.max_committee_size = *(e.new_max_committee_size);
         if( e.new_review_period_seconds.valid() )
            c.review_period_seconds = *(e.new_max_committee_size);
      } );
   }

   const database& db;
   const account_update_operation& op;
   const account_object& acct;
};

void_result account_update_evaluator::do_apply( const account_update_operation& o )
{ try {
   db().modify( *acnt, [&](account_object& a){
      if( o.owner ) a.owner = *o.owner;
      if( o.active ) a.active = *o.active;
      if( o.new_options ) a.options = *o.new_options;
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_whitelist_evaluator::do_evaluate(const account_whitelist_operation& o)
{ try {
   database& d = db();

   listed_account = &o.account_to_list(d);
   if( !d.get_global_properties().parameters.allow_non_member_whitelists )
      FC_ASSERT(o.authorizing_account(d).is_lifetime_member());

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_whitelist_evaluator::do_apply(const account_whitelist_operation& o)
{ try {
   database& d = db();

   d.modify(*listed_account, [&o](account_object& a) {
      if( o.new_listing & o.white_listed )
         a.whitelisting_accounts.insert(o.authorizing_account);
      else
         a.whitelisting_accounts.erase(o.authorizing_account);

      if( o.new_listing & o.black_listed )
         a.blacklisting_accounts.insert(o.authorizing_account);
      else
         a.blacklisting_accounts.erase(o.authorizing_account);
   });

   /** for tracking purposes only, this state is not needed to evaluate */
   d.modify( o.authorizing_account(d), [&]( account_object& a ) {
     if( o.new_listing & o.white_listed )
        a.whitelisted_accounts.insert( o.account_to_list );
     else
        a.whitelisted_accounts.erase( o.account_to_list );

     if( o.new_listing & o.black_listed )
        a.blacklisted_accounts.insert( o.account_to_list );
     else
        a.blacklisted_accounts.erase( o.account_to_list );
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_upgrade_evaluator::do_evaluate(const account_upgrade_evaluator::operation_type& o)
{ try {
   database& d = db();

   account = &d.get(o.account_to_upgrade);
   FC_ASSERT(!account->is_lifetime_member());

   return {};
//} FC_CAPTURE_AND_RETHROW( (o) ) }
} FC_RETHROW_EXCEPTIONS( error, "Unable to upgrade account '${a}'", ("a",o.account_to_upgrade(db()).name) ) }

void_result account_upgrade_evaluator::do_apply(const account_upgrade_evaluator::operation_type& o)
{ try {
   database& d = db();

   d.modify(*account, [&](account_object& a) {
      if( o.upgrade_to_lifetime_member )
      {
         // Upgrade to lifetime member. I don't care what the account was before.
         a.statistics(d).process_fees(a, d);
         a.membership_expiration_date = time_point_sec::maximum();
         a.referrer = a.registrar = a.lifetime_referrer = a.get_id();
         a.lifetime_referrer_fee_percentage = GRAPHENE_100_PERCENT - a.network_fee_percentage;
      } else if( a.is_annual_member(d.head_block_time()) ) {
         // Renew an annual subscription that's still in effect.
         FC_ASSERT(a.membership_expiration_date - d.head_block_time() < fc::days(3650),
                   "May not extend annual membership more than a decade into the future.");
         a.membership_expiration_date += fc::days(365);
      } else {
         // Upgrade from basic account.
         a.statistics(d).process_fees(a, d);
         assert(a.is_basic_account(d.head_block_time()));
         a.referrer = a.get_id();
         a.membership_expiration_date = d.head_block_time() + fc::days(365);
      }
   });

   return {};
} FC_RETHROW_EXCEPTIONS( error, "Unable to upgrade account '${a}'", ("a",o.account_to_upgrade(db()).name) ) }

} } // graphene::chain
