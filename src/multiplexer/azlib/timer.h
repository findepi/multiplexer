#ifndef AZLIB_TIMER_H
#define AZLIB_TIMER_H

#include <asio/io_service.hpp>
#include <asio/deadline_timer.hpp>
#include <asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "azlib/util/Assert.h"
#include "azlib/util/str.h"
#include "azlib/util/IntrusiveValue.h"
#include "azlib/logging.h"

namespace azlib {

    struct SimpleTimer : boost::noncopyable {
	typedef boost::intrusive_ptr<IntrusiveValue<bool> > ExpiryHolder;

	/*
	 * create SimpleTimer that expires in `time' seconds
	 * `time' must be at least 0. If it's exactly 0, such a timer
	 * expires immediatelly.
	 */
	SimpleTimer(asio::io_service& io_service, float time)
            : timer_(io_service, boost::posix_time::microseconds(boost::numeric_cast<long>(time * 1e6)))
	    , expiry_holder_(new IntrusiveValue<bool>(time == 0))
	{
	    Assert(time >= 0);
	    Assert(time == 0 || !expired());
	    if (!expired()) {
		timer_.async_wait(
			boost::bind(&SimpleTimer::_expire, expiry_holder_, asio::placeholders::error)
			);
	    }
	}

	/*
	 * create a SimpleTimer that will never expire
	 */
	SimpleTimer(asio::io_service& io_service)
	    : timer_(io_service)
	    , expiry_holder_()
	{
	    Assert(!expired());
	}

	~SimpleTimer() {
	    // well... calling calnel here doesn't trigger _expire() immediatelly
	    // so we can't let expiry_holder_ be GCed here
	    timer_.cancel();
	}

	inline bool expired() const { return expiry_holder_ && *expiry_holder_; }

    private:
	static void _expire(ExpiryHolder expiry_holder, const asio::error_code& error) {
	    if (error != asio::error::operation_aborted) {
		*expiry_holder = true;
	    }
	}

    private:
	asio::deadline_timer timer_;
	ExpiryHolder expiry_holder_;
    };

}; // namespace azlib

#endif
