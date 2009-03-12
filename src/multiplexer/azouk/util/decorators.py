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
from functools import wraps

def parametrizable_decorator(decorator):
    @wraps(decorator)
    def wrapper(f = None, *args, **kwargs):
        if f is not None:
            return wraps(f)(decorator(f, *args, **kwargs))
        else:
            return lambda f: wraps(f)(decorator(f, *args, **kwargs))
    return wrapper

@parametrizable_decorator
def never_throw(f, default = None):
    def wrapper(*args, **kwargs):
        try:
            return f(*args, **kwargs)
        except Exception:
            traceback.print_stack()
            traceback.print_exc()
            print >> sys.stderr, "Ignored."
            return default
    return wrapper
