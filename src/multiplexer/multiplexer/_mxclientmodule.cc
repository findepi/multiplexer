//
// Azouk Libraries -- Libraries and goodies created for www.azouk.com.
// Copyright (C) 2008-2009 Azouk Network Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Author:
//      Piotr Findeisen <piotr.findeisen at gmail.com>
//


#include <boost/python.hpp>
#include "azlib/util/type_utils.h"
#include "multiplexer/Client.h"

using namespace std;
using namespace multiplexer;
namespace py = boost::python;

#define EXPORTED_EXCEPTIONS(callback) \
    callback(NotConnected) \
    callback(OperationTimedOut) \
    callback(OperationFailed) \
    // EXPORTED_EXCEPTIONS


namespace multiplexer {
    struct PythonClient : public Client {

    public:

	PythonClient(asio::io_service& io_service, boost::uint32_t client_type)
	    : Client(io_service, client_type)
	{
	}

	py::object read_message(float timeout) {
	    BOOST_STATIC_ASSERT((boost::is_same<BasicClient::IncomingMessagesBuffer::value_type::second_type, ConnectionWrapper>::value));
	    BasicClient::IncomingMessagesBuffer::value_type next = Client::read_raw_message(timeout);
	    py::tuple t = py::make_tuple((std::string) next.first->get_message(), next.second);

	    return t;
	}

	ScheduledMessageTracker schedule_one(std::string message) {
	    return Client::schedule_one(&message);
	}
	ScheduledMessageTracker schedule_one(std::string message, ConnectionWrapper w, float timeout) {
	    return Client::schedule_one(&message, w, timeout);
	}

	unsigned int schedule_all(std::string message) {
	    return Client::schedule_all(&message);
	}
	
    private:
    };
}; // namespace multiplexer

//template <typename T>
//void foo() {
    //cerr << __PRETTY_FUNCTION__ << "\n";
//}
// asio::io_service factory function
boost::shared_ptr<asio::io_service> create_new_asio_ioservice() {
    return boost::shared_ptr<asio::io_service>(new asio::io_service());
}

namespace _MxClientExceptionDefinitions {
    static ::boost::python::object MultiplexerClientError;
#define EXPORTED_EXCEPTION_TYPE(exc) static ::boost::python::object exc;
    EXPORTED_EXCEPTIONS(EXPORTED_EXCEPTION_TYPE)
#undef EXPORTED_EXCEPTION_TYPE
}; // namespace _MxClientExceptionDefinitions

// simple translator of multiplexer::Client::EXCEPTION to Python's _mxclient.EXCEPTION
struct ExceptionTranslator {
#define EXPORTED_EXCEPTION_HANDLER(exc) \
    void operator() (const multiplexer::Client::exc& /*e*/) const { \
	AZDEBUG_MSG("converting an exception of type " #exc "..."); \
	Assert(_MxClientExceptionDefinitions::exc.ptr()); \
	Assert(_MxClientExceptionDefinitions::exc.ptr() != Py_None); \
	PyErr_SetNone(_MxClientExceptionDefinitions::exc.ptr()); \
    }
    EXPORTED_EXCEPTIONS(EXPORTED_EXCEPTION_HANDLER);
#undef EXPORTED_EXCEPTION_HANDLER
};

static void test_connection_wrapper(multiplexer::ConnectionWrapper /*wrapper*/) {
}


//BOOST_PYTHON_MODULE( _mxclient )
void init_module__mxclient() {

    using namespace boost::python;
    using multiplexer::ConnectionWrapper;
    using multiplexer::BasicClient;
    using multiplexer::Client;
    using multiplexer::PythonClient;

    class_<Client::NotConnected>("Client_NotConnected")
	;

    // export the Exceptions
    // TODO py::borrowed???
    _MxClientExceptionDefinitions::MultiplexerClientError = py::object(py::borrowed(PyErr_NewException((char*) "_mxclient.MultiplexerClientError", NULL, NULL)));
    ExceptionTranslator translator;
#define create_exported_exception(exc) \
    _MxClientExceptionDefinitions::exc = py::object(py::borrowed( \
		PyErr_NewException((char*) "azouk._allinone." #exc, _MxClientExceptionDefinitions::MultiplexerClientError.ptr(), NULL) \
		)); \
    scope().attr(#exc) = _MxClientExceptionDefinitions::exc; \
    register_exception_translator<Client::exc>(translator);\
    //
    EXPORTED_EXCEPTIONS(create_exported_exception);
#undef create_exported_exception
    
    class_<boost::shared_ptr<asio::io_service> >("Asio_IoService")
	.def("__init__", make_constructor(create_new_asio_ioservice))
	.def("run",	(std::size_t (asio::io_service::*)()) &asio::io_service::run)
	.def("run_one", (std::size_t (asio::io_service::*)()) &asio::io_service::run_one)
	.def("stop",	(void (asio::io_service::*)()) &asio::io_service::stop)
	.def("reset",	(void (asio::io_service::*)()) &asio::io_service::reset)
	;

    typedef PythonClient::ScheduledMessageTracker ScheduledMessageTracker;
    class_<ScheduledMessageTracker>("ScheduledMessageTracker", no_init)
	.def("__nonzero__", &ScheduledMessageTracker::operator bool)
	.def("in_queue",    &ScheduledMessageTracker::in_queue)
	.def("is_sent",	    &ScheduledMessageTracker::is_sent)
	.def("is_lost",	    &ScheduledMessageTracker::is_lost)
	;

    class_<ConnectionWrapper>("ConnectionWrapper", no_init)
	.def("__nonzero__", &ConnectionWrapper::operator bool)
	;

    def("test_connection_wrapper", test_connection_wrapper);

    class_<PythonClient/*, boost::shared_ptr<PythonClient>*/ >(
	    "Client",
	    "Client object providing Multiplexer services for python.\n"
	    "Client.__init__(self, Asio_IoService, int) ->\n"
	    "    construct a new object bound to the given Asio_IoService\n"
	    "    of specified type\n",
	    init<asio::io_service&, boost::uint32_t>()
	    )
	
	.def("_get_instance_id",    &PythonClient::instance_id)

	.add_property("multiplexer_password",
		make_function(&PythonClient::multiplexer_password, return_value_policy<copy_const_reference>()),
		make_function((void (PythonClient::*)(const std::string&)) &PythonClient::set_multiplexer_password))

	.def("async_connect",	    (ConnectionWrapper (PythonClient::*) (const std::string&, boost::uint16_t)) &PythonClient::async_connect)
	.def("connect",		    (ConnectionWrapper (PythonClient::*) (const std::string&, boost::uint16_t, float)) &PythonClient::connect)
	.def("wait_for_connection", &PythonClient::wait_for_connection)
	.def("connections_count",   &PythonClient::connections_count)
	.def("shutdown",	    &PythonClient::shutdown)

	.def("read_message",	    &PythonClient::read_message)
	.def("schedule_one",	    (ScheduledMessageTracker (PythonClient::*) (std::string)) &PythonClient::schedule_one)
	.def("schedule_one",	    (ScheduledMessageTracker (PythonClient::*) (std::string, ConnectionWrapper, float)) &PythonClient::schedule_one)
	.def("schedule_all",	    &PythonClient::schedule_all)
	.def("flush",		    (void (PythonClient::*) (ScheduledMessageTracker, float)) &PythonClient::flush)
	.def("flush_all",	    &PythonClient::flush_all)

	.def("random",		    &PythonClient::random64)
	;

#define EXPORT_CONSTANT(name) (scope().attr(#name) = name)
    EXPORT_CONSTANT(DEFAULT_INCOMING_QUEUE_MAX_SIZE);
    EXPORT_CONSTANT(AUTO_RECONNECT_TIME);
    EXPORT_CONSTANT(DEFAULT_TIMEOUT);
    EXPORT_CONSTANT(MAX_MESSAGE_SIZE);

    EXPORT_CONSTANT(HEARTBIT_INTERVAL);
    EXPORT_CONSTANT(NO_HEARTBIT_SO_PREPARE_DROP_INTERVAL);
    EXPORT_CONSTANT(NO_HEARTBIT_SO_REALLY_DROP_INTERVAL);
#undef EXPORT_CONSTANT
}

