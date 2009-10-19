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
import traceback
import functools

def _update_wrapper(wrapper, wrapped, *args, **kwargs):
    """Run functools.update_wrapper iff `wrapper` is not `wrapped`."""
    if wrapper is not wrapped:
        functools.update_wrapper(wrapper, wrapped, *args, **kwargs)
    return wrapper

def parametrizable_decorator(decorator):
    @functools.wraps(decorator)
    def wrapper(fn=None, *args, **kwargs):
        if fn is not None:
            return _update_wrapper(decorator(fn, *args, **kwargs), fn)
        else:
            return lambda fn: \
                    _update_wrapper(decorator(fn, *args, **kwargs), fn)
    return wrapper

@parametrizable_decorator
def post_eval_decorator(decorator):
    """A decorator making easier writing decorators that modify returned
       value."""
    @parametrizable_decorator
    def actual_decorator(fn):
        def wrapper(*args, **kwargs):
            value = fn(*args, **kwargs)
            return decorator(args, kwargs, value)
        return wrapper
    return actual_decorator

@parametrizable_decorator
def never_throw(fn, default = None):
    def wrapper(*args, **kwargs):
        try:
            return fn(*args, **kwargs)
        except Exception:
            traceback.print_stack()
            traceback.print_exc()
            print >> sys.stderr, "Ignored."
            return default
    return wrapper

class memoized_property(object):   # Copied from SqlAlchemy
    """A read-only @property that is only evaluated once."""
    def __init__(self, fget, doc=None):
        self.fget = fget
        self.__doc__ = doc or fget.__doc__
        self.__name__ = fget.__name__

    def __get__(self, obj, cls):
        if obj is None:
            return None
        obj.__dict__[self.__name__] = result = self.fget(obj)
        return result

def reset_memoized(instance, name):   # Copied from SqlAlchemy
    try:
        del instance.__dict__[name]
    except KeyError:
        pass


@parametrizable_decorator
def memoized(fn):
    cache = {}
    def memoized_wrapper(*args):
        if args not in cache:
            cache[args] = fn(*args)
        return cache[args]
    return memoized_wrapper

