#!/usr/bin/env python

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

from multiplexer.protocolbuffers import *
from multiplexer.multiplexer_constants import peers, types
import multiplexer.mxclient as mxclient
from multiplexer.mxclient import *
import sys, os, struct, socket, time
from time import sleep

MULTIPLEXER_HOSTS = [ "localhost", ]
PORT = 1980

def parse(type, serialized):
    t = type()
    t.ParseFromString(serialized)
    assert t.IsInitialized()
    return t

def create_client(type):
    c = Client(type)
    cw = map(lambda host: c.async_connect((host, PORT)), MULTIPLEXER_HOSTS)
    c.wait_for_connection(cw[0])
    return c

def test():
    # helper programs
    MAIN = "[01;32m[KMAIN[m[K "
    CHILD = "[01;31m[KCHILD[m[K"
    if len(sys.argv) > 1:
        if sys.argv[1] == "SECOND_TO_QUERY":
            print CHILD, "running a child to support query testing"
            c = create_client(peers.PREDICT)
            print CHILD, "connected"
    
            assert c.connections_count() == 1

            while True:
                try:
                    #print CHILD, "waiting for messages", time.time()
                    mxmsg, connwrap = c.receive_message(timeout = 5)
                    #print CHILD, "wait finished"
                except mxclient.OperationTimedOut:
                    print CHILD, "OperationTimedOut", time.time()
                    break

                if mxmsg.type == types.PREDICT_REQUEST:
                    print CHILD, "sending a response to SEARCH_QUERY %r" % [(mxmsg.id, mxmsg.type, mxmsg.message)]
                    c.send_message(message = mxmsg.message.swapcase(), embed = True, to = mxmsg.from_, references = mxmsg.id, type = types.PREDICT_RESPONSE)
                elif mxmsg.type == types.BACKEND_FOR_PACKET_SEARCH:
                    print CHILD, "sending a response to query event %r" % [(mxmsg.id, mxmsg.type)]
                    c.send_message(message = "", embed = True, multiplexer = connwrap, flush = True,
                            type = types.PING, references = mxmsg.id, to = mxmsg.from_)
                
                else:
                    print CHILD, "dropping unsupported mxmsg %r" % [(mxmsg.id, mxmsg.type, mxmsg.message)]
            #

            print CHILD, "finishing"
            return
        raise RuntimeError, "sys.argv[1] == %r is not supported" % sys.argv[1]

    print MAIN, time.time()

    c = create_client(peers.SEARCH)
    print MAIN, "created a client #%d" % c.instance_id

    #try:
        #print MAIN, "receiving a message from not-yet-openned Client"
        #c.receive_message()
        #assert False, "receive_message() shouldn't work while we are not connected"
    #except mxclient.NotConnected:
        #print MAIN, "uh, good Client, good. Knows it's not connected anywhere!"
        #pass
    
    #print MAIN, "start the real work"
    #assert c.connections_count() == 0
    #cw = c.async_connect((MULTIPLEXER_HOST, 1980))
    #print MAIN, "testing convertion from python to ConnectionWrapper"
    #mxclient.test_connection_wrapper(cw)
    #assert c.wait_for_connection(cw), "otherwise we can't proceed"
    #assert c.connections_count() == 1
    #print MAIN, "connected"

    ## send a stupid search_query
    #query = "this is a search query with null (\x00) bytes and other " + "".join(chr(i) for i in range(256)) + " bytes"
    #print MAIN, "sending sample search query"
    #id, tracker = c.send_message(query, type = types.SEARCH_QUERY)
    #assert tracker
    #c.flush(tracker)
    #assert tracker and (tracker.is_sent() or tracker.is_lost())
    #assert tracker.is_sent(), "otherwise we can't proceed"
    #print MAIN, "waiting for sample search query"
    #msg = c.read_message()
    #print MAIN, "validating sample search query"
    #assert msg.id == id
    #assert msg.type == types.SEARCH_QUERY
    #assert msg.message == query
    #
    ## send a large search_query
    query = open("/dev/urandom", "r").read(1024 * 1024)
    #print MAIN, "sending large search query"
    #id, tracker = c.send_message(query, type = types.SEARCH_QUERY)
    #assert tracker
    #assert tracker.is_sent() or tracker.in_queue(), "otherwise we can't proceed"
    #print MAIN, "waiting for large search query"
    #msg = c.read_message()
    #print MAIN, "validating large search query"
    #assert msg.id == id
    #assert msg.type == types.SEARCH_QUERY
    #assert msg.message == query
    #assert tracker and tracker.is_sent(), "otherwise we wouldn't receive a response"

    # send a message to self by ID
    query = query[:1024] # make it shorter
    print MAIN, "sending a message to self"
    id, tracker = c.send_message(query, type = 317, to = c.instance_id)
    assert tracker
    msg = c.read_message()
    assert msg.id == id
    assert msg.type == 317
    assert msg.to == c.instance_id
    assert msg.from_ == c.instance_id
    assert msg.message == query
    assert tracker and tracker.is_sent(), "otherwise we wouldn't receive a response"

    # query a child replying to the first message
    for i in range(5): print
    print MAIN, "forking to test Client.query()"
    if os.fork() > 0:
        # parent
        pass
    else:
        os.execlp(sys.argv[0], sys.argv[0], "SECOND_TO_QUERY")
        os._exit(1)

    time.sleep(1)

    print MAIN, "querying SEARCHes"
    query = "ala ma dwa Koty i JEDNEGO PSA"
    #open("/dev/urandom", "r").read(1024)
    try:
        #for i in xrange(0, 100000):
        while True:
            response = c.query(query, type = types.PREDICT_REQUEST, timeout = 2)
            assert response.message == query.swapcase(), (response.message, query)
    except mxclient.OperationTimedOut:
        print MAIN, "OperationTimedOut"
        raise

    print MAIN, "wating for the child"
    #os.wait()
    c.shutdown()
    print MAIN, "finishing cleanly"
    print MAIN, time.time()
    sys.exit(0)

if __name__ == "__main__":
    test()

