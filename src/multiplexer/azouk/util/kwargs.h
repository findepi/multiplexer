#ifndef AZOUK_UTIL_KWARGS_H
#define AZOUK_UTIL_KWARGS_H

#include <map>
#include <string>
#include <set>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

#include "azlib/util/Exception.h"

namespace azouk {
    namespace util {
	namespace kwargs {

	    struct KeyError : azlib::Exception {
	    };

	    class Kwargs;
	    class KwargsKeys;

	    /**
	     * Kwargs
	     * 
	     * A C++ implementantion subsituting Python **kwargs
	     */
	    struct Kwargs {

		typedef std::map<std::string, boost::any> KwValuesMap;
		typedef KwValuesMap::value_type KwValue;

		Kwargs()
		    : __values(new KwValuesMap())
		{}

		template <typename T>
		Kwargs& set(const std::string& key, const T& value) {
		    (*__values)[key] = value;
		    return *this;
		}

		template <typename T>
		T get(const std::string& key, const T& default_) {
		    try {
			return get<T>(key);
		    } catch (const KeyError&) {
			return default_;
		    }
		}

		template <typename T>
		T get(const std::string& key) {
		    KwValuesMap::const_iterator pos = __values->find(key);
		    if (pos != __values->end()) {
			return __cast<T>(pos);
		    } else {
			throw KeyError();
		    }
		}
		
		template <typename T>
		T inline unsafe_get(const std::string& key) {
		    return __cast<T>(__values->find(key));
		}

		bool has_key(const std::string& key) {
		    KwValuesMap::const_iterator pos = __values->find(key);
		    return (pos != __values->end());
		}

		template <typename T>
		bool empty_or(const std::string& key) {
		    return !has_key(key) || unsafe_is<T>(key);
		}

		template <typename T>
		bool unsafe_is(const std::string& key) {
		    return __values->find(key)->second.type() == typeid(T);
		}

		template <typename T>
		Kwargs& set_default(const std::string& key, const T& value) {
		    __values->insert(KwValue(key, boost::any(value)));
		    return *this;
		}

		bool check_keys(const KwargsKeys& keys);

	    private:
		template <typename T>
		T inline __cast(KwValuesMap::const_iterator pos) {
		    return boost::any_cast<T>(pos->second);
		}

	    private:
		boost::shared_ptr<KwValuesMap> __values;
	    };

	    //struct KwargsKeys : boost::noncopyable {
	    struct KwargsKeys {
		KwargsKeys()
		{}

		KwargsKeys(const std::string& key) {
		    (*this)(key);
		}

		KwargsKeys& operator() (const std::string& key) {
		    __keys.insert(key);
		    return *this;
		}

	    private:
		std::set<std::string> __keys;
		friend class Kwargs;
	    };

	};
    };
};

#endif
