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

#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

void database::build_extended_fee_parameters( const operation& op, variant& v )
{
   switch( op.which() )
   {
      case operation::tag<transfer_operation>::value :
      {
         const transfer_operation& o = op.get<transfer_operation>();
         const asset_object& a = get( o.amount.asset_id );
         transfer_operation::extended_calculate_fee_parameters e;
         e.transferring_asset_transfer_fee_mode = a.get_transfer_fee_mode();
         to_variant( e, v );
         break;
      }
      case operation::tag<transfer_v2_operation>::value :
      {
         const transfer_v2_operation& o = op.get<transfer_v2_operation>();
         const asset_object& a = get( o.amount.asset_id );
         transfer_v2_operation::extended_calculate_fee_parameters e;
         e.scale = current_fee_schedule().scale;
         e.transferring_asset_transfer_fee_mode = a.get_transfer_fee_mode();
         e.transferring_asset_core_exchange_rate = a.options.core_exchange_rate;
         to_variant( e, v );
         break;
      }
      default:
         break;
   }
}

} }
