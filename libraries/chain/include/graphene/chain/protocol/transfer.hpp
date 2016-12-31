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
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/memo.hpp>

namespace graphene { namespace chain {

   /**
    * @ingroup operations
    *
    * @brief Transfers an amount of one asset from one account to another
    *
    *  Fees are paid by the "from" account
    *
    *  @pre amount.amount > 0
    *  @pre fee.amount >= 0
    *  @pre from != to
    *  @post from account's balance will be reduced by fee and amount
    *  @post to account's balance will be increased by amount
    *  @return n/a
    */
   struct transfer_operation : public base_operation
   {
      struct fee_parameters_type {
         uint64_t fee             =  20 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte =  10 * GRAPHENE_BLOCKCHAIN_PRECISION; /// only required for large memos.
      };
      struct extended_calculate_fee_parameters {
         /// Transfer fee mode of transferring asset
         asset_transfer_fee_mode transferring_asset_transfer_fee_mode  = asset_transfer_fee_mode_flat;
         /// data field for future extensions
         extensions_type extensions;
      };

      asset            fee;
      /// Account to transfer asset from
      account_id_type  from;
      /// Account to transfer asset to
      account_id_type  to;
      /// The amount of asset to transfer from @ref from to @ref to
      asset            amount;

      /// User provided data encrypted to the memo key of the "to" account
      optional<memo_data> memo;
      extensions_type   extensions;

      account_id_type fee_payer()const { return from; }
      void            validate()const;
      /// The original calculate_fee function. Deprecated in this operation.
      share_type      calculate_fee(const fee_parameters_type& k)const;
      /// Extended calculate fee function
      share_type      calculate_fee_extended(const fee_parameters_type& k, const variant& extended)const;
   };

   /**
    * @ingroup operations
    *
    * @brief Transfers an amount of one asset from one account to another, support simple percentage based fee mode
    *
    *  Fees are paid by the "from" account
    *
    *  @pre amount.amount > 0
    *  @pre fee.amount >= 0
    *  @pre from != to
    *  @post from account's balance will be reduced by fee and amount
    *  @post to account's balance will be increased by amount
    *  @return n/a
    */
   struct transfer_v2_operation : public base_operation
   {
      struct fee_parameters_type {
         /// fee required when transferring asset with flat fee mode
         uint64_t flat_fee            =  20 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte     =  10 * GRAPHENE_BLOCKCHAIN_PRECISION; /// only required for large memos.
         /// Minimum fee amount, take effect when transfer asset with percentage based fee mode
         uint32_t percentage_min_fee  =   1 * GRAPHENE_BLOCKCHAIN_PRECISION;
         /// Maximum fee amount, take effect when transfer asset with percentage based fee mode
         uint32_t percentage_max_fee  = 300 * GRAPHENE_BLOCKCHAIN_PRECISION;
         /// the percentage, take effect when transfer asset with percentage based fee mode
         uint16_t percentage          = GRAPHENE_DEFAULT_TRANSFER_FEE_PERCENT;
         /// data field for future extensions
         extensions_type extensions;
      };
      struct extended_calculate_fee_parameters {
         /// Scale
         uint32_t                scale                                 = 1;
         /// Transfer fee mode of transferring asset
         asset_transfer_fee_mode transferring_asset_transfer_fee_mode  = asset_transfer_fee_mode_flat;
         /// core_exchange_rate of transferring asset
         price                   transferring_asset_core_exchange_rate = price::unit_price();
         /// data field for future extensions
         extensions_type extensions;
      };

      asset            fee;
      /// Account to transfer asset from
      account_id_type  from;
      /// Account to transfer asset to
      account_id_type  to;
      /// The amount of asset to transfer from @ref from to @ref to
      asset            amount;

      /// User provided data encrypted to the memo key of the "to" account
      optional<memo_data> memo;
      extensions_type   extensions;

      account_id_type fee_payer()const { return from; }
      void            validate()const;
      /// The original calculate_fee function. Deprecated in this operation.
      share_type      calculate_fee(const fee_parameters_type& k)const;
      /// Extended calculate fee function.
      /// Calculate fee and scale. Since it's better that percentage not scale, we scale inside.
      share_type      calculate_fee_extended(const fee_parameters_type& k, const variant& extended)const;
      /// Override is_fee_scalable() to return false
      virtual bool is_fee_scalable() const override { return false; }
   };

   /**
    *  @class override_transfer_operation
    *  @brief Allows the issuer of an asset to transfer an asset from any account to any account if they have override_authority
    *  @ingroup operations
    *
    *  @pre amount.asset_id->issuer == issuer
    *  @pre issuer != from  because this is pointless, use a normal transfer operation
    */
   struct override_transfer_operation : public base_operation
   {
      struct fee_parameters_type {
         uint64_t fee       = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte = 10; /// only required for large memos.
      };

      asset           fee;
      account_id_type issuer;
      /// Account to transfer asset from
      account_id_type from;
      /// Account to transfer asset to
      account_id_type to;
      /// The amount of asset to transfer from @ref from to @ref to
      asset amount;

      /// User provided data encrypted to the memo key of the "to" account
      optional<memo_data> memo;
      extensions_type   extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const;
   };

}} // graphene::chain

FC_REFLECT( graphene::chain::transfer_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::transfer_v2_operation::fee_parameters_type,
              (flat_fee)(price_per_kbyte)(percentage_min_fee)(percentage_max_fee)(percentage)(extensions) )
FC_REFLECT( graphene::chain::override_transfer_operation::fee_parameters_type, (fee)(price_per_kbyte) )

FC_REFLECT( graphene::chain::override_transfer_operation, (fee)(issuer)(from)(to)(amount)(memo)(extensions) )
FC_REFLECT( graphene::chain::transfer_operation, (fee)(from)(to)(amount)(memo)(extensions) )
FC_REFLECT( graphene::chain::transfer_v2_operation, (fee)(from)(to)(amount)(memo)(extensions) )

FC_REFLECT( graphene::chain::transfer_operation::extended_calculate_fee_parameters,
              (transferring_asset_transfer_fee_mode)(extensions) )
FC_REFLECT( graphene::chain::transfer_v2_operation::extended_calculate_fee_parameters,
              (scale)(transferring_asset_transfer_fee_mode)(transferring_asset_core_exchange_rate)(extensions) )
