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
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>
#include <graphene/chain/protocol/operations.hpp>

namespace graphene { namespace chain {

   class database;
   struct signed_transaction;
   class generic_evaluator;
   class transaction_evaluation_state;

   /**
    * Observes evaluation events, providing
    * pre- and post-evaluation hooks.
    *
    * Every call to pre_evaluate() is followed by
    * a call to either post_evaluate() or evaluation_failed().
    *
    * A subclass which needs to do a "diff" can gather some
    * "before" state into its members in pre_evaluate(),
    * then post_evaluate() will have both "before"
    * and "after" state, and will be able to do the diff.
    *
    * evaluation_failed() is a cleanup method which notifies
    * the subclass to "throw away" the diff.
    */
   class evaluation_observer
   {
   public:
      virtual ~evaluation_observer(){}

      virtual void pre_evaluate(const transaction_evaluation_state& eval_state,
                                const operation& op,
                                bool apply,
                                generic_evaluator* ge)
      {}

      virtual void post_evaluate(const transaction_evaluation_state& eval_state,
                                 const operation& op,
                                 bool apply,
                                 generic_evaluator* ge,
                                 const operation_result& result)
      {}

      virtual void evaluation_failed(const transaction_evaluation_state& eval_state,
                                     const operation& op,
                                     bool apply,
                                     generic_evaluator* ge,
                                     const operation_result& result)
      {}
   };

   class generic_evaluator
   {
   public:
      virtual ~generic_evaluator(){}

      virtual int get_type()const = 0;
      virtual operation_result start_evaluate(transaction_evaluation_state& eval_state, const operation& op, bool apply);

      /**
       * @note derived classes should ASSUME that the default validation that is
       * indepenent of chain state should be performed by op.validate() and should
       * not perform these extra checks.
       */
      virtual operation_result evaluate(const operation& op) = 0;
      virtual operation_result apply(const operation& op) = 0;

      /**
       * Routes the fee to where it needs to go.  The default implementation
       * routes the fee to the account_statistics_object of the fee_paying_account.
       *
       * Before pay_fee() is called, the fee is computed by prepare_fee() and has been
       * moved out of the fee_paying_account and (if paid in a non-CORE asset) converted
       * by the asset's fee pool.
       *
       * Therefore, when pay_fee() is called, the fee only exists in this->core_fee_paid.
       * So pay_fee() need only increment the receiving balance.
       *
       * The default implementation simply calls account_statistics_object->pay_fee() to
       * increment pending_fees or pending_vested_fees.
       */
      virtual void pay_fee();

      database& db()const;

      //void check_required_authorities(const operation& op);
   protected:
      /**
       * @brief Fetch objects relevant to fee payer and set pointer members
       * @param account_id Account which is paying the fee
       * @param fee The fee being paid. May be in assets other than core.
       *
       * This method verifies that the fee is valid and sets the object pointer members and the fee fields. It should
       * be called during do_evaluate.
       *
       * In particular, core_fee_paid field is set by prepare_fee().
       */
      void prepare_fee(account_id_type account_id, asset fee);

      /**
       * Convert the fee into BTS through the exchange pool.
       *
       * Reads core_fee_paid field for how much CORE is deducted from the exchange pool,
       * and fee_from_account for how much USD is added to the pool.
       *
       * Since prepare_fee() does the validation checks ensuring the account and fee pool
       * have sufficient balance and the exchange rate is correct,
       * those validation checks are not replicated here.
       *
       * Rather than returning a value, this method fills in core_fee_paid field.
       */
      void convert_fee();

      object_id_type get_relative_id( object_id_type rel_id )const;

      asset                            fee_from_account;
      share_type                       core_fee_paid;
      const account_object*            fee_paying_account = nullptr;
      const account_statistics_object* fee_paying_account_statistics = nullptr;
      const asset_object*              fee_asset          = nullptr;
      const asset_dynamic_data_object* fee_asset_dyn_data = nullptr;
      transaction_evaluation_state*    trx_state;
   };

   class op_evaluator
   {
   public:
      virtual ~op_evaluator(){}
      virtual operation_result evaluate(transaction_evaluation_state& eval_state, const operation& op, bool apply) = 0;

      vector<evaluation_observer*> eval_observers;
   };

   template<typename T>
   class op_evaluator_impl : public op_evaluator
   {
   public:
      virtual operation_result evaluate(transaction_evaluation_state& eval_state, const operation& op, bool apply = true) override
      {
         // fc::exception from observers are suppressed.
         // fc::exception from evaluation is deferred (re-thrown
         // after all observers receive evaluation_failed)

         T eval;
         shared_ptr<fc::exception> evaluation_exception;
         size_t observer_count = 0;
         operation_result result;

         for( const auto& obs : eval_observers )
         {
            try
            {
               obs->pre_evaluate(eval_state, op, apply, &eval);
            }
            catch( const fc::exception& e )
            {
               elog( "suppressed exception in observer pre method:\n${e}", ( "e", e.to_detail_string() ) );
            }
            observer_count++;
         }

         try
         {
            result = eval.start_evaluate(eval_state, op, apply);
         }
         catch( const fc::exception& e )
         {
            evaluation_exception = e.dynamic_copy_exception();
         }

         while( observer_count > 0 )
         {
            --observer_count;
            const auto& obs = eval_observers[observer_count];
            try
            {
               if( evaluation_exception )
                  obs->post_evaluate(eval_state, op, apply, &eval, result);
               else
                  obs->evaluation_failed(eval_state, op, apply, &eval, result);
            }
            catch( const fc::exception& e )
            {
               elog( "suppressed exception in observer post method:\n${e}", ( "e", e.to_detail_string() ) );
            }
         }

         if( evaluation_exception )
            evaluation_exception->dynamic_rethrow_exception();
         return result;
      }
   };

   template<typename DerivedEvaluator>
   class evaluator : public generic_evaluator
   {
   public:
      virtual int get_type()const override { return operation::tag<typename DerivedEvaluator::operation_type>::value; }

      virtual operation_result evaluate(const operation& o) final override
      {
         auto* eval = static_cast<DerivedEvaluator*>(this);
         const auto& op = o.get<typename DerivedEvaluator::operation_type>();

         prepare_fee(op.fee_payer(), op.fee);
         if( !trx_state->skip_fee_schedule_check )
         {
            GRAPHENE_ASSERT( core_fee_paid >= db().current_fee_schedule().calculate_fee( op ).amount,
                       insufficient_fee,
                       "Insufficient Fee Paid",
                       ("core_fee_paid",core_fee_paid)("required",db().current_fee_schedule().calculate_fee( op ).amount) );
         }

         return eval->do_evaluate(op);
      }

      virtual operation_result apply(const operation& o) final override
      {
         auto* eval = static_cast<DerivedEvaluator*>(this);
         const auto& op = o.get<typename DerivedEvaluator::operation_type>();

         convert_fee();
         pay_fee();

         auto result = eval->do_apply(op);

         db().adjust_balance(op.fee_payer(), -fee_from_account);

         return result;
      }
   };
} }
