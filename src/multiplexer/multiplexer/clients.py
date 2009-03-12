"""Provide abstract multiplexer server functionality and client side
   utilities."""

import time
import copy
import cPickle as pickle
import sys
import functools
import traceback
import types
import warnings
from functools import wraps
from multiplexer import mxclient

from azouk.logging import *
from azouk.util import Timer
from multiplexer.multiplexer_constants import peers, types

# `settings' must be a module on your path.
try:
    from settings import MULTIPLEXER_ADDRESSES, MULTIPLEXER_PASSWORD
except ImportError:
    print >> sys.stderr, "Module `settings' not found. Using defaults."
    MULTIPLEXER_ADDRESSES = [('127.0.0.1', 1980)]
    MULTIPLEXER_PASSWORD = ''

from multiplexer.mxclient import set_if_missing, make_message, dict_message, \
        parse_message

class MultiplexerRelatedException(Exception):
    pass

# server side exceptions
class MultiplexerServerException(MultiplexerRelatedException):
    pass

class MustRestartServer(MultiplexerServerException):
    pass

# client side exceptions
class MultiplexerClientException(MultiplexerRelatedException):
    pass

class BackendError(MultiplexerClientException):
    pass


class BasicClient(mxclient.Client):

    # TODO consider inclusion of this functionality in mxclient.Client

    @log_call
    def receive_message(self, *args, **kwargs):
        """wrapper around mxclient.Client.send_and_receive"""
        mxmsg, connwrap = super(BasicClient, self).receive_message(
                *args, **kwargs)
        if mxmsg.type == types.BACKEND_ERROR:
            raise BackendError, mxmsg.message
        return mxmsg, connwrap

    @log_call
    def send_and_receive(self, *args, **kwargs):
        set_if_missing(kwargs, 'ignore_function',
                lambda mxmsg, connwrap: mxmsg.type == types.REQUEST_RECEIVED)
        return super(BasicClient, self).send_and_receive(*args, **kwargs)

class MultiplexerPeer(object):

    # what is the type of this multiplexer client (may be overriden in
    # __init__)
    multiplexer_client_type = None

    @log_call
    def __init__(self, addresses=MULTIPLEXER_ADDRESSES, type=None,
            password=MULTIPLEXER_PASSWORD):
        """
            Constructor.

            :Parameters:
                host
                    Host of multiplexer.
                addresses
                    list of (host, port) pairs
                type : multiplexer.multiplexer_constants.peers.* constant
                    Type of client to create
        """

        if type is None:
            type = self.multiplexer_client_type
            if type is None: raise ValueError, \
                    "no type provided and " \
                    "self.multiplexer_client_type is not set"
        self.type = type
        self.conn = BasicClient(self.type)
        self.conn.multiplexer_password = password
        for (host, port) in addresses:
            self.conn.connect((host, port))

class BaseMultiplexerServer(MultiplexerPeer):
    """Abstract multiplexer server functionality."""

    class ExceptionPolicy(object):
        @log_call
        def handle(self, server, e):
            raise NotImplementedError()

    class IgnoreExceptionPolicy(ExceptionPolicy):
        @log_call
        def handle(self, server, e):
            print >> sys.stderr, "Exception %r ignored" % e
            pass

    class TerminateOnExceptionPolicy(ExceptionPolicy):
        min_time_from_start = None
        def __init__(self, min_time_from_start = None):
            self.min_time_from_start = min_time_from_start

        @log_call
        def handle(self, server, e):
            if self.min_time_from_start is not None and \
                    time.time() - server.start_time < self.min_time_from_start:
                # restart forbidden
                log(WARNING, LOWVERBOSITY, text=lambda:"Ignoring exception "
                        "%s in %s as it's only %s from start; min is %s" %
                        (repr(e), repr(server),
                            repr(time.time() - server.start_time),
                            repr(self.min_time_from_start)))
                return
            log(INFO, LOWVERBOSITY, text=lambda:"Exception %r forced "
                    "server %r termination" % (e, server))
            raise MustRestartServer

    # time when the instance was initilized
    _start_time = None

    # what to do in case of an exception reported by a subclass through
    # exception_occurred()
    #
    # (exception_type, policy) list
    # policies are examined from freshest to oldest
    #_exceptions_policies = [ (Exception, IgnoreExceptionPolicy()) ]
    _exceptions_policies = []

    @log_call
    def __init__(self, addresses = MULTIPLEXER_ADDRESSES, type = None):
        """
            Constructor.
            See docstring for MultiplexerPeer.__init__ for meaning of
            parameters.
        """
        super(BaseMultiplexerServer, self).__init__(addresses, type)
        self.working = True
        self._start_time = time.time()
        self._exceptions_policies = copy.copy(self._exceptions_policies)

    start_time = property(lambda self: self._start_time,
            doc="Time when the instance was instantiated")

    @log_call
    def loop(self):
        """Serve forever."""
        while self.working:
            self.last_mxmsg, self.last_connwrap = self.conn.receive_message()
            self.__handle_message()

    serve_forever = loop

    @log_call
    def __handle_internal_message(self):
        mxmsg = self.last_mxmsg
        if mxmsg.type == types.BACKEND_FOR_PACKET_SEARCH:
            self.send_message(message="", embed=True, flush=True,
                    type=types.PING)

        elif mxmsg.type == types.PING:
            if not mxmsg.references:
                assert mxmsg.id
                self.send_message(message=mxmsg.message, embed=True,
                        flush=True, type = types.PING)
            else:
                self.no_response()

        else:
            log(ERROR, LOWVERBOSITY, text="received unknown meta-packet",
                    data=PickleData(mxmsg.type))

    @log_call
    def __handle_message(self):
        try:
            self._has_sent_response = False
            if self.last_mxmsg.type <= types.MAX_MULTIPLEXER_META_PACKET:
                # internal messages
                self.__handle_internal_message()
                if not self._has_sent_response:
                    log(WARNING, LOWVERBOSITY,
                            text="__handle_internal_message() finished w/o "
                                "exception and w/o any response")
            else:
                # the rest
                self.handle_message(self.last_mxmsg)
                if not self._has_sent_response:
                    log(WARNING, LOWVERBOSITY, text="handle_message() " \
                            "finished w/o exception and w/o any response")

        except Exception, e:
            # report exception
            traceback.print_exc()
            if not self._has_sent_response:
                log(DEBUG, MEDIUMVERBOSITY, text=lambda:"sending " \
                        "BACKEND_ERROR notification for Exception %s" % e)
                self.report_error(message = str(e))
            handled = self.exception_occurred(e)
            if not handled: raise

    @log_call
    def handle_message(self, mxmsg):
        """This method should be overriden in child classes."""
        raise NotImplementedError()

    def parse_message(self, type, mxmsg=None):
        """parse mxmsg.message with new Protobuf message of type `type'"""
        return parse_message(type,
                self.last_mxmsg.message if mxmsg is None else mxmsg.message)

    @log_call
    def notify_start(self):
        assert not self._has_sent_response, "If you use notify_start(), " \
                "place it as a first function in your handle_message() code"
        self.send_message(
                message = "",
                type = types.REQUEST_RECEIVED,
                # `references` would be set in send_message but make this
                # explicit
                references = self.last_mxmsg.id,
            )
        self._has_sent_response = False

    @log_call
    def send_message(self, **kwargs):
        assert self.last_mxmsg is not None
        self._has_sent_response = True
        set_if_missing(kwargs, 'multiplexer', self.last_connwrap)
        set_if_missing(kwargs, 'references', self.last_mxmsg.id)
        set_if_missing(kwargs, 'workflow', self.last_mxmsg.workflow)
        set_if_missing(kwargs, 'to', self.last_mxmsg.from_)
        # XXX set_if_missing(kwargs, 'flush', True) ?
        # performance penalty, but
        # more messages delivered in case process crashes soon after calling
        # send_message
        return self.conn.send_message(**kwargs)

    @log_call
    def no_response(self):
        self._has_sent_response = True

    @log_call
    def report_error(self, message="", type=types.BACKEND_ERROR, flush=True,
            **kwargs):
        self.send_message(message=message, type=type, flush=flush, **kwargs)

    @log_call
    def close(self):
        """In case we ever what to finish."""
        self.conn.shutdown()
        self.conn = None

    @log_call
    def exception_occurred(self, e):
        """invoke matching exception policy's handle; returns if matchin policy
           was found"""
        for (type, policy) in reversed(self._exceptions_policies):
            if isinstance(e, type):
                policy.handle(self, e)
                return True
        return False

    @log_call
    def set_exception_policy(self, type, policy):
        if isinstance(policy, types.TypeType) and \
                issubclass(policy, MultiplexerServer.ExceptionPolicy):
            policy = policy()
        assert isinstance(policy, MultiplexerServer.ExceptionPolicy), policy
        self._exceptions_policies.append((type, policy))

    @log_call
    def remove_exception_policy(self, type, policy = None, all = False):
        if all:
            removed = False
            while self.remove_exception_policy(type, policy, all = False):
                removed = True
            return removed

        if policy is not None:
            try:
                self._exceptions_policies.remove((type, policy))
                return True
            except ValueError:
                return False
        else:
            for (i, entry) in enumerate(self._exceptions_policies):
                if entry[0] == type:
                    del self._exceptions_policies[i]
                    return True
            return False

class MultiplexerServer(BaseMultiplexerServer):

    """
        Subclass of BaseMultiplexerServer
        using Python pickles as message payload.
    """

    @log_call
    def __init__(self, addresses=MULTIPLEXER_ADDRESSES, type=None):
        """
            Initialize the MultiplexerServer
            See docstring for MultiplexerPeer.__init__ for meaning of
            parameters.

            :Parameters:
                - `type`: either letter (backward compatible and obsolete) or a
                  constant
        """
        assert isinstance(type, (int, long))
        super(MultiplexerServer, self).__init__(addresses, type)

    @log_call
    def send_pickle(self, data):
        """Method for sending data back to multiplexer after processing it."""
        self.send_message(
                message = pickle.dumps(data),
                type = types.PICKLE_RESPONSE,
                flush = True,
            )

    @log_call
    def process_pickle(self, data):
        """This method should be overriden in child classes."""
        raise NotImplementedError


    @log_call
    def handle_message(self, mxmsg):
        try:
            data = pickle.loads(mxmsg.message)
        except (pickle.UnpicklingError, EOFError):
            print >> sys.stderr, "Failed to cPickle.load(%r) in #%d" % \
                    (mxmsg.message, mxmsg.id)
            raise
        return self.process_pickle(data)

class Client(BasicClient):

    @log_call
    def __init__(self, addresses=MULTIPLEXER_ADDRESSES, type=None,
            password=MULTIPLEXER_PASSWORD):
        # arguments are in this order to preserve compatibility
        if type is None:
            raise ValueError
        super(Client, self).__init__(type)
        self.multiplexer_password = password
        for host, port in addresses:
            self.connect((host, port))

def connect_client(*args, **kwargs):
    return Client(*args, **kwargs)


DEFAULT_PEER_TYPE = None
def connect(**kwargs):
    """Create MX Client with type DEFAULT_PEER_TYPE or any of type passed."""
    kwargs['type'] = kwargs.get('type', DEFAULT_PEER_TYPE)
    return connect_client(**kwargs)

_connection = None

def get_connection():
    """
        Return module's global connection.
        Create, if it does not exists yet.
    """
    global _connection
    if _connection is None:
        _connection = connect()
        assert _connection is not None
    return _connection

class MagicConnection(object):
    """Magic proxy for get_connection()."""
    def __getattr__(self, key):
        return getattr(get_connection(), key)

connection = MagicConnection()

def _limit_str(s, how=20):
    return s if len(s) <= how else s[:how - 4] + " ..."

@Timer.logtime
@log_call
def MultiplexerGet(data, type, timeout=15, warning_stacklevel=4):
    """
        Sends data to multiplexer (which will pass it based on information from
        type.

        :Parameters:
            data
                Any pickable data.
            type
                Which multiplexer server should process this data (peers.*)
    """

    assert isinstance(type, (int, long))

    request_type = type
    message = pickle.dumps(data)

    log(DEBUG, HIGHVERBOSITY, text=lambda:"sending MX-request type %r "
            "with timeout %r -- %s" % (type, timeout, _limit_str(repr(data), 60)))
    response = connection.query(message = message,
            type = type,
            timeout = timeout / 2.0
        )

    data = pickle.loads(response.message)
    if (isinstance(data, BackendError)):
        raise data
    log(DEBUG, HIGHVERBOSITY,
            text=lambda:"got MX-response type %r" % request_type)
    return (request_type, data)

def MultiplexerQuietGet(*args, **kwargs):
    try:
        return MultiplexerGet(warning_stacklevel=5, *args, **kwargs)
    except Exception:
        log_exception(text="Unhandled exception occurred when "
                "querying %r, %r; ingoring" % (args, kwargs))

