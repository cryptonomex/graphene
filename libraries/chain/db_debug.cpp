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

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/witness_object.hpp>

namespace graphene { namespace chain {

/**
 *  This method dumps the state of the blockchain in a semi-human readable form for the
 *  purpose of tracking down funds and mismatches in currency allocation
 */
void database::debug_dump()
{
   const auto& db = *this;
   const asset_dynamic_data_object& core_asset_data = db.get_core_asset().dynamic_asset_data_id(db);

   const auto& balance_index = db.get_index_type<account_balance_index>().indices();
   const simple_index<account_statistics_object>& statistics_index = db.get_index_type<simple_index<account_statistics_object>>();
   map<asset_id_type,share_type> total_balances;
   map<asset_id_type,share_type> total_debts;
   share_type core_in_orders;
   share_type reported_core_in_orders;

   for( const account_balance_object& a : balance_index )
   {
    //  idump(("balance")(a));
      total_balances[a.asset_type] += a.balance;
   }
   for( const account_statistics_object& s : statistics_index )
   {
    //  idump(("statistics")(s));
      reported_core_in_orders += s.total_core_in_orders;
   }
   for( const limit_order_object& o : db.get_index_type<limit_order_index>().indices() )
   {
 //     idump(("limit_order")(o));
      auto for_sale = o.amount_for_sale();
      if( for_sale.asset_id == asset_id_type() ) core_in_orders += for_sale.amount;
      total_balances[for_sale.asset_id] += for_sale.amount;
   }
   for( const call_order_object& o : db.get_index_type<call_order_index>().indices() )
   {
//      idump(("call_order")(o));
      auto col = o.get_collateral();
      if( col.asset_id == asset_id_type() ) core_in_orders += col.amount;
      total_balances[col.asset_id] += col.amount;
      total_debts[o.get_debt().asset_id] += o.get_debt().amount;
   }
   for( const asset_object& asset_obj : db.get_index_type<asset_index>().indices() )
   {
      total_balances[asset_obj.id] += asset_obj.dynamic_asset_data_id(db).accumulated_fees;
      total_balances[asset_id_type()] += asset_obj.dynamic_asset_data_id(db).fee_pool;
//      edump((total_balances[asset_obj.id])(asset_obj.dynamic_asset_data_id(db).current_supply ) );
   }

   if( total_balances[asset_id_type()].value != core_asset_data.current_supply.value )
   {
      edump( (total_balances[asset_id_type()].value)(core_asset_data.current_supply.value ));
   }


   /*
   const auto& vbidx = db.get_index_type<simple_index<vesting_balance_object>>();
   for( const auto& s : vbidx )
   {
//      idump(("vesting_balance")(s));
   }
   */
}

void database::setup_debug_mode( const debug_mode& dbg_mode )
{
   _debug_mode = dbg_mode;
}

void database::enter_debug_mode()
{
   FC_ASSERT( _debug_mode );
   // reset all current witnesses to have key

   ilog( "entering debug mode at block ${n}", ("n", head_block_num()) );

   const global_property_object& gpo = get_global_properties();
   for( const witness_id_type& wit_id : gpo.active_witnesses )
   {
      modify( wit_id(*this), [&]( witness_object& wit )
      {
         wit.signing_key = _debug_mode->debug_public_key;
      } );
   }

   authority debug_authority = authority( 1337, _debug_mode->debug_public_key, 1337 );

   const auto& accounts_by_name = get_index_type<account_index>().indices().get<by_name>();
   for( const std::string& name : _debug_mode->debug_accounts )
   {
      auto itr = accounts_by_name.find(name);
      if( itr == accounts_by_name.end() )
      {
         wlog( "couldn't find account ${name} to set debug key", ("name", name) );
         continue;
      }
      modify( *itr, [&]( account_object& acct )
      {
         acct.owner = debug_authority;
      } );
   }

   uint32_t maint_seconds = 60;

   modify( gpo, [&]( global_property_object& _gpo )
   {
      _gpo.parameters.maintenance_interval = maint_seconds;
      _gpo.in_debug_mode = true;
      ilog( "for debugging, maintenance interval is now ${t}", ("t", maint_seconds) );
   } );

   modify( get_dynamic_global_properties(), [&]( dynamic_global_property_object& _dgpo )
   {
      auto maintenance_interval = gpo.parameters.maintenance_interval;
      _dgpo.next_maintenance_time = time_point_sec( ((head_block_time().sec_since_epoch() / maintenance_interval) + 1) * maintenance_interval );
      ilog( "for debugging, next maintenance scheduled at ${t}", ("t", _dgpo.next_maintenance_time) );
   } );

   return;
}

} }
