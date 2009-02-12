#ifndef MX_LIB_EXCEPTION_H
#define MX_LIB_EXCEPTION_H

#include <string>
#include <exception>

namespace azlib {

    struct Exception : public std::exception {

	    static bool abort_on_exception();
	    static void abort_on_exception(bool);

	    Exception() throw();
	    explicit Exception(const std::string explanation, const std::string& file = "<unknown file>", int line = 0,
		    const std::string& function = "<unknown function>") throw();
	    
	    virtual ~Exception() throw();
	    virtual const char* what() const throw();
	    //virtual std::string what() throw(); // hides std::exception::what() const

	    const std::string& file() const { return file_; }
	    const std::string& function() const { return function_; }
	    int line() const { return line_; }

	    void set_file(const std::string& file) { file_ = file; }
	    void set_line(const int line) { line_ = line; }
	    void set_function(const std::string& function) { function_ = function; }

	protected:
	    std::string explanation_;
	    std::string file_, function_;
	    int line_;
    };

    struct NotImplementedError : Exception {
    };

    namespace impl {
	template <typename T>
	T raise(T e, const char* file, int line, const char* function) {
	    e.set_file(file);
	    e.set_line(line);
	    e.set_function(function);
	    throw e;
	}
    };

#define MXTHROW(e...) (throw ::azlib::impl::raise((e), __FILE__, __LINE__, __PRETTY_FUNCTION__))

}; // namespace azlib

#ifdef BOOST_NO_EXCEPTIONS
#include <iostream>
namespace boost {
    template <typename T>
    void throw_exception(const T& t) {
	std::cerr << "gotta throw: " << t.what() << "\n";
	abort();
    }

    inline void throw_exception(const std::exception& t) {
	std::cerr << "gotta throw: " << t.what() << "\n";
	abort();
    }
}; // namespace boost
#endif

#endif
