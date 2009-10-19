
#include <iostream>
#include "azlib/logging.h"
#include "TasksHolder.h"

using namespace azlib::logging;

namespace mxcontrol {

    void TasksHolder::register_(const std::string& name, TaskProxy* task_proxy) throw() {
	if (named_tasks_.count(name) != 0) {
            // TODO(findepi) we can't call die() here, this involces logging and
            // logging module might have been not initialized yet -- we are
            // called from static initalizers
            // die("Task '" + name + "' already registered");
            std::cerr << "Error: Task '" << name << "' already registered " <<
                named_tasks_.count(name) << " time(s).\n";
            abort();
	}
	named_tasks_.insert(std::make_pair(name, task_proxy));
    }

    int TasksHolder::__run(TasksMap::iterator ti, std::vector<std::string>& args) {
	boost::shared_ptr<Task> task = ti->second->task();
	Assert(task);
	task->parse_options(args);
	return task->run();
    }

    namespace tasks_holder_detail {

	TasksHolder& tasks_holder() {
	    static TasksHolder* tasks_holder_ = NULL;
	    if (!tasks_holder_) {
		tasks_holder_ = new TasksHolder();
	    }
	    return *tasks_holder_;
	}
    };
};
