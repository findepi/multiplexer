#ifndef AZLIB_UTIL_FUNCTORS_H
#define AZLIB_UTIL_FUNCTORS_H

#include <functional>

namespace azlib {
    
    template <typename What, typename From>
    struct ConstructingFunctor : std::unary_function<From, What> {
	What operator() (From& f) const { return What(f); }
	What operator() (const From& f) const { return What(f); }
    };

    template <typename What>
    struct ReferencingFunctor : std::unary_function<What, What> {
	What& operator() (What& w) const { return w; }
	What operator() (const What& w) const { return w; }
    };

    template <typename What>
    struct DefaultConstructingFactory {
	What operator() () const { return What(); }
    };

    template <typename Pair>
    struct FirstFromPairExtractor : std::unary_function<Pair, typename Pair::first_type> {
	typename Pair::first_type& operator() (Pair& p) const { return p.first; }
	const typename Pair::first_type& operator() (const Pair& p) const { return p.first; }
    };

    template <typename Pair>
    struct SecondFromPairExtractor : std::unary_function<Pair, typename Pair::second_type> {
	typename Pair::second_type& operator() (Pair& p) const { return p.second; }
	const typename Pair::second_type& operator() (const Pair& p) const { return p.second; }
    };
};

#endif
