
#include <boost/python.hpp>
#include "azlib/util/type_utils.h"
#include "azlib/logging.h"

using namespace azlib;
namespace py = boost::python;

//BOOST_PYTHON_MODULE( _logging )
void init_module__logging() {
    using namespace py;

    def("should_log", &azlib::logging::impl::should_log);
    def("current_timestamp", &azlib::logging::impl::current_timestamp);
    def("emit_log", (void (*)(const azlib::logging::LogEntry&, unsigned int flags)) &azlib::logging::impl::emit_log);
    def("set_logging_file", &azlib::logging::set_logging_file);
    def("set_logging_fd", (void (*)(unsigned int, bool)) &azlib::logging::set_logging_fd);
    def("create_log_id", &azlib::logging::create_log_id);
    def("process_context", &azlib::logging::process_context, return_value_policy<copy_const_reference>());
    def("set_process_context", &azlib::logging::set_process_context);
    def("set_process_context_program_name", &azlib::logging::set_process_context_program_name);
    def("set_maximal_logging_verbosity", &azlib::logging::set_maximal_logging_verbosity);

#define export_constant_level_or_verbosity(r, d, tup) \
    scope().attr(BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 0, tup))) = BOOST_PP_TUPLE_ELEM(2, 1, tup);

    BOOST_PP_SEQ_FOR_EACH(export_constant_level_or_verbosity, ~, AZOUK_LOGGING_LEVELS_SEQ);
    BOOST_PP_SEQ_FOR_EACH(export_constant_level_or_verbosity, ~, AZOUK_LOGGING_VERBOSITIES_SEQ);

    using logging::LogEntry;
    class_<LogEntry>("LogEntry")
#define export_a_method(class, name) \
	.def(BOOST_PP_STRINGIZE(name), &class::name)
#define export_field_ctl(class, name) \
	export_a_method(class, BOOST_PP_CAT(has_, name)) \
	export_a_method(class, BOOST_PP_CAT(clear_, name))

#define export_num_field(r, class, name) \
	export_field_ctl(class, name) \
	.add_property(BOOST_PP_STRINGIZE(name), &class::name, &class::BOOST_PP_CAT(set_, name))

#define export_str_field(r, class, name) \
	export_field_ctl(class, name) \
	.add_property(BOOST_PP_STRINGIZE(name), \
		make_function(&class::name, return_value_policy<copy_const_reference>()), \
		make_function((void (class::*)(const std::string&)) &class::BOOST_PP_CAT(set_, name)))

	BOOST_PP_SEQ_FOR_EACH(export_num_field, LogEntry, (id) (timestamp) (level) (verbosity) (data_type) (source_line) (pid))
	BOOST_PP_SEQ_FOR_EACH(export_str_field, LogEntry, (context) (text) (workflow) (data_class) (data) (source_file) (compilation_datetime))
	;
}
