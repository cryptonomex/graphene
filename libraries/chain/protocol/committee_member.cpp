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
#include <graphene/chain/protocol/committee_member.hpp>
#include <graphene/chain/protocol/operation.hpp>

namespace graphene { namespace chain {

void committee_member_create_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT(url.size() < GRAPHENE_MAX_URL_LENGTH );
}

void committee_member_update_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   if( new_url.valid() )
      FC_ASSERT(new_url->size() < GRAPHENE_MAX_URL_LENGTH );
}

void committee_member_update_global_parameters_operation::validate() const
{
   FC_ASSERT( fee.amount >= 0 );
   new_parameters.validate();
}

struct fee_field_validate_visitor
{
   typedef void result_type;

   fee_field_validate_visitor( field_index_type _findex ) : findex( _findex ) {}

   field_index_type findex;
   uint64_t new_value;

   template< typename T >
   void operator()( const T& op )
   {
      check_imap_field< T::fee_schedule >( findex, new_value );
   }
};

void committee_member_update_parameter_operation::validate() const
{
   FC_ASSERT( fee.amount >= 0 );
   switch( which_target_space.value )
   {
      case target_space_parameters:
      {
         FC_ASSERT( param1.value <= std::numeric_limits< field_index_type >::max() );
         FC_ASSERT( param2.value == 0 );
         check_imap_field< chain_parameters >( field_index_type( param1 ), new_value );
         break;
      }
      case target_space_fees:
      {
         FC_ASSERT( param1.value < operation::count() );
         FC_ASSERT( param2.value <= std::numeric_limits< field_index_type >::max() );
         operation op;
         op.set_which( param1.value );
         fee_validate_visitor
         op.visit( vtor );
         break;
      }
      default:
         FC_ASSERT( false );
   }
}

} } // graphene::chain
