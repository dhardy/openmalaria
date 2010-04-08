/*
 This file is part of OpenMalaria.
 
 Copyright (C) 2005-2010 Swiss Tropical Institute and Liverpool School Of Tropical Medicine
 
 OpenMalaria is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.
 
 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

// This is a small file to separate out ESDecisionRandom's parsers.

#include "Clinical/parser.h"
#include "util/errors.hpp"

#include <sstream>
#include <boost/spirit/home/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/home/phoenix/operator/self.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

using namespace OM::util;
/** We use boost::spirit (2.1) for parsing here. A warning: debugging this
 * code when it won't compile is nearly impossible, so be very careful when
 * making changes!
 *************************************************************************/
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
using namespace boost::phoenix;

// We need to tell fusion about our Branch struct to make it a first-class
// fusion citizen. This has to be in global scope.
BOOST_FUSION_ADAPT_STRUCT(
    OM::Clinical::parser::Branch,
    (std::string, decision)
    (std::string, dec_value)
    (OM::Clinical::parser::Outcome, outcome)
)

namespace OM { namespace Clinical {
    typedef string::const_iterator iter_t;
    namespace parser {
	template <typename Iterator>
	struct list_grammar : qi::grammar<Iterator, SymbolList(), ascii::space_type> {
	    list_grammar() : list_grammar::base_type(list) {
		using qi::alnum;
		using qi::lexeme;
		
		symbol %= lexeme[ +( alnum | '.' | '_' ) ];
		list %= symbol % ',';
	    }
	    
	    qi::rule<Iterator, string(), ascii::space_type> symbol;
	    qi::rule<Iterator, SymbolList(), ascii::space_type> list;
	};
	
	template <typename Iterator>
	struct DR_grammar : qi::grammar<Iterator, Outcome(), ascii::space_type>
	{
	    DR_grammar() : DR_grammar::base_type(tree)
	    {
		using qi::alnum;
		using qi::lexeme;
		
		symbol %= lexeme[ +( alnum | '.' | '_' ) ] ;
		tree %= branches | symbol ;
		outcome %= ('{' > branches > '}') | (':' > symbol) ;
		branch %= symbol >> '(' > symbol > ')' > outcome ;
		branches %= +branch ;
	    }
	    
	    qi::rule<Iterator, string(), ascii::space_type> symbol;
	    qi::rule<Iterator, Outcome(), ascii::space_type> tree;
	    qi::rule<Iterator, Outcome(), ascii::space_type> outcome;
	    qi::rule<Iterator, Branch(), ascii::space_type> branch;
	    qi::rule<Iterator, Branches(), ascii::space_type> branches;
	};
	
	template <typename Iterator>
	struct SymbolValueMap_grammar : qi::grammar<Iterator, SymbolValueMap(), ascii::space_type> {
	    SymbolValueMap_grammar() : SymbolValueMap_grammar::base_type(map) {
		using qi::alnum;
		using qi::digit;
		using qi::lexeme;
		using qi::double_;
		using qi::_val;
		using qi::_1;
		using qi::_2;
		
		symbol %= lexeme[ +( alnum | '.' | '_' ) ];
		map = (symbol > '(' > double_ > ')')[_val[_1] = _2] % ',';
	    }
	    
	    qi::rule<Iterator, string(), ascii::space_type> symbol;
	    qi::rule<Iterator, SymbolValueMap(), ascii::space_type> map;
	};
	
	template <typename Iterator>
	struct SymbolRangeMap_grammar : qi::grammar<Iterator, SymbolRangeMap(), ascii::space_type> {
	    SymbolRangeMap_grammar() : SymbolRangeMap_grammar::base_type(map) {
		using qi::alnum;
		using qi::digit;
		using qi::lexeme;
		using qi::double_;
		using qi::_val;
		using qi::_1;
		using qi::_2;
		using qi::_3;
		
		symbol %= lexeme[ +( alnum | '.' | '_' ) ];
		map = (symbol > '(' > double_ > '-' > double_ > ')')[_val[_1] = construct< pair<double,double > >(_2,_3)] % ',';
	    }
	    
	    qi::rule<Iterator, string(), ascii::space_type> symbol;
	    qi::rule<Iterator, SymbolRangeMap(), ascii::space_type> map;
	};
    }
    
    parser::SymbolList parser::parseSymbolList (const string& s, const string& errObj) {
	parser::list_grammar<iter_t> list_rule;
	iter_t first = s.begin(); // we need a copy of the iterator, not a temporary
	SymbolList ret;
	
	qi::phrase_parse(
		first, s.end(),	// iterators
		list_rule,		// rule
		ascii::space,	// space skipper
		ret			// output (type must match rule's attribute type)
	);
	
	if (first != s.end ()) {
	    ostringstream msg;
	    msg
		<< "failed to parse comma-separated fields for " << errObj
		<< "; remainder: " << string(first,s.end())
	    ;
	    throw xml_scenario_error (msg.str());
	}
	
	return ret;
    }
    
    parser::Outcome parser::parseTree (const string& s, const string& errObj) {
	parser::DR_grammar<iter_t> tree_rule;
	iter_t first = s.begin();
	parser::Outcome tree;
	
	qi::phrase_parse(
		first, s.end(),	// iterators
		tree_rule,		// rule
		ascii::space,	// space skipper
		tree			// output (type must match rule's attribute type)
	);
	
	if (first != s.end ()) {
	    ostringstream msg;
	    msg
		<< "failed to parse tree for " << errObj
		<< "; remainder: " << string(first,s.end())
	    ;
	    throw xml_scenario_error (msg.str());
	}
	
	return tree;
    }
    
    parser::SymbolValueMap parser::parseSymbolValueMap (const string& s, const string& errObj) {
	parser::SymbolValueMap_grammar<iter_t> list_rule;
	iter_t first = s.begin(); // we need a copy of the iterator, not a temporary
	SymbolValueMap ret;
	
	qi::phrase_parse(
		first, s.end(),	// iterators
		list_rule,		// rule
		ascii::space,	// space skipper
		ret			// output (type must match rule's attribute type)
	);
	
	if (first != s.end ()) {
	    ostringstream msg;
	    msg
		<< "failed to parse comma-separated fields for " << errObj
		<< "; remainder: " << string(first,s.end())
	    ;
	    throw xml_scenario_error (msg.str());
	}
	
	return ret;
    }
    
    parser::SymbolRangeMap parser::parseSymbolRangeMap (const string& s, const string& errObj) {
	parser::SymbolRangeMap_grammar<iter_t> list_rule;
	iter_t first = s.begin(); // we need a copy of the iterator, not a temporary
	SymbolRangeMap ret;
	
	qi::phrase_parse(
		first, s.end(),	// iterators
		list_rule,		// rule
		ascii::space,	// space skipper
		ret			// output (type must match rule's attribute type)
	);
	
	if (first != s.end ()) {
	    ostringstream msg;
	    msg
		<< "failed to parse comma-separated fields for " << errObj
		<< "; remainder: " << string(first,s.end())
	    ;
	    throw xml_scenario_error (msg.str());
	}
	
	return ret;
    }
} }
