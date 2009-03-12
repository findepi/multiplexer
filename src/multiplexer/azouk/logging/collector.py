
import os
import sys
import collections
import time
from functools import partial

try:
    from django.utils import termcolors
except ImportError:
    import django_termcolors as termcolors

from azouk.logging import *
from azlib.logging.Logging_pb2 import LogEntry
from multiplexer.Multiplexer_pb2 import LogEntriesMessage, SearchCollectedLogs
from multiplexer.multiplexer_constants import peers, types
from multiplexer.clients import BaseMultiplexerServer, \
        get_connection
from multiplexer.mxclient import make_message, parse_message, OperationFailed

class MemoryLogCollector(BaseMultiplexerServer):

    # remember at last __memory_size LogEntries
    __memory_size = 1024 * 128

    def __init__(self, addresses):
        super(MemoryLogCollector, self).__init__(addresses=addresses,
                type=peers.LOG_COLLECTOR)

        self.__entries = collections.deque()
        self.__entriesIds = set()
        self.__byWorkflow = {}

    def __remove_entry(self):
        le = self.__entries.popleft()
        self.__entriesIds.remove(le.id)
        self.__byWorkflow[le.workflow].remove(le)

    def __add_entry(self, le):
        if le.id in self.__entriesIds:
            return

        while len(self.__entries) >= self.__memory_size:
            self.__remove_entry()

        self.__entries.append(le)
        self.__entriesIds.add(le.id)
        try:
            self.__byWorkflow[le.workflow].append(le)
        except KeyError:
            self.__byWorkflow[le.workflow] = [le]

    def __filter_log_entry(self, le, query):
        return \
                query in le.context or \
                query in le.text or \
                query in le.workflow or \
                query in le.source_file or \
                False

    @log_call
    def __search_log_entries(self, lem, q):
        """
            Filters LogEntries that match q.query.
            This may not modify `lem'.
        """
        if not q.query:
            return lem
        return filter(partial(self.__filter_log_entry, query=q.query), lem)

    @log_call
    def __slice_log_entries(self, lem, q):
        """
            Select LogEntries according to q.limit and q.offset.
            This may not modify `lem'.
        """
        if q.offset or len(lem) > q.limit:
            lem = list(lem)
            lem.reverse()
            lem = lem[q.offset:q.limit]
            lem.reverse()
        return lem

    @log_call
    def handle_message(self, mxmsg):
        if mxmsg.type == types.LOGS_STREAM:
            self.notify_start()
            lem = self.parse_message(LogEntriesMessage)
            lem = lem.log
            for le in lem:
                self.__add_entry(le)
            print "Received %d new LogEntries; currently remembered entries: %d (max %d)" % (len(lem),
                    len(self.__entries), self.__memory_size)
            self.no_response()

        elif mxmsg.type == types.SEARCH_COLLECTED_LOGS_REQUEST:
            q = self.parse_message(SearchCollectedLogs)
            print "---------------------------------------------- QUERY"
            print q

            if q._has_workflow:
                # select the results
                lem = self.__byWorkflow.get(q.workflow, [])

            else:
                lem = self.__entries

            lem = self.__search_log_entries(lem, q)
            lem = self.__slice_log_entries(lem, q)

            # construct the response
            res = LogEntriesMessage()
            for le in lem:
                res.log.add().CopyFrom(le)
            lem = None # allow GC
            self.send_message(message=res, type=types.SEARCH_COLLECTED_LOGS_RESPONSE)

            print "Sent %d LogEntries in reply" % len(res.log)
            print

        else:
            raise Exception, "unknown request type %d" % mxmsg.type

class Console(object):

    def Style():
        """constructs a helper object with coloring methods"""
        class Style(object):
            def __getattr__(self, attr):
                # By default every coloring method returns its argument
                # unmodified.
                return lambda x: x
            pass

        style = Style()

        if os.isatty(1):
            color_names = ('black', 'red', 'green', 'yellow', 'blue',
                    'magenta', 'cyan', 'white')
            for col in color_names:
                setattr(style, col, termcolors.make_style(fg=col))
                setattr(style, "bold_" + col,
                        termcolors.make_style(fg=col, opts=('bold',)))

        return style
    Style = Style()

    def __init__(self):
        self.__connection = get_connection()
        self.style = Console.Style

    def __text_color(self, level):
        if level >= WARNING:
            return self.style.red
        return self.style.green

    def print_(self, le, q=None):
        """console.print_(le, [q]) -> pretty print LogEntry `le' returned by query `q'"""
        print self.style.yellow("------------------------------------------------")
        print self.style.yellow("id:\t%d" % le.id)
        print self.style.yellow("time:\t%s" % time.ctime(le.timestamp))
        print "context: %s" % le.context
        print "level:\t%d" % le.level
        print "verbosity: %d" % le.verbosity
        print self.__text_color(le.level)("text:\t%s" % le.text)
        if le.workflow:
            print self.style.bold_blue("workflow: %s" % le.workflow)
        if le.pid:
            print "pid:\t%d" % le.pid

    def __print(self, les, q):
        """pretty print log entries"""
        for le in les:
            self.print_(le, q)
            print

    def search(self, prnt=False, print_=False, **kwargs):
        """
            console.search(prnt, print_, **kwargs)
                Construct SearchCollectedLogs message with kwargs (see Logging.proto for
                documentation) and perform search query to a log collector.

                If prnt or print_ then the logs are pretty printed instead of returning.
        """
        q = make_message(SearchCollectedLogs, **kwargs)
        mxmsg = self.__connection.query(q, type=types.SEARCH_COLLECTED_LOGS_REQUEST, timeout=3)
        if mxmsg.type != types.SEARCH_COLLECTED_LOGS_RESPONSE:
            # Beware: the following may or may not work; OperationFailed is a C-defined exception.
            raise OperationFailed, "invalid response of type %r received from backend" % mxmsg.type
        message = parse_message(LogEntriesMessage, mxmsg.message)
        del mxmsg
        les = list(message.log)
        del message

        if not print_ and not prnt:
            return l

        # let's print the results
        self.__print(les, q)

    def workflow(self, workflow, **kwargs):
        """
            console.workflow(workflow, **kwargs) -> console.search(workflow=workflow, **kwargs)
                (this is only handy wrapper for searching for specific workflow)
        """
        return self.search(workflow=workflow, **kwargs)
