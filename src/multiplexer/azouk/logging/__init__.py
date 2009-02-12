
import sys
import os
import random
import cPickle as pickle
import traceback
from time import time
from types import NoneType
from functools import wraps
from azouk.util.decorators import parametrizable_decorator, never_throw

from azouk._allinone import *
from type_id_constants import *
import azouk._allinone as _logging

__all__ = [
        'should_log', 'log',
        'log_call', 'log_exception',
        'PickleData',
    ] + list(k for k in dir(_logging) if isinstance(k, str) and k.isupper())

@never_throw(default=False)
def should_log(level, verbosity):
    assert isinstance(level, (int, long))
    assert isinstance(verbosity, (int, long))
    return _logging.should_log(level, verbosity)

def log(level, verbosity, *args, **kwarg):
    assert isinstance(level, (int, long))
    assert isinstance(verbosity, (int, long))
    if not should_log(level, verbosity):
        return
    return do_log(level, verbosity, *args, **kwarg)

log_defaults = {}
@never_throw
def do_log(level, verbosity, **kwargs):
    if log_defaults:
        kwargs = dict(log_defaults, **kwargs)
    return _do_log(level, verbosity, **kwargs)

def _do_log(level, verbosity, ctx=None, context=None, text=None, data=None,
        data_type=None, flow=None, flags=0):
    assert isinstance(level, (int, long))
    assert isinstance(verbosity, (int, long))
    assert ctx is None or context is None
    assert isinstance(ctx, (NoneType, str))
    assert isinstance(context, (str, NoneType))
    assert isinstance(text, (NoneType, str)) or callable(text)
    # data can be anything for now
    assert isinstance(data_type, (NoneType, int, long))
    assert isinstance(flow, (NoneType, str))

    log_msg = _logging.LogEntry()

    # id, timestamp
    log_msg.id = _logging.create_log_id()
    log_msg.timestamp = int(time())

    # context
    if context is None:
        context = _logging.process_context()
        if ctx is not None:
            context += "." + ctx
    log_msg.context = context

    # level, verbosity
    log_msg.level = level
    log_msg.verbosity = verbosity

    # text
    if callable(text):
        text = text()
    assert text is None or isinstance(text, str)
    if text is not None:
        log_msg.text = text

    # data, data_type
    if callable(data): data = data()
    if data is not None and data_type is None:
        if isinstance(data, PickleData):
            data_type = PYTHON_PICKLE
        else:
            raise ValueError, "data_type is missing and data is not"

    if data is not None and not isinstance(data, str):
        if isinstance(data, PickleData):
            data = data.pickle
        else:
            raise ValueError, "data must be string or some known type"

    assert data is None or isinstance(data, str)
    if data is not None: log_msg.data = data
    if data_type is not None: log_msg.data_type = data_type

    # data_class ?

    # workflow
    if flow is not None:
        log_msg.workflow = flow

    # pid
    log_msg.pid = os.getpid()

    # TODO(findepi): think if this can be retrieved (stack analysis; see
    #       python-gflags code for example)
    # source_file
    # source_line
    # compilation_datetime

    _logging.emit_log(log_msg, flags)

@parametrizable_decorator
def log_call(f, level=DEBUG, verbosity=CHATTERBOX):
    assert isinstance(level, (int, long))
    assert isinstance(verbosity, (int, long))
    assert callable(f)

    callidmax = sys.maxint
    def wrapper(*args, **kwargs):
        if not should_log(level, verbosity):
            return f(*args, **kwargs)

        logsig = {'fname' : f.__name__, 'callid': random.randint(1, callidmax)}
        do_log(level, verbosity, text="entering %s(args= %r, kwargs= %r)" % (f.__name__, args, kwargs), data=PickleData(dict(logsig, args=args, kwargs=kwargs)))
        try:
            ret = f(*args, **kwargs)
        except Exception, exc:
            do_log(level, verbosity, text="leaving %s after exception %r" % (f.__name__, exc), data=PickleData(dict(logsig, exc=exc)))
            raise
        else:
            do_log(level, verbosity, text="leaving %s with %r" % (f.__name__, ret), data=PickleData(dict(logsig, ret=ret)))
            return ret

    return wrapper

def log_exception(level=ERROR, verbosity=LOWVERBOSITY, text="Unhandled exception occurred", **kwargs):
    traceback.print_stack()
    traceback.print_exc()
    kwargs = dict({'data': lambda: PickleData(sys.exc_info()[:2])}, **kwargs)
    log(level, verbosity, text=text, **kwargs)

class PickleData(object):
    __slots__ = ('_pkl', '_obj')
    def __init__(self, obj):
        self._obj = obj
        self._pkl = None

    @property
    def pickle(self):
        if self._pkl is None:
            try:
                self._pkl = pickle.dumps(self._obj)
            except TypeError:
                # C++-exported classes are (at least sometimes) not picklable
                self._pkl = repr(self._obj)
            assert self._pkl is not None

            self._obj = None
        return self._pkl

