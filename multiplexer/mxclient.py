
from azouk._allinone import *
import azouk._allinone as _mxclient
import socket, sys, struct, random, time
from functools import wraps
import google.protobuf.message
from google.protobuf import reflection
from types import MethodType

from multiplexer.protocolbuffers import *
from multiplexer.multiplexer_constants import peers, types

def set_if_missing(d, k, v):
    """assign key `k' in the dictionary `d' if it's not assigned"""
    if k not in d:
        d[k] = v

def make_message(*args, **kwargs):
    """
        make_message(MessageType, **kwargs) -> instance of MessageType with attributes set
        Create a message using factory function type and
        assign its attributes according do kwargs (attribute, value).
    """
    if len(args) != 1: raise ValueError
    m = args[0]()
    for (k, v) in kwargs.items():
        setattr(m, k, v)
    return m

def dict_message(m, all_fields = False, recursive = False):
    """
        Operation reverse to make_message.
        If all_fields == False, only set fields are provied.
    """
    d = {}
    if all_fields:
        for fd in m.DESCRIPTOR.fields:
            d[fd.name] = getattr(m, fd.name)
    else:
        for fd, value in m.ListFields():
            d[fd.name] = value

    if recursive:
        for k, v in d.items():
            if isinstance(v, google.protobuf.message.Message):
                d[k] = dict_message(v, all_fields = all_fields, recursive = True)

    return d

def parse_message(type, buffer):
    t = type()
    t.ParseFromString(buffer)
    return t

class TimeoutTicker(object):

    __slots__ = ['_expires_at', '_timeout']

    def __init__(self, timeout):
        object.__init__(self)
        self._timeout = timeout
        self._expires_at = time.time() + timeout

    def __call__(self):
        if self._timeout >= 0:
            return max(self._expires_at - time.time(), 0)
        else:
            return self._timeout

    def permit(self):
        return self() > 0 if self._timeout >= 0 else True

class Client(_mxclient.Client):

    DEFAULT_TIMEOUT = _mxclient.DEFAULT_TIMEOUT
    ONE = 0
    ALL = 1

    def __init__(self, client_type):
        """initialize a client with given client (peer) type"""
        self._io_service = _mxclient.Asio_IoService()
        self._client_type = client_type
        self.__instance_id = None
        super(Client, self).__init__(self._io_service, client_type)

    def __get_instance_id(self):
        if self.__instance_id is None:
            self.__instance_id = super(Client, self)._get_instance_id()
        return self.__instance_id

    instance_id = property(__get_instance_id)

    def async_connect(self, endpoint):
        """
            asynchronous connect
            endpoint is e.g. ("localhost", 1980)
            return ConnectionWrapper
        """
        ip4 = socket.gethostbyname(endpoint[0])
        return super(Client, self).async_connect(ip4, endpoint[1])

    def connect(self, endpoint, timeout = DEFAULT_TIMEOUT):
        """
            synchronous connect
            endpoint is e.g. ("localhost", 1980)
            return ConnectionWrapper
        """
        ip4 = socket.gethostbyname(endpoint[0])
        return super(Client, self).connect(ip4, endpoint[1], timeout)

    def wait_for_connection(self, connwrap, timeout = DEFAULT_TIMEOUT):
        """wait for connection initiated with async_connect"""
        return super(Client, self).wait_for_connection(connwrap, timeout)

    def __receive_message(self, timeout = -1):
        """
            blocking read from all the sockets (or from incoming message queue)
            returns (MultiplexerMessage, ConnectionWrapper)
        """
        next = super(Client, self).read_message(timeout)
        assert isinstance(next[0], str)
        mxmsg = parse_message(MultiplexerMessage, next[0])
        return (mxmsg, next[1])

    def receive_message(self, timeout = -1):
        return self.__receive_message(timeout)

    def handle_drop(self, mxmsg, connwrap = None):
        """overide in subclass if you want to get hold on every message dropped"""
        print >> sys.stderr, "dropping message %r" % dict(
                [ (k, getattr(mxmsg, k)) for k in ['id', 'type', 'to', 'from', 'references'] ] + \
                [ ('len', len(mxmsg.message)) ]
                )

    def _check_type(self, message, type, exc = OperationFailed):
        assert isinstance(message, MultiplexerMessage)
        if message.type != type:
            raise exc()

    def _unpack(self, message, type):
        return parse_message(type, message)

    def event(self, *args, **kwargs):
        """send message through all active MX connections (arguments same as for send_message)"""
        return self.send_message(multiplexer = Client.ALL, *args, **kwargs)

    def query(self, message, type, timeout = DEFAULT_TIMEOUT):
        """
            Reliably ask for the response.

            Send a request `message' with type `type' through one MX connection.
            If there is no response within `timeout' seconds, query all connected MX servers,
            to find working backend that will handle the request. If such a backend is found
            within `timeout' seconds, repeat the request directly to the working backend through
            the working MX connection.
        """
        assert not isinstance(message, MultiplexerMessage)
        query = self.new_message(type = type, message = message)
        ignore_function = lambda mxmsg, connwrap: mxmsg.type == types.REQUEST_RECEIVED
        first_request_delivery_errored = False
        try:
            response, connwrap = self.send_and_receive(query, multiplexer = Client.ONE, timeout = timeout, ignore_function = ignore_function)
            if response.type != types.DELIVERY_ERROR:
                return response
            first_request_delivery_errored = True
        except OperationTimedOut:
            pass

        # find working backends
        search = BackendForPacketSearch()
        search.packet_type = type
        mxmsg = self.new_message(message = search, type = types.BACKEND_FOR_PACKET_SEARCH)
        response, connwrap = self.send_and_receive(mxmsg, accept_ids = [query.id], multiplexer = Client.ALL, timeout = timeout,
                handle_delivery_errors = True, ignore_function = ignore_function)
        
        if response.type == types.DELIVERY_ERROR:
            if first_request_delivery_errored:
                raise OperationFailed
            return self.receive(accept_ids = [query.id], timeout = timeout)
        if response.references == query.id:
            return response

        self._check_type(response, types.PING)

        # resent original query directly to the working backend
        direct_query = self.new_message(to = response.from_, message = message, type = type)
        assert direct_query.type == type
        assert type
        response, connwrap = self.send_and_receive(direct_query, accept_ids = [query.id], ignore_ids = [mxmsg.id], multiplexer = connwrap,
                timeout = timeout, ignore_function = ignore_function)

        if response.type == types.DELIVERY_ERROR:
            raise OperationTimedOut

        return response

    def send_and_receive(self, message, accept_ids = [], ignore_ids = [], timeout = DEFAULT_TIMEOUT, handle_delivery_errors = False, ignore_function = None, **kwargs):
        """
            Unreliably ask for the response.

            Sends a request `message' and wait until `timeout' second pass or a response to message or one of accept_ids
            is received.
        """
        timeout_ticker = TimeoutTicker(timeout)
        id, tracker = self.send_message(message, timeout = timeout_ticker(), **kwargs)
        if isinstance(tracker, ScheduledMessageTracker):
            self.flush(tracker, timeout = timeout_ticker())
        else:
            assert isinstance(tracker, int)
            assert tracker >= 0

        accept_ids = [id] + accept_ids
        while timeout_ticker.permit():
            mxmsg, connwrap = self.receive(accept_ids = accept_ids, ignore_ids = ignore_ids, ignore_function = ignore_function, timeout_ticker = timeout_ticker)
            if mxmsg.type == types.DELIVERY_ERROR and handle_delivery_errors and isinstance(tracker, int):
                # event was sent and we don't want to stop on first DELIVERY_ERROR received
                tracker -= 1
                if tracker > 0:
                    continue
            return mxmsg, connwrap

        raise OperationTimedOut


    def receive(self, accept_ids, ignore_ids = [], ignore_function = None, timeout = DEFAULT_TIMEOUT, timeout_ticker = None):
        """
            like send_and_receive but without sending anything
        """
        if timeout_ticker is None: timeout_ticker = TimeoutTicker(timeout)
        if ignore_function is None: ignore_function = lambda *args: False

        while timeout_ticker.permit():
            mxmsg, connwrap = self.__receive_message(timeout = timeout_ticker())
            if ignore_function(mxmsg, connwrap):
                continue

            if mxmsg.references in accept_ids:
                return (mxmsg, connwrap)

            if mxmsg.references not in ignore_ids:
                # unexpected message received
                print >> sys.stderr, "other while waiting for reply for %r" % (accept_ids,)
                self.handle_drop(mxmsg, connwrap)

        raise OperationTimedOut()

    def send_message(self, message, **kwargs):
        """
            sends a message through one or more MX servers; usually non-blocking

            params
                message -- MultiplexerMessage or something that will become a message body (string, PorotocolBuffer Message)
                    (if embed is True, message will become final message's body without checking types)

            keyword-only params
                embed -- force creating new MultiplexerMessage, don't check `message' type
                multiplexer [ ONE | ALL | Connwrap ] -- use specific (set of) MX, default = ONE
                flush -- wait until message is passed to the socket layer (kernel)
                timeout -- timeout
                **kwargs -- will be used in construting MultiplexerMessage where `embed' or message is not instance of MultiplexerMessage
            returns (message_id, message_tracker)
        """
        return self.__send_message(message, **kwargs)

    def __send_message(self, message, embed=False, multiplexer=ONE, flush=False, timeout=DEFAULT_TIMEOUT, **kwargs):

        timeout_ticker = TimeoutTicker(timeout)

        if embed or not isinstance(message, MultiplexerMessage):
            mxmsg = self.new_message(message = message, **kwargs)
        else:
            mxmsg = message

        raw = mxmsg.SerializeToString()

        if multiplexer is Client.ONE:
            tracker = self.schedule_one(raw)
        elif multiplexer is Client.ALL:
            tracker = self.schedule_all(raw)
        elif isinstance(multiplexer, ConnectionWrapper):
            tracker = self.schedule_one(raw, multiplexer, timeout_ticker())
        else:
            raise NotImplementedError, "selecting multiplexer with %r is not supported" % multiplexer
       
        # handle flush
        if flush and tracker is not None:
            if multiplexer is Client.ALL:
                raise NotImplementedError, "Flushing when sending to multiple peers is not implemented."
            self.flush(tracker, timeout = timeout_ticker())

        return (mxmsg.id, tracker)

    def flush(self, tracker, timeout = DEFAULT_TIMEOUT):
        """ensure that message tracked by tracker is either sent or dropped"""
        super(Client, self).flush(tracker, timeout)

    def flush_all(self, timeout=DEFAULT_TIMEOUT):
        """try to empty all outgoing messages buffers within timeout seconds"""
        super(Client, self).flush_all(timeout)

    def read_message(self, *args, **kwargs):
        """shortcut for receiving a message and ingoring connection used"""
        return self.receive_message(*args, **kwargs)[0]

    # helper functions
    def random(self):
        """returns random uint64"""
        return super(Client, self).random()

    message_defaults = {}
    def new_message(self, **kwargs):
        """creates new MultiplexerMessage with some predefined values"""
        if self.message_defaults:
            kwargs = dict(self.message_defaults, **kwargs)

        # defaults
        set_if_missing(kwargs, 'id', self.random())
        set_if_missing(kwargs, 'from', self.instance_id)
        
        # special handling of some values
        if 'message' in kwargs and not isinstance(kwargs['message'], str):
            kwargs['message'] = kwargs['message'].SerializeToString()

        return make_message(MultiplexerMessage, **kwargs)
