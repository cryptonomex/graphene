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

#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/protocol.hpp>
#include <graphene/chain/protocol/operations.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/reflect/reflect.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace graphene { namespace fee_parameters_dump {

// we use specialization instead of plain overloading here,
// the reason being to specifically instantiate the template with the actual member type
// and ensure the compiler doesn't "helpfully" performing a widening integer conversion

template< typename T >
uint64_t convert_to_string( const T& t )
{
   std::cerr << "generic convert_to_string not allowed for type " << fc::get_typename<T>() << std::endl;
   exit(1);
}

using graphene::chain::share_type;

template<> uint64_t convert_to_string< uint8_t    >( const uint8_t&    x )   {  return x;   }
template<> uint64_t convert_to_string< uint16_t   >( const uint16_t&   x )   {  return x;   }
template<> uint64_t convert_to_string< uint32_t   >( const uint32_t&   x )   {  return x;   }
template<> uint64_t convert_to_string< uint64_t   >( const uint64_t&   x )   {  return x;   }
template<> uint64_t convert_to_string< share_type >( const share_type& x )
{
   FC_ASSERT( x.value >= 0 );
   return uint64_t( x.value );
}

struct context
{
   context( std::stringstream& _out, std::stringstream& _reflect_out ) : out(_out), reflect_out(_reflect_out) {}

   std::stringstream& out;
   std::stringstream& reflect_out;
};

struct dump_fee_member_visitor
{
   dump_fee_member_visitor( std::shared_ptr< context >& _ctx ) : ctx(_ctx) {}

   template<typename Member, class Class, Member (Class::*member)>
   void operator()( const char* name )const
   {
      std::string mtype = fc::get_typename<Member>::name();
      std::string mname = name;
      Class c;
      uint64_t default_amount_satoshis = convert_to_string< Member >( c.*member );
      FC_ASSERT( 0 == (default_amount_satoshis % GRAPHENE_BLOCKCHAIN_PRECISION) );
      uint64_t default_amount_units = default_amount_satoshis / GRAPHENE_BLOCKCHAIN_PRECISION;
      std::string str_amount = fc::to_string( default_amount_units );
      ctx->out << "   " << std::left << std::setw( 10 ) << mtype
        << " " << std::left << std::setw( 25 ) << mname
        << " = " << std::right << std::setw( 6 ) << default_amount_units
        << " * GRAPHENE_BLOCKCHAIN_PRECISION;\n";
      ctx->reflect_out << "(" << name << ")";
   }

   std::shared_ptr< context > ctx;
};

struct dump_fee_struct_visitor
{
   typedef void result_type;

   dump_fee_struct_visitor( std::shared_ptr< context >& _ctx ) : ctx(_ctx) {}

   template< typename Op > void operator()( const Op& op )
   {
      dump_fee_member_visitor vtor( ctx );
      std::string optype = fc::get_typename<Op>::name();

      ctx->reflect_out << "FC_REFLECT( " << optype << ", ";

      ctx->out << "template<>\n"
         "struct fee_parameters_type< " << optype << " >\n"
         "{\n";
      fc::reflector< typename Op::fee_parameters_type >::visit( vtor );
      ctx->out << "};\n\n";

      ctx->reflect_out << " )\n";
   }

   std::shared_ptr< context > ctx;
};

} }

void _main()
{
   std::stringstream out;
   std::stringstream reflect_out;
   std::shared_ptr< graphene::fee_parameters_dump::context > ctx =
      std::make_shared< graphene::fee_parameters_dump::context >( out, reflect_out );

   graphene::fee_parameters_dump::dump_fee_struct_visitor vtor(ctx);
   graphene::chain::operation op;

   for( int which=0; which<op.count(); which++ )
   {
      op.set_which( which );
      op.visit( vtor );
   }

   std::cout << out.str() << "\n" << reflect_out.str();
   return;
}

int main( int argc, char** argv, char** envp )
{
   try
   {
      _main();
   }
   catch( const fc::exception& e )
   {
      std::cerr << e.to_detail_string() << std::endl;
      return 1;
   }

   return 0;
}
