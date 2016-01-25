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
#include <graphene/chain/asset_object.hpp>

namespace graphene { namespace chain {

share_type transfer_operation::calculate_fee( const fee_parameters_type& schedule )const
{
   // FIXME need hard fork check logic here or somewhere else for backward compatibility.
   FC_THROW( "Deprecated. Use calculate_fee( const fee_parameters_type& schedule, const asset_object& asset) instead." );
}

share_type transfer_operation::calculate_fee( const fee_parameters_type& schedule, const asset_object& asset_obj)const
{
   share_type core_fee_required;
   auto o = asset_obj.get_transfer_fee_mode();
   if( o == asset_transfer_fee_mode_flat || asset_obj.options.core_exchange_rate.is_null() ) // flat fee mode
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
   FC_THROW( "Use calculate_fee( const fee_parameters_type& schedule, const asset_object& asset) instead." );
}

share_type transfer_v2_operation::calculate_fee( const fee_parameters_type& schedule, const asset_object& asset_obj)const
{
   share_type core_fee_required;
   auto o = asset_obj.get_transfer_fee_mode();
   if( o == asset_transfer_fee_mode_flat || asset_obj.options.core_exchange_rate.is_null() ) // flat fee mode
   {
      core_fee_required = schedule.fee;
   }
   else if( o == asset_transfer_fee_mode_percentage_simple ) // simple percentage fee mode
   {
      // need to know CER of amount.asset_id so that fee can be calculated
      // fee = amount.amount * ~asset.CER * transfer_v2_operation.fee_parameters_type.percentage
      auto core_amount = amount * ( ~asset_obj.options.core_exchange_rate );
      auto core_fee_amount = fc::uint128(core_amount.amount.value);
      core_fee_amount *= schedule.percentage;
      core_fee_amount /= GRAPHENE_100_PERCENT;
      core_fee_required = core_fee_amount.to_uint64();
      if( core_fee_required < schedule.min_fee ) core_fee_required = schedule.min_fee;
      if( core_fee_required > schedule.max_fee ) core_fee_required = schedule.max_fee;
   }
   if( memo )
      core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), schedule.price_per_kbyte );
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
