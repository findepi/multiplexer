#ifndef MX_ENCODERS_BASEDECODER_H
#define MX_ENCODERS_BASEDECODER_H

namespace azlib {
    namespace encoders {

	template <typename Base, template <typename> class Encoding>
	struct BaseDecoder {
	protected:
	    BaseDecoder() {}

	public:
	    template <typename T>
	    void operator() (T& t) {
		Encoding<T>::decode(*static_cast<Base*>(this), t);
	    }
	};

    };

};

#endif
