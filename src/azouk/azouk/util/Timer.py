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


from __future__ import with_statement

import time
from functools import wraps

from azouk.logging import *
from decorators import parametrizable_decorator

def _function_pretty_name(f, cls = None):
    nametokens = [f.__name__]
    if cls:
        nametokens.append(str(cls))
    else:
        if getattr(f, '__module__', False) and getattr(f.__module__, '__name__', False) and f.__module__.__name__ != "__main__":
            nametokens.append(f.__module__.__name__)
    nametokens.reverse()
    return ".".join(nametokens)


@parametrizable_decorator
def autotime(f, cls = None, silent = False):
    timernamepattern = "Timer for %s() #%%d" % _function_pretty_name(f, cls)
    callcounter = [0]
   
    @wraps(f)
    def wrapper(*args, **kwargs):
        timer = Timer(name = timernamepattern % callcounter[0], silent = silent)
        try:
            callcounter[0] += 1
            ret = f(*args, **kwargs)
        except:
            timer.report(after = "exception thrown")
            raise
        else:
            timer.report(after = "finish")
            pass
        return ret
    return wrapper

@parametrizable_decorator
def logtime(f, cls = None):
    functionname = _function_pretty_name(f, cls)
    callcounter = [0]

    @wraps(f)
    def wrapper(*args, **kwargs):
        timer = Timer(silent = True)
        r = f(*args, **kwargs)
        try:
            callcounter[0] += 1
            span = timer.fromstart()
            log(DEBUG, HIGHVERBOSITY, text=lambda:"Call #%d to %s took %.2f s" % (callcounter[0], functionname, span))
        except Exception:
            log_exception()
        return r
    return wrapper

class Timer(object):

    id = 0
    name = None
    start = None
    last = None

    def __init__(self, name = None, silent = False):
        self.id = Timer.id
        Timer.id += 1
        if name is None:
            self.name = "Timer #" + str(self.id)
        else:
            self.name = name

        self.restart()
        if not silent:
            print "%s: started @ %.3f" % (self.name, self.start)

    def restart(self):
        self.last = self.start = time.time()
    start = restart

    def timing(self):
        now = time.time()
        try:
            return (now - self.start, now - self.last)
        finally:
            self.last = now

    def fromstart(self):
        return self.timing()[0]

    def fromlast(self):
        return self.timing()[1]

    def report(self, after = None):
        times = self.timing()
        print "%s: %s%.3f (%.3f)" % (self.name, after and "after " + after + ": " or "", times[0], times[1])

    def __enter__(self):
        self.restart()

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.report(after = "finish")
        self.restart()

    autotime = staticmethod(autotime)
