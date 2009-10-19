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


import os
import socket

import azouk.logging

__all__ = ['enable_single_thread_log_streaming']

logging_fd_set_from_pid = None
logging_fd = None

def _spawn_streamer(multiplexer_addresses, multiplexer_password='',
        mxcontrol=None):
    global logging_fd_set_from_pid, logging_fd

    if logging_fd is not None:
        try:
            os.close(logging_fd)
        except Exception:
            pass

    if mxcontrol is None:
        mxcontrol = os.path.abspath(
                os.path.dirname(os.path.dirname(azouk.__file__)) +
                '/mxcontrol')

    logging_fd_set_from_pid = os.getpid()

    # Create a pipe that will become a binary logging stream.
    (reading, writing) = os.pipe()

    # Spawn a log streamer for this stream.
    if os.fork():
        # parent
        os.close(reading)
        azouk.logging.set_logging_fd(writing, True)
        logging_fd = writing

    else:
        # child
        os.close(writing)
        if reading != 0:
            os.dup2(reading, 0)
            os.close(reading)

        command = [ mxcontrol, "streamlogs" ] + \
                [ e for host, port in multiplexer_addresses
                        for e in ["--multiplexer", "%s:%d" %
                            (socket.gethostbyname(host), port) ]
                        ] + \
                [ "--chunksize", "16",
                    "--multiplexer-password", multiplexer_password
                ]
        os.execvp(command[0], command)

def enable_single_thread_log_streaming(multiplexer_addresses,
        multiplexer_password='', mxcontrol=None):

    if (logging_fd_set_from_pid is None or
            logging_fd_set_from_pid != os.getpid()):
        _spawn_streamer(multiplexer_addresses,
                multiplexer_password=multiplexer_password, mxcontrol=mxcontrol)

    return logging_fd
