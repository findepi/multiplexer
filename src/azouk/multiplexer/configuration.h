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

#ifndef MULTIPLEXER_CONFIG_H
#define MULTIPLEXER_CONFIG_H

namespace multiplexer {

    static const unsigned int DEFAULT_INCOMING_QUEUE_MAX_SIZE = 1024;
    static const unsigned int AUTO_RECONNECT_TIME = 3;
    static const float DEFAULT_TIMEOUT = 10.0;
    static const float DEFAULT_READ_TIMEOUT = -1; // "< 0" means inifinity
    static const unsigned int MAX_MESSAGE_SIZE = 128 * 1024 * 1024;

    static const float HEARTBIT_INTERVAL = 3.0;
    static const float NO_HEARTBIT_SO_PREPARE_DROP_INTERVAL = 5 * 60;
    static const float NO_HEARTBIT_SO_REALLY_DROP_INTERVAL = 10;

}; // namespace multiplexer

#endif
