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
#include <fc/exception/exception.hpp>

namespace graphene { namespace net {
   // registered in node.cpp 
   
   FC_DECLARE_EXCEPTION( net_exception, 90000, "P2P Networking Exception" ); 
   FC_DECLARE_DERIVED_EXCEPTION( send_queue_overflow,                   graphene::net::net_exception, 90001, "send queue for this peer exceeded maximum size" ); 
   FC_DECLARE_DERIVED_EXCEPTION( insufficient_relay_fee,                graphene::net::net_exception, 90002, "insufficient relay fee" );
   FC_DECLARE_DERIVED_EXCEPTION( already_connected_to_requested_peer,   graphene::net::net_exception, 90003, "already connected to requested peer" );
   FC_DECLARE_DERIVED_EXCEPTION( block_older_than_undo_history,         graphene::net::net_exception, 90004, "block is older than our undo history allows us to process" );
   FC_DECLARE_DERIVED_EXCEPTION( peer_is_on_an_unreachable_fork,        graphene::net::net_exception, 90005, "peer is on another fork" );
   FC_DECLARE_DERIVED_EXCEPTION( unlinkable_block_exception,            graphene::net::net_exception, 90006, "unlinkable block" )

} }
