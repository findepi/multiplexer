#ifndef AZLIB_TRIPLE_H
#define AZLIB_TRIPLE_H

namespace azlib {

    template <typename First, typename Second, typename Third>
    struct triple {
	typedef First first_type;
	typedef Second second_type;
	typedef Third third_type;

	triple(const first_type& f, const second_type& s, const third_type& t)
	    : first(f), second(s), third(t)
	{}

	template <typename FirstP, typename SecondP, typename ThirdP>
	triple(const triple<FirstP, SecondP, ThirdP>& other)
	    : first(other.first), second(other.second), third(other.third)
	{}

	triple()
	{}

	// TODO write assignments et al

	first_type first;
	second_type second;
	third_type third;
    };

    template <typename First, typename Second, typename Third>
    triple<First, Second, Third> make_triple(const First& f, const Second& s, const Third& t) {
	return triple<First, Second, Third>(f, s, t);
    }

}; // namespace azlib

#endif
