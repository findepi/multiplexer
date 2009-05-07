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

#define IS_GENERATE_CONSTANTS 1

#include <iostream>
#include <fstream>
#include <azlib/hash_set.h>
#include <set>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "azlib/util/type_utils.h"
#include "multiplexer/Config.h"

using namespace std;

typedef multiplexer::Config<std::multimap> Config;

// TODO(findepi) implement AZOUK_DEFAULT_PROGRAM_LOGGING_LEVEL
//AZOUK_DEFAULT_PROGRAM_LOGGING_LEVEL(azlib::logging::INFO);

bool endswith(const std::string& str, const std::string& with) {
    if (str.size() < with.size())
	return false;
    for (unsigned int i = str.size() - with.size(); i != str.size(); ++ i)
	if (str[i] != with[i + with.size() - str.size()])
	    return false;
    return true;
}

template <typename ValueType>
struct RepeatedKeyException : azlib::Exception {
    RepeatedKeyException(const ValueType& v)
	: azlib::Exception("value of '" + boost::lexical_cast<std::string>(v) + "' repeats")
    {}
    RepeatedKeyException(const ValueType& v, const std::string& hint)
	: azlib::Exception("value of '" + boost::lexical_cast<std::string>(v) + "' repeats (" + hint + ")")
    {}
};

template <typename SetType, typename ValueType> void __set_checked_add(SetType& hs, const ValueType& v) {
    if (!hs.insert(v).second) MXTHROW(RepeatedKeyException<ValueType>(v));
}

template <typename SetType, typename ValueType> void __set_checked_add(SetType& hs, const ValueType& v, const std::string& hint) {
    if (!hs.insert(v).second) MXTHROW(RepeatedKeyException<ValueType>(v, hint));
}

template <typename Map>
void check_map_values_name_type_uniqueness(const Map& m) {
    using azlib::hash_set;
    std::set<std::string> names;
    hash_set<boost::uint32_t> ids;

    BOOST_FOREACH(const typename Map::value_type& v, m) {
	__set_checked_add(names, v.second.name());
	__set_checked_add(ids, v.second.type(), "somewhere about " + v.second.name());
    }
    Assert(m.size() == names.size());
    Assert(m.size() == ids.size());
}

void check_config(Config& config) {

    check_map_values_name_type_uniqueness(config.message_description_by_id());
    check_map_values_name_type_uniqueness(config.peer_by_type());
}

void write_signature(const char* comment, ostream& out, const std::string& source_file) {
    out
	<< "\n"
	<< comment << "\n"
	<< comment << " this file is generated from " << source_file << "\n"
	<< comment << " by a program from " << __FILE__ << " compiled " << __DATE__ " " __TIME__ << "\n"
	<< comment << "\n"
	<< "\n"
	;
}

template <typename Map>
void __write_python_name_to_type_mapping(ostream& out, const Map& map, const std::string& set_name) {
    out << "class " << set_name << ":\n";
    out << "\n";
    BOOST_FOREACH(const typename Map::value_type& m, map)
	out << "\t" << m.second.name() << " = " << m.second.type() << "\n";
    out << "\n";
    out << "\t" << "idtoname = {}\n";
    BOOST_FOREACH(const typename Map::value_type& m, map)
	out << "\t" << "idtoname[" << m.second.name() << "] = '" << m.second.name() << "'\n";
    out
	<< "\t" << "pass\n"
	<< "\n"
	;
}

int write_python(Config& config, ostream& out, const std::string& source_file) {
    write_signature("#", out, source_file);
    out
	<< "class _constants_base:\n"
	<< "\t" << "idtoname = None # dict defined by a subclass\n"
	<< "\t" << "@classmethod\n"
	<< "\t" << "def get_name(cls, type, default = 'UNKNOWN'):\n"
	<< "\t" << "\t" << "return cls.idtoname.get(type, default)\n"
	<< "\t" << "pass\n"
	<< "\n"
	;
    __write_python_name_to_type_mapping(out, config.message_description_by_id(), "types(_constants_base)");
    __write_python_name_to_type_mapping(out, config.peer_by_type(), "peers(_constants_base)");

    return 0;
}

std::string get_random_identifier(unsigned int l) {
    ifstream in("/dev/urandom");
    std::string r;
    while (r.size() < l) {
	char c;
	do {
	    in >> c;
	} while (!(
		    (c >= 'A' && c <= 'Z') |
		    (c >= 'a' && c <= 'z') |
		    (c >= '0' && c <= '9')
		));
	r += c;
    }
    return r;
}

template <typename Map>
void __write_cxx_name_to_type_mapping(ostream& out, const Map& map, const std::string& set_name) {
    const char* const PREF = "\t";

    out << PREF << "namespace " << set_name << " {\n";
    BOOST_FOREACH(const typename Map::value_type& m, map)
	out << PREF << "\t" << "static const boost::uint32_t " << m.second.name() << " = " << m.second.type() << ";\n";
    
    // generate get_name() function using great switch() statement
    out
	<< "\n"
	<< PREF << "\t" << "static inline const char* get_name(const boost::uint32_t t, const char* default_ = \"UNKNOWN\") {\n"
	<< PREF << "\t" << "\t" << "switch(t) {\n"
	;
    BOOST_FOREACH(const typename Map::value_type& m, map)
	out << PREF << "\t" << "\t" << "\t" << "case " << m.second.name() << ": return \"" << m.second.name() << "\";\n";
    out
	<< PREF << "\t" << "\t" << "\t" << "default: return default_;\n"
	<< PREF << "\t" << "\t" << "}\n"
	<< PREF << "\t" << "} // get_name\n"
	;
    out
	<< PREF << "} // namespace " << set_name << "\n"
	<< "\n"
	;
}

int write_cxx(Config& config, ostream& out, const std::string& source_file) {
    std::string r = get_random_identifier(10);
    out
	<< "#ifndef GENERATED_" << r << "\n"
	<< "#define GENERATED_" << r << "\n"
	<< "\n"
	<< "#include <boost/cstdint.hpp>\n"
	<< "\n"
	;

    write_signature("//", out, source_file);
    out
	<< "namespace multiplexer {\n"
	;
    __write_cxx_name_to_type_mapping(out, config.message_description_by_id(), "types");
    __write_cxx_name_to_type_mapping(out, config.peer_by_type(), "peers");
    
    out
	<< "}; // namespace multiplexer\n"
	<< "#endif\n"
	;
    return 0;
}

int AzoukMain(int argc, char** argv) {

    if (argc != 3) {
	std::cerr
	    << "Usage: " << argv[0] << " <multiplexer.rules file> (C++ header file | Python file)\n"
	    << "  Program generates definition of constants found in multiplexer.rules file.\n"
	    ;
	return 1;
    }


    enum OUTFILETYPE { PYTHON, CXX };
    OUTFILETYPE filetype;
    if (endswith(argv[2], ".h")) 
	filetype = CXX;
    else if (endswith(argv[2], ".py"))
	filetype = PYTHON;
    else {
	std::cerr << "Unknown file type that is " << argv[2] << "\n";
	return 2;
    }
    

    ofstream out(argv[2], ofstream::binary | ofstream::out);
    if (!out) {
	std::cerr << "Failed to open " << argv[2] << " for writing.\n";
	return 2;
    }

    Config config(argv[1]);
    
    check_config(config);

    switch (filetype) {
	case PYTHON:
	    return write_python(config, out, argv[1]);
	case CXX:
	    return write_cxx(config, out, argv[1]);
    }
    assert(false);
}

int main(int argc, char** argv) {
    using std::cerr;
    using std::endl;

    try {
        return AzoukMain(argc, argv);

    } catch (azlib::Exception& e) {
	cerr
	    << azlib::type_utils::type_name(e) << " in " << e.file() << ":" << e.line() << " (" << e.function() << ")\n"
	    << "    " << e.what() << endl;
        return 1;

    } catch (std::exception& e) {
	cerr
	    << azlib::type_utils::type_name(e) << ": " << e.what() << "\n";
        return 1;
    }
}

