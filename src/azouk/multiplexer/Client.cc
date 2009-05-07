//
// Azouk Libraries -- Libraries and goodies created for www.azouk.com.
// Copyright (C) 2008-2009 Azouk Network Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Author:
//      Piotr Findeisen <piotr.findeisen at gmail.com>
//


#include "build/multiplexer/Multiplexer.pb.h"
#include "Client.h"

using namespace multiplexer;
using std::cerr;

Client::Client(boost::uint32_t client_type)
    : io_service_ptr_(new asio::io_service())
    , io_service_(*io_service_ptr_)
    , basic_client_(BasicClient::Create(io_service_, client_type))
{}

Client::Client(boost::shared_ptr<asio::io_service> io_service, boost::uint32_t
        client_type)
    : io_service_ptr_(io_service)
    , io_service_(*io_service_ptr_)
    , basic_client_(BasicClient::Create(io_service_, client_type))
{}

Client::Client(asio::io_service& io_service, boost::uint32_t client_type)
    : io_service_(io_service)
    , basic_client_(BasicClient::Create(io_service_, client_type))
{}

Client::~Client() {
    shutdown();
}


