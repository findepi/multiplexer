#ifndef MX_ENCODERS_BASEENCODER_H
#define MX_ENCODERS_BASEENCODER_H

namespace azlib {
    namespace encoders {

	template <typename Base, template <typename> class Encoding>
	struct BaseEncoder {
	protected:
	    BaseEncoder() {}

	public:
	    template <typename T>
	    void operator() (const T& t) {
		Encoding<T>::encode(*static_cast<Base*>(this), t);
	    }
	};

    };

};

#endif
