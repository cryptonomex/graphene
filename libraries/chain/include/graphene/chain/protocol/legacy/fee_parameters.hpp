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
#pragma once
#include <graphene/chain/protocol/operations.hpp>

namespace graphene { namespace chain { namespace legacy {

// legacy::fee_parameters_type<T> for each operation defines the fee_parameters_type for the operation at launch
// it is necessary to keep the old definitions around so we are able to parse the pre-554 fee setting operation
//
// We DON'T do things such as applying transform_to_fee_parameters to operation here, instead listing the static_variant
// out manually.  The reason is that any post-554 updates to optype::fee_parameters would result in an incompatible
// serialization change to the legacy committee_member_update_global_parameters_operation, so the structs here are
// the versions used to describe the particular (frozen) serialization of that legacy op.
//

template<>
struct fee_parameters_type< graphene::chain::transfer_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::transfer_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::limit_order_create_operation >
{
   uint64_t   fee                       =      5 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::limit_order_create_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::limit_order_cancel_operation >
{
   uint64_t   fee                       =      0;

   void apply_to( graphene::chain::limit_order_cancel_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::call_order_update_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::call_order_update_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::fill_order_operation >
{

   void apply_to( graphene::chain::fill_order_operation::fee_parameters_type& fp )const
   {
   }
};

template<>
struct fee_parameters_type< graphene::chain::account_create_operation >
{
   uint64_t   basic_fee                 =      5 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint64_t   premium_fee               =   2000 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::account_create_operation::fee_parameters_type& fp )const
   {
      fp.basic_fee = basic_fee;
      fp.premium_fee = premium_fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::account_update_operation >
{
   share_type fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::account_update_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::account_whitelist_operation >
{
   share_type fee                       =      3 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::account_whitelist_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::account_upgrade_operation >
{
   uint64_t   membership_annual_fee     =   2000 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint64_t   membership_lifetime_fee   =  10000 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::account_upgrade_operation::fee_parameters_type& fp )const
   {
      fp.membership_annual_fee = membership_annual_fee;
      fp.membership_lifetime_fee = membership_lifetime_fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::account_transfer_operation >
{
   uint64_t   fee                       =    500 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::account_transfer_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_create_operation >
{
   uint64_t   symbol3                   = 500000 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint64_t   symbol4                   = 300000 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint64_t   long_symbol               =   5000 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10;

   void apply_to( graphene::chain::asset_create_operation::fee_parameters_type& fp )const
   {
      fp.symbol3 = symbol3;
      fp.symbol4 = symbol4;
      fp.long_symbol = long_symbol;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_update_operation >
{
   uint64_t   fee                       =    500 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10;

   void apply_to( graphene::chain::asset_update_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_update_bitasset_operation >
{
   uint64_t   fee                       =    500 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_update_bitasset_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_update_feed_producers_operation >
{
   uint64_t   fee                       =    500 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_update_feed_producers_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_issue_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_issue_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_reserve_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_reserve_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_fund_fee_pool_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_fund_fee_pool_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_settle_operation >
{
   uint64_t   fee                       =    100 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_settle_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_global_settle_operation >
{
   uint64_t   fee                       =    500 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_global_settle_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_publish_feed_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_publish_feed_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::witness_create_operation >
{
   uint64_t   fee                       =   5000 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::witness_create_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::witness_update_operation >
{
   share_type fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::witness_update_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::proposal_create_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10;

   void apply_to( graphene::chain::proposal_create_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::proposal_update_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10;

   void apply_to( graphene::chain::proposal_update_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::proposal_delete_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::proposal_delete_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::withdraw_permission_create_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::withdraw_permission_create_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::withdraw_permission_update_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::withdraw_permission_update_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::withdraw_permission_claim_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10;

   void apply_to( graphene::chain::withdraw_permission_claim_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::withdraw_permission_delete_operation >
{
   uint64_t   fee                       =      0;

   void apply_to( graphene::chain::withdraw_permission_delete_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::committee_member_create_operation >
{
   uint64_t   fee                       =   5000 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::committee_member_create_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::committee_member_update_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::committee_member_update_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::committee_member_update_global_parameters_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::committee_member_update_global_parameters_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::vesting_balance_create_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::vesting_balance_create_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::vesting_balance_withdraw_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::vesting_balance_withdraw_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::worker_create_operation >
{
   uint64_t   fee                       =   5000 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::worker_create_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::custom_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10;

   void apply_to( graphene::chain::custom_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::assert_operation >
{
   uint64_t   fee                       =      1 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::assert_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::balance_claim_operation >
{

   void apply_to( graphene::chain::balance_claim_operation::fee_parameters_type& fp )const
   {
   }
};

template<>
struct fee_parameters_type< graphene::chain::override_transfer_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_kbyte           =     10;

   void apply_to( graphene::chain::override_transfer_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_kbyte = price_per_kbyte;
   }
};

template<>
struct fee_parameters_type< graphene::chain::transfer_to_blind_operation >
{
   uint64_t   fee                       =      5 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_output          =      5 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::transfer_to_blind_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_output = price_per_output;
   }
};

template<>
struct fee_parameters_type< graphene::chain::blind_transfer_operation >
{
   uint64_t   fee                       =      5 * GRAPHENE_BLOCKCHAIN_PRECISION;
   uint32_t   price_per_output          =      5 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::blind_transfer_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
      fp.price_per_output = price_per_output;
   }
};

template<>
struct fee_parameters_type< graphene::chain::transfer_from_blind_operation >
{
   uint64_t   fee                       =      5 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::transfer_from_blind_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_settle_cancel_operation >
{

   void apply_to( graphene::chain::asset_settle_cancel_operation::fee_parameters_type& fp )const
   {
   }
};

template<>
struct fee_parameters_type< graphene::chain::asset_claim_fees_operation >
{
   uint64_t   fee                       =     20 * GRAPHENE_BLOCKCHAIN_PRECISION;

   void apply_to( graphene::chain::asset_claim_fees_operation::fee_parameters_type& fp )const
   {
      fp.fee = fee;
   }
};

template<>
struct fee_parameters_type< graphene::chain::fba_distribute_operation >
{

   void apply_to( graphene::chain::fba_distribute_operation::fee_parameters_type& fp )const
   {
   }
};

typedef fc::static_variant<
   fee_parameters_type< graphene::chain::transfer_operation >,
   fee_parameters_type< graphene::chain::limit_order_create_operation >,
   fee_parameters_type< graphene::chain::limit_order_cancel_operation >,
   fee_parameters_type< graphene::chain::call_order_update_operation >,
   fee_parameters_type< graphene::chain::fill_order_operation >,
   fee_parameters_type< graphene::chain::account_create_operation >,
   fee_parameters_type< graphene::chain::account_update_operation >,
   fee_parameters_type< graphene::chain::account_whitelist_operation >,
   fee_parameters_type< graphene::chain::account_upgrade_operation >,
   fee_parameters_type< graphene::chain::account_transfer_operation >,
   fee_parameters_type< graphene::chain::asset_create_operation >,
   fee_parameters_type< graphene::chain::asset_update_operation >,
   fee_parameters_type< graphene::chain::asset_update_bitasset_operation >,
   fee_parameters_type< graphene::chain::asset_update_feed_producers_operation >,
   fee_parameters_type< graphene::chain::asset_issue_operation >,
   fee_parameters_type< graphene::chain::asset_reserve_operation >,
   fee_parameters_type< graphene::chain::asset_fund_fee_pool_operation >,
   fee_parameters_type< graphene::chain::asset_settle_operation >,
   fee_parameters_type< graphene::chain::asset_global_settle_operation >,
   fee_parameters_type< graphene::chain::asset_publish_feed_operation >,
   fee_parameters_type< graphene::chain::witness_create_operation >,
   fee_parameters_type< graphene::chain::witness_update_operation >,
   fee_parameters_type< graphene::chain::proposal_create_operation >,
   fee_parameters_type< graphene::chain::proposal_update_operation >,
   fee_parameters_type< graphene::chain::proposal_delete_operation >,
   fee_parameters_type< graphene::chain::withdraw_permission_create_operation >,
   fee_parameters_type< graphene::chain::withdraw_permission_update_operation >,
   fee_parameters_type< graphene::chain::withdraw_permission_claim_operation >,
   fee_parameters_type< graphene::chain::withdraw_permission_delete_operation >,
   fee_parameters_type< graphene::chain::committee_member_create_operation >,
   fee_parameters_type< graphene::chain::committee_member_update_operation >,
   fee_parameters_type< graphene::chain::committee_member_update_global_parameters_operation >,
   fee_parameters_type< graphene::chain::vesting_balance_create_operation >,
   fee_parameters_type< graphene::chain::vesting_balance_withdraw_operation >,
   fee_parameters_type< graphene::chain::worker_create_operation >,
   fee_parameters_type< graphene::chain::custom_operation >,
   fee_parameters_type< graphene::chain::assert_operation >,
   fee_parameters_type< graphene::chain::balance_claim_operation >,
   fee_parameters_type< graphene::chain::override_transfer_operation >,
   fee_parameters_type< graphene::chain::transfer_to_blind_operation >,
   fee_parameters_type< graphene::chain::blind_transfer_operation >,
   fee_parameters_type< graphene::chain::transfer_from_blind_operation >,
   fee_parameters_type< graphene::chain::asset_settle_cancel_operation >,
   fee_parameters_type< graphene::chain::asset_claim_fees_operation >,
   fee_parameters_type< graphene::chain::fba_distribute_operation >
   > fee_parameters;

struct fee_schedule
{
   flat_set<fee_parameters> parameters;
   uint32_t                 scale = GRAPHENE_100_PERCENT; ///< fee * scale / GRAPHENE_100_PERCENT
};

} } }

FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::transfer_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::limit_order_create_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::limit_order_cancel_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::call_order_update_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::fill_order_operation >,  )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::account_create_operation >, (basic_fee)(premium_fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::account_update_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::account_whitelist_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::account_upgrade_operation >, (membership_annual_fee)(membership_lifetime_fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::account_transfer_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_create_operation >, (symbol3)(symbol4)(long_symbol)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_update_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_update_bitasset_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_update_feed_producers_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_issue_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_reserve_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_fund_fee_pool_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_settle_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_global_settle_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_publish_feed_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::witness_create_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::witness_update_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::proposal_create_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::proposal_update_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::proposal_delete_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::withdraw_permission_create_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::withdraw_permission_update_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::withdraw_permission_claim_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::withdraw_permission_delete_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::committee_member_create_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::committee_member_update_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::committee_member_update_global_parameters_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::vesting_balance_create_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::vesting_balance_withdraw_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::worker_create_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::custom_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::assert_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::balance_claim_operation >,  )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::override_transfer_operation >, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::transfer_to_blind_operation >, (fee)(price_per_output) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::blind_transfer_operation >, (fee)(price_per_output) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::transfer_from_blind_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_settle_cancel_operation >,  )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::asset_claim_fees_operation >, (fee) )
FC_REFLECT( graphene::chain::legacy::fee_parameters_type< graphene::chain::fba_distribute_operation >,  )

FC_REFLECT_TYPENAME( graphene::chain::legacy::fee_parameters )

FC_REFLECT( graphene::chain::legacy::fee_schedule, (parameters)(scale) )
