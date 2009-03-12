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


#include "azlib/logging.h"
#include "TasksHolder.h"

using namespace azlib::logging;

namespace mxcontrol {

    void TasksHolder::register_(const std::string& name, TaskProxy* task_proxy) throw() {
	if (named_tasks_.count(name) != 0) {
	    die("Task '" + name + "' already registered");
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
