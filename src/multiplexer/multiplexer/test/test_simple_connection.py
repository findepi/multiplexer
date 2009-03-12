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

#!/usr/bin/env python

import socket, struct, sys, random, zlib, time
from build.multiplexer.Multiplexer_pb2 import *
from multiplexer_constants import peers, types

def randint():
    return random.randint(0, 2**30)

class Connection(object):

    socket = None
    instance_id = None

    def __init__(self, address):

        self.socket = socket.socket()
        self.socket.connect(address)
        self.instance_id = randint()


    def send_message(self, message, type):
        message = message.SerializeToString() if not isinstance(message, str) else message
        mxmsg = MultiplexerMessage()
        mxmsg.id = randint()
        setattr(mxmsg, 'from', self.instance_id)
        # mxmsg.set_to
        mxmsg.type = type
        mxmsg.message = message

        body = mxmsg.SerializeToString()
        print "sending %d bytes (%d meaningful)" % (len(body), len(message))
        header = struct.pack("Ii", len(body), zlib.crc32(body))
        self.socket.sendall(header + body)
        return mxmsg.id

    def _read_all(self, l):
        s = ""
        while len(s) < l:
            ch = self.socket.recv(l - len(s))
            if not ch:
                print >> sys.stderr, "Connection died unexpectedly"
                break
            s += ch
        assert not s or len(s) == l, (s, len(s), l)
        return s

    def receive_message(self):
        header = self._read_all(struct.calcsize("Ii"))
        if not header: return None
        length, crc = struct.unpack("Ii", header)
        body = self._read_all(length)
        assert body
        assert zlib.crc32(body) == crc, "Crc doesn't match"
        
        mxmsg = MultiplexerMessage()
        mxmsg.ParseFromString(body)
        assert mxmsg.IsInitialized()
        print "received %d bytes (%d meaningful)" % (len(body), len(mxmsg.message))
        return mxmsg

def parse(type, serialized):
    t = type()
    t.ParseFromString(serialized)
    assert t.IsInitialized()
    return t

def test():
    c = Connection(("localhost", 1980))

    # receive the invitation
    print "waiting for welcome message"
    msg = c.receive_message()
    print "validating welcome message"
    assert msg.type == types.CONNECTION_WELCOME
    welcome = parse(WelcomeMessage, msg.message)
    assert welcome.type == peers.MULTIPLEXER, "we have connected to Multiplexer"
    c.peer_id = welcome.id

    # send out invitation
    print "sending welcome message"
    welcome = WelcomeMessage()
    welcome.type = peers.SEARCH # FIXME
    welcome.id = c.instance_id
    c.send_message(welcome, types.CONNECTION_WELCOME)

    # send a stupid search_query
    query = "this is a search query with null (\x00) bytes and other " + "".join(chr(i) for i in range(256)) + " bytes"
    print "sending sample search query"
    id = c.send_message(query, types.SEARCH_QUERY)
    print "waiting for sample search query"
    msg = c.receive_message()
    print "validating sample search query"
    assert msg.id == id
    assert msg.type == types.SEARCH_QUERY
    assert msg.message == query
    
    # send a large search_query
    query = open("/dev/urandom", "r").read(1024 * 1024)
    print "sending large search query"
    id = c.send_message(query, types.SEARCH_QUERY)
    print "waiting for large search query"
    msg = c.receive_message()
    print "validating large search query"
    assert msg.id == id
    assert msg.type == types.SEARCH_QUERY
    assert msg.message == query

if __name__ == "__main__":
    test()

