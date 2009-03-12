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


import Multiplexer_pb2
from Multiplexer_pb2 import *

def _get_from(self):
    return getattr(self, 'from')

def _set_from(self, value):
    return setattr(self, 'from', value)

def _del_from(self):
    delattr(self, 'from')

MultiplexerMessage.from_ = property(_get_from, _set_from, _del_from)
