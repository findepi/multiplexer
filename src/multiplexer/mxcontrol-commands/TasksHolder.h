#ifndef MXCONTROL_TASKSHOLDER_H
#define MXCONTROL_TASKSHOLDER_H

#include <iostream>
#include <string>
#include "mxcontrol-commands/Task.h"
#include "azlib/util/Assert.h"
#include "azlib/macros.h"
#include "azlib/initialization.h"

namespace mxcontrol {


    class TasksHolder {

    public:
	/*
	 * class representing a Task without instantiating it
	 */
	struct TaskProxy {
	    virtual ~TaskProxy() {}

	    // get the Task pointer by TaskProxy
	    virtual boost::shared_ptr<Task> operator()() = 0;

	    // alias
	    inline boost::shared_ptr<Task> task() { return (*this)(); }
	};

	TasksHolder()
	    : original_argc_(0)
	    , original_argv_(NULL)
	    , general_options("General options")
	{}

	typedef std::map<std::string, boost::shared_ptr<TaskProxy> > TasksMap;

	// takes ownership
	void register_(const std::string& name, TaskProxy* task_proxy) throw();

	bool inline is_command(const std::string& name) const { return named_tasks_.count(name); }
	const TasksMap& tasks() const { return named_tasks_; }

	template <typename ArgsVector>
	int run(ArgsVector& args) {
	    using namespace azlib;

	    TasksMap::iterator ti = named_tasks_.find(args.front());
	    if (named_tasks_.end() == ti) {
		std::cerr << "ERROR: unknown command: " << args.front() << "\n";
		return 1;
	    }
	    logging::set_process_context(logging::process_context() + "." + args.front());
	    args.pop_front();

	    std::vector<std::string> args_copy(args.begin(), args.end());
	    return __run(ti, args_copy);
	}

	TasksHolder& set_original_args(int argc, const char* const* argv) {
	    original_argc_ = argc;
	    original_argv_ = argv;
	    return *this;
	}

	int original_argc() { return original_argc_; }
	const char* const* original_argv() { return original_argv_; }

    private:
	int __run(TasksMap::iterator, std::vector<std::string>& args);

    private:
	TasksMap named_tasks_;
	int original_argc_;
	const char* const* original_argv_;
    public:
	boost::program_options::options_description general_options;
    };

    namespace tasks_holder_detail {
	// TasksHolder singleton
	TasksHolder& tasks_holder();

	template <typename subcommand>
	struct TaskProxyImpl : TasksHolder::TaskProxy {
	    virtual boost::shared_ptr<Task> operator()() {
		return boost::shared_ptr<subcommand>(new subcommand());
	    }
	};

    }; // namespace tasks_holder_detail

    using tasks_holder_detail::tasks_holder;

#define REGISTER_MXCONTROL_SUBCOMMAND(name, subcommand) \
    AZOUK_TRIGGER_STATIC_INITILIZATION_CODE(( \
		::mxcontrol::tasks_holder_detail::tasks_holder().register_(BOOST_PP_STRINGIZE(name), new ::mxcontrol::tasks_holder_detail::TaskProxyImpl<subcommand>()); \
	), true);

}; // namespace mxcontrol

#endif
