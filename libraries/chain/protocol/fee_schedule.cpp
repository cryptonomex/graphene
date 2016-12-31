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
#include <algorithm>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <fc/smart_ref_impl.hpp>
#include <boost/tti/has_member_function.hpp>

namespace fc
{
   // explicitly instantiate the smart_ref, gcc fails to instantiate it in some release builds
   //template graphene::chain::fee_schedule& smart_ref<graphene::chain::fee_schedule>::operator=(smart_ref<graphene::chain::fee_schedule>&&);
   //template graphene::chain::fee_schedule& smart_ref<graphene::chain::fee_schedule>::operator=(U&&);
   //template graphene::chain::fee_schedule& smart_ref<graphene::chain::fee_schedule>::operator=(const smart_ref&);
   //template smart_ref<graphene::chain::fee_schedule>::smart_ref();
   //template const graphene::chain::fee_schedule& smart_ref<graphene::chain::fee_schedule>::operator*() const;
}

#define MAX_FEE_STABILIZATION_ITERATION 4

namespace graphene { namespace chain {

   typedef fc::smart_ref<fee_schedule> smart_fee_schedule;

   static smart_fee_schedule tmp;

   fee_schedule::fee_schedule()
   {
   }

   fee_schedule fee_schedule::get_default()
   {
      fee_schedule result;
      for( int i = 0; i < fee_parameters().count(); ++i )
      {
         fee_parameters x; x.set_which(i);
         result.parameters.insert(x);
      }
      return result;
   }

   struct fee_schedule_validate_visitor
   {
      typedef void result_type;

      template<typename T>
      void operator()( const T& p )const
      {
         //p.validate();
      }
   };

   void fee_schedule::validate()const
   {
      for( const auto& f : parameters )
         f.visit( fee_schedule_validate_visitor() );
   }

   struct calc_fee_visitor
   {
      typedef uint64_t result_type;

      const fee_parameters& param;
      calc_fee_visitor( const fee_parameters& p ):param(p){}

      template<typename OpType>
      result_type operator()(  const OpType& op )const
      {
         return op.calculate_fee( param.get<typename OpType::fee_parameters_type>() ).value;
      }

   };

   BOOST_TTI_HAS_MEMBER_FUNCTION(calculate_fee_extended)

   struct calc_fee_extended_visitor
   {
      typedef uint64_t result_type;

      const fee_parameters& param;
      const variant& extended;

      calc_fee_extended_visitor( const fee_parameters& p, const variant& e ) : param(p), extended(e) {}

      template<typename OpType>
      result_type calculate_operation_fee( const OpType& op, std::true_type )const
      {
         auto& fee_param = param.get<typename OpType::fee_parameters_type>();
         return op.calculate_fee_extended( fee_param, extended ).value;
      }

      template<typename OpType>
      result_type calculate_operation_fee( const OpType& op, std::false_type )const
      {
         auto& fee_param = param.get<typename OpType::fee_parameters_type>();
         return op.calculate_fee( fee_param ).value;
      }

      template<typename OpType>
      result_type operator()( const OpType& op )const
      {
         const bool b = has_member_function_calculate_fee_extended<
                              OpType,
                              share_type,
                              boost::mpl::vector<const typename OpType::fee_parameters_type&, const variant&>,
                              boost::function_types::const_qualified
                        >::value;
         return calculate_operation_fee( op, std::integral_constant<bool, b>() );
      }

   };

   struct is_fee_scalable_visitor
   {
      typedef bool result_type;

      is_fee_scalable_visitor() {}

      template<typename OpType>
      result_type operator()(  const OpType& op )const
      {
         return op.is_fee_scalable();
      }

   };

   struct set_fee_visitor
   {
      typedef void result_type;
      asset _fee;

      set_fee_visitor( asset f ):_fee(f){}

      template<typename OpType>
      void operator()( OpType& op )const
      {
         op.fee = _fee;
      }
   };

   struct zero_fee_visitor
   {
      typedef void result_type;

      template<typename ParamType>
      result_type operator()(  ParamType& op )const
      {
         memset( (char*)&op, 0, sizeof(op) );
      }
   };

   void fee_schedule::zero_all_fees()
   {
      *this = get_default();
      for( fee_parameters& i : parameters )
         i.visit( zero_fee_visitor() );
      this->scale = 0;
   }

   fee_parameters fee_schedule::find_op_fee_parameters( const operation& op )const
   {
      fee_parameters params; params.set_which(op.which());
      auto itr = parameters.find(params);
      if( itr != parameters.end() ) params = *itr;
      return params;
   }

   asset fee_schedule::calculate_fee( const operation& op, const price& core_exchange_rate )const
   {
      //idump( (op)(core_exchange_rate) );
      const fee_parameters& params = find_op_fee_parameters( op );
      auto base_value = op.visit( calc_fee_visitor( params ) );
      return scale_and_convert_fee( base_value, core_exchange_rate );
   }

   asset fee_schedule::calculate_fee_extended( const operation& op,
                                               const variant& extended,
                                               const price& core_exchange_rate )const
   {
      //idump( (op)(extended)(core_exchange_rate) );
      const fee_parameters& params = find_op_fee_parameters( op );
      auto base_value = op.visit( calc_fee_extended_visitor( params, extended ) );
      bool is_fee_scalable = op.visit( is_fee_scalable_visitor() );
      if( is_fee_scalable ) return scale_and_convert_fee( base_value, core_exchange_rate );
      else return convert_fee( base_value, core_exchange_rate );
   }

   asset fee_schedule::scale_and_convert_fee( const uint64_t base_value, const price& core_exchange_rate )const
   {
      //idump( (base_value)(core_exchange_rate) );
      return convert_fee( scale_fee( base_value ), core_exchange_rate );
   }

   fc::uint128 fee_schedule::scale_fee( const uint64_t base_value )const
   {
      auto scaled = fc::uint128(base_value) * scale;
      scaled /= GRAPHENE_100_PERCENT;
      //idump( (base_value)(scaled) );
      FC_ASSERT( scaled <= GRAPHENE_MAX_SHARE_SUPPLY );
      return scaled;
   }

   asset fee_schedule::convert_fee( const fc::uint128& scaled, const price& core_exchange_rate )const
   {
      auto result = asset( scaled.to_uint64(), asset_id_type(0) ) * core_exchange_rate;
      //FC_ASSERT( result * core_exchange_rate >= asset( scaled.to_uint64()) );

      while( result * core_exchange_rate < asset( scaled.to_uint64()) )
        result.amount++;

      FC_ASSERT( result.amount <= GRAPHENE_MAX_SHARE_SUPPLY );
      return result;
   }

   asset fee_schedule::set_fee( operation& op, const price& core_exchange_rate )const
   {
      auto f = calculate_fee( op, core_exchange_rate );
      auto f_max = f;
      for( int i=0; i<MAX_FEE_STABILIZATION_ITERATION; i++ )
      {
         op.visit( set_fee_visitor( f_max ) );
         auto f2 = calculate_fee( op, core_exchange_rate );
         if( f == f2 )
            break;
         f_max = std::max( f_max, f2 );
         f = f2;
         if( i == 0 )
         {
            // no need for warnings on later iterations
            wlog( "set_fee requires multiple iterations to stabilize with core_exchange_rate ${p} on operation ${op}",
               ("p", core_exchange_rate) ("op", op) );
         }
      }
      return f_max;
   }

   asset fee_schedule::set_fee_extended( operation& op, const variant& extended, const price& core_exchange_rate )const
   {
      auto f = calculate_fee_extended( op, extended, core_exchange_rate );
      auto f_max = f;
      for( int i=0; i<MAX_FEE_STABILIZATION_ITERATION; i++ )
      {
         op.visit( set_fee_visitor( f_max ) );
         auto f2 = calculate_fee_extended( op, extended, core_exchange_rate );
         if( f == f2 )
            break;
         f_max = std::max( f_max, f2 );
         f = f2;
         if( i == 0 )
         {
            // no need for warnings on later iterations
            wlog( "set_fee requires multiple iterations to stabilize with core_exchange_rate ${p} on operation ${op}",
               ("p", core_exchange_rate) ("op", op) );
         }
      }
      return f_max;
   }

   void chain_parameters::validate()const
   {
      current_fees->validate();
      FC_ASSERT( reserve_percent_of_fee <= GRAPHENE_100_PERCENT );
      FC_ASSERT( network_percent_of_fee <= GRAPHENE_100_PERCENT );
      FC_ASSERT( lifetime_referrer_percent_of_fee <= GRAPHENE_100_PERCENT );
      FC_ASSERT( network_percent_of_fee + lifetime_referrer_percent_of_fee <= GRAPHENE_100_PERCENT );

      FC_ASSERT( block_interval >= GRAPHENE_MIN_BLOCK_INTERVAL );
      FC_ASSERT( block_interval <= GRAPHENE_MAX_BLOCK_INTERVAL );
      FC_ASSERT( block_interval > 0 );
      FC_ASSERT( maintenance_interval > block_interval,
                 "Maintenance interval must be longer than block interval" );
      FC_ASSERT( maintenance_interval % block_interval == 0,
                 "Maintenance interval must be a multiple of block interval" );
      FC_ASSERT( maximum_transaction_size >= GRAPHENE_MIN_TRANSACTION_SIZE_LIMIT,
                 "Transaction size limit is too low" );
      FC_ASSERT( maximum_block_size >= GRAPHENE_MIN_BLOCK_SIZE_LIMIT,
                 "Block size limit is too low" );
      FC_ASSERT( maximum_time_until_expiration > block_interval,
                 "Maximum transaction expiration time must be greater than a block interval" );
      FC_ASSERT( maximum_proposal_lifetime - committee_proposal_review_period > block_interval,
                 "Committee proposal review period must be less than the maximum proposal lifetime" );
   }

} } // graphene::chain
