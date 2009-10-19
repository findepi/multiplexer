#
# Azouk Libraries -- Libraries and goodies created for www.azouk.com.
# Copyright (C) 2008-2009 Azouk Network Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author:
#      Piotr Findeisen <piotr.findeisen at gmail.com>
#


import sys
import time
import collections

from azouk.logging import *
from multiplexer import mxclient
from multiplexer.Multiplexer_pb2 import ReplayCollectedEvents
from multiplexer.multiplexer_constants import peers, types
from multiplexer.clients import BaseMultiplexerServer

class EventsCollectorServer(BaseMultiplexerServer):

    # remember at last __memory_size events
    __memory_size = 1024 * 128

    def __init__(self, addresses):
        super(EventsCollectorServer, self).__init__(addresses=addresses,
                type=peers.EVENTS_COLLECTOR)

        # all events from oldest to newest
        self.__events = collections.deque()

        # events by type and by id like this:
        # { type: deque }
        self.__eventsByType = {}

    def __remove_event(self):
        mxmsg = self.__events.popleft()
        ts, mxmsg_by_type = self.__eventsByType[mxmsg.type].popleft()
        assert mxmsg is mxmsg_by_type

    def __add_event(self, mxmsg):
        while len(self.__events) >= self.__memory_size:
            self.__remove_event()

        self.__events.append(mxmsg)
        try:
            events_for_type = self.__eventsByType[mxmsg.type]
        except KeyError:
            self.__eventsByType[mxmsg.type] = events_for_type = \
                    collections.deque()
        events_for_type.append((time.time(), mxmsg))

    def __select_events(self, query):
        """Returns an iterator over events matching query."""
        event_types = query.event_type or self.__eventsByType.iterkeys()
        from_timestamp = query.from_timestamp
        assert query,_has_to_timestamp or not query.to_timestamp
        to_timestamp = query.to_timestamp or time.time()

        if to_timestamp < from_timestamp:
            return
        for type in event_types:
            # TODO(findepi) select events using bisection
            for (ts, mxmsg) in self.__eventsByType.get(type, ()):
                if ts < from_timestamp:
                    continue
                elif ts > to_timestamp:
                    break
                else:
                    yield mxmsg

    def __replay_events(self, mxmsg, query):
        assert mxmsg.from_
        i = 0
        for event in self.__select_events(query):
            event.to = mxmsg.from_
            print event
            self.conn.send_message(message=event, multiplexer=self.conn.ALL)
            i += 1
        return i

    @log_call
    def handle_message(self, mxmsg):
        if mxmsg.type == types.REPLAY_EVENTS_REQUEST:
            print "----------------------------------------------------------------------"
            query = mxclient.parse_message(
                ReplayCollectedEvents, mxmsg.message)
            print mxmsg
            print query
            count = self.__replay_events(mxmsg, query)
            print "Replied with %d events." % count
            self.no_response()
        else:
            self.__add_event(mxmsg)
            self.no_response()
            print ("Received an event type=%-4d id=%d;\t " +
                    "remembered entries: %d (max %d)") % \
                    (mxmsg.type, mxmsg.id, len(self.__events), self.__memory_size)
            print mxmsg

