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

#include <fc/optional.hpp>
#include <fc/reflect/reflect.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

/**
 * Let T be an FC reflected type where every field is one of
 * bool, uint8_t, uint16_t, uint32_t, uint64_t, share_type (must be positive).
 *
 * Then T is known as an "imap" and has method set_imap_field<T>( index, val ) which sets
 * the integer field at the given index to the given value.
 */

/**
 * Helper method to convert a field value based on destination type, FC_ASSERT's it's in range.
 */
template< typename T > T convert_imap_field_value( uint64_t x );

/**
 * Set a field given its index and value.  T must have been declared as GRAPHENE_REFLECT_IMAP( TYPE, MEMBERS ).
 */
template< typename T > void set_imap_field<T>( T& obj, field_index_type field, uint64_t val );

/**
 * Determine whether a type is an imap.
 */
template< typename T > struct is_imap;

// convert_imap_field_value implementations ////////////////////////////////////////////////////////

// default implementation
template< typename T >
T convert_imap_field_value( uint64_t x )
{
   FC_ASSERT( false );
   return T(); // prevent warning
}

// specialization for bool
template<>
bool convert_imap_field_value<bool>( uint64_t x )
{
   switch(x)
   {
      case 0:
         return false;
      case 1:
         return true;
      default:
         FC_ASSERT( false );
         return false; // prevent warning
   }
}

// specialization for share_type
template<>
share_type convert_imap_field_value<share_type>( uint64_t x )
{
   FC_ASSERT( x <= GRAPHENE_MAX_SHARE_SUPPLY );
   return share_type(x);
}

// specializations for uint types with range checks based on std::numeric_limits<T>
#define GRAPHENE_DEFINE_CONVERT_IMAP_FIELD_VALUE( TYPE ) \
   template<> TYPE convert_imap_field_value<TYPE>( uint64_t x ) {   FC_ASSERT( x <= std::numeric_limits<TYPE>::max() );  return TYPE(x);   }

GRAPHENE_DEFINE_CONVERT_IMAP_FIELD_VALUE( uint8_t  )
GRAPHENE_DEFINE_CONVERT_IMAP_FIELD_VALUE( uint16_t )
GRAPHENE_DEFINE_CONVERT_IMAP_FIELD_VALUE( uint32_t )
GRAPHENE_DEFINE_CONVERT_IMAP_FIELD_VALUE( uint64_t )

template< typename T >
struct is_imap
{
   typedef fc::false_type is_true;
};

template< typename T >
void set_imap_field<T>( T& obj, field_index_type field, uint64_t val )
{
   elog( "Attempt to set field on unsupported imap type ${t}", ("t", fc::reflect::get_typename<T>::name()) );
   FC_ASSERT( false );
}

#define GRAPHENE_REFLECT_IMAP_VISIT_MEMBER_SET( r, TYPE, INDEX, MEMBER ) \
   case INDEX: \
      MEMBER = convert_imap_field_value< decltype( ((TYPE*) nullptr)->MEMBER) ) >( val ); \
      break;

#define GRAPHENE_REFLECT_IMAP_VISIT_MEMBER_CHECK( r, TYPE, INDEX, MEMBER ) \
   case INDEX: \
      convert_imap_field_value< decltype( ((TYPE*) nullptr)->MEMBER) ) >( val ); \
      break;

#define GRAPHENE_REFLECT_IMAP( TYPE, MEMBERS ) \
   FC_REFLECT( TYPE, MEMBERS ) \
   namespace graphene { namespace chain { \
   template<> \
   void set_imap_field<TYPE>( TYPE& obj, field_index_type field, uint64_t val ) \
   { \
      switch( field ) \
      { \
         BOOST_PP_SEQ_FOR_EACH_I( GRAPHENE_REFLECT_IMAP_VISIT_MEMBER_SET, TYPE, MEMBERS ) \
         default: \
            FC_ASSERT( false ); \
      } \
   } \
   \
   void check_imap_field<TYPE>( field_index_type field, uint64_t val ) \
   { \
      switch( field ) \
      { \
         BOOST_PP_SEQ_FOR_EACH_I( GRAPHENE_REFLECT_IMAP_VISIT_MEMBER_CHECK, TYPE, MEMBERS ) \
         default: \
            FC_ASSERT( false ); \
      } \
   } \
   \
   template<> \
   struct is_imap< TYPE > \
   { \
      typedef fc::true_type is_true; \
   }; \
   } }

} } // graphene::chain
