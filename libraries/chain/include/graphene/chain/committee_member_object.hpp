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
#pragma once
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   class account_object;

   /**
    *  @brief tracks information about a committee_member account.
    *  @ingroup object
    *
    *  A committee_member is responsible for setting blockchain parameters and has
    *  dynamic multi-sig control over the committee account.  The current set of
    *  active committee_members has control.
    *
    *  committee_members were separated into a separate object to make iterating over
    *  the set of committee_member easy.
    */
   class committee_member_object : public abstract_object<committee_member_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = committee_member_object_type;

         account_id_type  committee_member_account;
         vote_id_type     vote_id;
         uint64_t         total_votes = 0;
         string           url;
         account_id_type  committee_account;
   };

   struct by_vote_id;
   struct by_account_committee;

   class committee_object : public abstract_object<committee_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = committee_object_type;

         asset_id_type    committee_asset;
         account_id_type  committee_account;
         uint16_t         min_committee_size = 1;
         uint16_t         max_committee_size = 1;
         uint32_t         review_period_seconds = 0;
   };

   using committee_member_multi_index_type = multi_index_container<
      committee_member_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_account_committee>,
            composite_key<
               committee_member_object,
               member<committee_member_object, account_id_type, &committee_member_object::committee_member_account>
               member<committee_member_object, account_id_type, &committee_member_object::committee_account>
            >
         >,
         ordered_unique< tag<by_vote_id>,
            member<committee_member_object, vote_id_type, &committee_member_object::vote_id>
         >
      >
   >;
   using committee_member_index = generic_index<committee_member_object, committee_member_multi_index_type>;

   struct by_asset;

   typedef multi_index_container<
      committee_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
         ordered_unique< tag<by_asset>, member< committee_object, asset_id_type, &committee_object::committee_asset > >
      >
   > committee_multi_index_type;

   /**
    * @ingroup object_index
    */
   typedef generic_index< committee_object, committee_multi_index_type> committee_index;

} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::committee_member_object, (graphene::db::object),
   (committee_member_account)
   (vote_id)
   (total_votes)
   (url)
   (committee_account)
   )
FC_REFLECT_DERIVED( graphene::chain::committee_object, (graphene::db::object),
   (committee_asset)
   (committee_account)
   (min_committee_size)
   (max_committee_size)
   (review_period_seconds)
   )
