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
#include <graphene/chain/protocol/transfer.hpp>

namespace graphene { namespace chain {

share_type transfer_operation::calculate_fee( const fee_parameters_type& schedule )const
{
   FC_THROW( "Deprecated. Use calculate_fee_extended( const fee_parameters_type& schedule, const variant& extended ) instead." );
}

share_type transfer_operation::calculate_fee_extended( const fee_parameters_type& schedule, const variant& extended )const
{
   share_type core_fee_required;
   extended_calculate_fee_parameters p;
   from_variant( extended, p );
   if( p.transferring_asset_transfer_fee_mode == asset_transfer_fee_mode_flat ) // flat fee mode
   {
      core_fee_required = schedule.fee;
   }
   else // other fee modes
   {
      FC_THROW( "transfer_operation doesn't support asset with non-flat fee mode." );
   }
   if( memo )
      core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), schedule.price_per_kbyte );
   return core_fee_required;
}

void transfer_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( from != to );
   FC_ASSERT( amount.amount > 0 );
}


share_type transfer_v2_operation::calculate_fee( const fee_parameters_type& schedule )const
{
   FC_THROW( "Use calculate_fee_extended( const fee_parameters_type& schedule, const variant& extended ) instead." );
}

share_type transfer_v2_operation::calculate_fee_extended( const fee_parameters_type& schedule,
                                                          const variant& extended )const
{
   share_type core_fee_required;
   extended_calculate_fee_parameters p;
   from_variant( extended, p );
   uint32_t scale = p.scale;
   if( p.transferring_asset_transfer_fee_mode == asset_transfer_fee_mode_flat ) // flat fee mode
   {
      auto core_fee_128 = fc::uint128(schedule.flat_fee);
      core_fee_128 *= scale;
      core_fee_128 /= GRAPHENE_100_PERCENT;
      core_fee_required = core_fee_128.to_uint64();
   }
   else if( p.transferring_asset_transfer_fee_mode == asset_transfer_fee_mode_percentage_simple ) // simple percentage fee mode
   {
      // need to know CER of amount.asset_id so that fee can be calculated
      // fee = amount.amount * asset.CER * transfer_v2_operation.fee_parameters_type.percentage
      auto core_amount = amount * p.transferring_asset_core_exchange_rate;
      auto core_fee_amount = fc::uint128(core_amount.amount.value);
      core_fee_amount *= schedule.percentage;
      core_fee_amount /= GRAPHENE_100_PERCENT;
      core_fee_required = core_fee_amount.to_uint64();
      auto min_fee_128 = fc::uint128( schedule.percentage_min_fee );
      min_fee_128 *= scale;
      min_fee_128 /= GRAPHENE_100_PERCENT;
      auto min_fee_64 = min_fee_128.to_uint64();
      auto max_fee_128 = fc::uint128( schedule.percentage_max_fee );
      max_fee_128 *= scale;
      max_fee_128 /= GRAPHENE_100_PERCENT;
      auto max_fee_64 = max_fee_128.to_uint64();
      if( core_fee_required < min_fee_64 ) core_fee_required = min_fee_64;
      if( core_fee_required > max_fee_64 ) core_fee_required = max_fee_64;
   }
   if( memo )
   {
      auto memo_fee_128 = fc::uint128( calculate_data_fee( fc::raw::pack_size(memo), schedule.price_per_kbyte ) );
      memo_fee_128 *= scale;
      memo_fee_128 /= GRAPHENE_100_PERCENT;
      core_fee_required += memo_fee_128.to_uint64();
   }
   return core_fee_required;
}


void transfer_v2_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( from != to );
   FC_ASSERT( amount.amount > 0 );
}



share_type override_transfer_operation::calculate_fee( const fee_parameters_type& schedule )const
{
   share_type core_fee_required = schedule.fee;
   if( memo )
      core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), schedule.price_per_kbyte );
   return core_fee_required;
}


void override_transfer_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( from != to );
   FC_ASSERT( amount.amount > 0 );
   FC_ASSERT( issuer != from );
}

} } // graphene::chain
