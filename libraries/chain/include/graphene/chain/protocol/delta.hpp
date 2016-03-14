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

/**
 * Let T be an FC reflected type.
 * Then delta<T> is a type such that, for each field of T with type S,
 * delta<T> has a field with the same name of type optional<S>.
 *
 * delta<T>::apply_to(T) is a method which allows the present fields to be written into the type.
 */

#define GRAPHENE_REFLECT_DELTA_VISIT_MEMBER_DECLARE( r, TYPE, MEMBER ) \
   fc::optional< decltype( ((base_type*) nullptr)->MEMBER) ) > MEMBER;

#define GRAPHENE_REFLECT_DELTA_VISIT_MEMBER_APPLY( r, TYPE, MEMBER ) \
   if( MEMBER.valid() ) base.MEMBER = *MEMBER;

//
// We allow optionals to be default initialized by the delta(base) ctor,
// then overwrite them in the ctor body.  Otherwise this visitor macro would
// have to generate a comma-separated list with no trailing comma.
// Hopefully a sufficiently smart compiler will notice and optimize it out.
//
#define GRAPHENE_REFLECT_DELTA_VISIT_BASE_CTOR_INIT( r, TYPE, MEMBER ) \
   MEMBER = base.MEMBER;

#define GRAPHENE_REFLECT_DELTA( TYPE, MEMBERS ) \
    FC_REFLECT( TYPE, MEMBERS ) \
    namespace graphene { namespace chain { \
    template<> \
    struct delta<TYPE> \
    { \
       typedef TYPE base_type; \
       \
       delta() {} \
       delta( const delta<TYPE>& ) = default; \
       delta( const base_type& base ) \
       { \
          BOOST_PP_SEQ_FOR_EACH( GRAPHENE_REFLECT_DELTA_VISIT_MEMBER_BASE_CTOR_INIT, TYPE, MEMBERS ) \
       } \
       \
       BOOST_PP_SEQ_FOR_EACH( GRAPHENE_REFLECT_DELTA_VISIT_MEMBER_DECLARE, TYPE, MEMBERS ) \
       void apply_to( base_type& base )const \
       { \
          BOOST_PP_SEQ_FOR_EACH( GRAPHENE_REFLECT_DELTA_VISIT_MEMBER_APPLY, TYPE, MEMBERS ) \
       } \
    }; \
    } }

template< typename T >
struct delta
{
   static_assert( false, "attempted to use delta<T> without GRAPHENE_REFLECT_DELTA() macro" );
};
