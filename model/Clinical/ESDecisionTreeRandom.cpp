/*
 This file is part of OpenMalaria.
 
 Copyright (C) 2005-2009 Swiss Tropical Institute and Liverpool School Of Tropical Medicine
 
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

#include "Clinical/ESDecisionTree.h"
#include "util/errors.hpp"

#include <string>
#include <sstream>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

using namespace OM::util;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace OM { namespace Clinical {
    /** We use boost::spirit (2.1) for parsing here. A warning: debugging this
     * code when it won't compile is nearly impossible, so be very careful when
     * making changes!
     *************************************************************************/
    namespace parser {
	typedef std::vector<string> SymbolList;
	
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
	
	class Branch;
	typedef std::vector<Branch> Branches;
	typedef boost::variant< boost::recursive_wrapper< Branches >, string > Outcome;
	struct Branch {
	    string decision;
	    string dec_value;
	    Outcome outcome;
	};
    }
} }

// We need to tell fusion about our Branch struct to make it a first-class
// fusion citizen. This has to be in global scope.
BOOST_FUSION_ADAPT_STRUCT(
    OM::Clinical::parser::Branch,
    (std::string, decision)
    (std::string, dec_value)
    (OM::Clinical::parser::Outcome, outcome)
)

namespace OM { namespace Clinical {
    namespace parser {
	template <typename Iterator>
	struct DR_grammar : qi::grammar<Iterator, Outcome(), ascii::space_type>
	{
	    DR_grammar() : DR_grammar::base_type(tree)
	    {
		using qi::alnum;
		using qi::lexeme;
		
		symbol %= lexeme[ +( alnum | '.' | '_' ) ] ;
		tree %= branches | symbol ;
		outcome %= '{' > branches > '}' | ':' > symbol ;
		branch %= symbol >> '(' > symbol > ')' > outcome ;
		branches %= +branch ;
	    }
	    
	    qi::rule<Iterator, string(), ascii::space_type> symbol;
	    qi::rule<Iterator, Outcome(), ascii::space_type> tree;
	    qi::rule<Iterator, Outcome(), ascii::space_type> outcome;
	    qi::rule<Iterator, Branch(), ascii::space_type> branch;
	    qi::rule<Iterator, Branches(), ascii::space_type> branches;
	};
    }
    
    ESDecisionRandom::ESDecisionRandom (ESDecisionValueMap dvMap, const ::scnXml::Decision& xmlDc) {
	decision = xmlDc.getName();
	
	// Set depends, values. Start by defining a rule matching a symbol:
	typedef string::iterator iter_t;
	typedef parser::list_grammar<iter_t> list_grammar;
	list_grammar list_rule;
	
	
	string s = xmlDc.getDepends();
	iter_t first = s.begin(); // we need a copy of the iterator, not a temporary
	
	// Parse s into depends; note that the "attribute type" of the
	// expression must match the type of depends (a vector<string>):
	qi::phrase_parse(first, s.end(),
			    list_rule,
			    ascii::space,
			    depends);
	if (first != s.end ()) {
	    ostringstream msg;
	    msg << "ESDecision: failed to parse dependencies; remainder: " << string(first,s.end());
	    throw xml_scenario_error (msg.str());
	}
	
	
	vector<string> valueList;
	s = xmlDc.getValues();
	first = s.begin();
	
	// Same as above, for valueList:
	qi::phrase_parse(first, s.end(),
			    list_rule,
			    ascii::space,
			    valueList);
	if (first != s.end ()) {
	    ostringstream msg;
	    msg << "ESDecision: failed to parse values; remainder: " << string(first,s.end());
	    throw xml_scenario_error (msg.str());
	}
	
	setValues (dvMap, valueList);
	
	
	typedef parser::DR_grammar<iter_t> DR_grammar;
	DR_grammar tree_rule;
	parser::Outcome tree;
	
	const ::xml_schema::String *content_p = dynamic_cast< const ::xml_schema::String * > (&xmlDc);
	if (content_p == NULL)
	    throw runtime_error ("ESDecision: bad upcast?!");
	s = *content_p;
	cout << "Got content: "<<s<<endl;
	first = s.begin();
	
	// For now, we ignore output and just test it wil pass the tree
	qi::phrase_parse(first, s.end(),
			    tree_rule,
			    ascii::space,
			    tree);
	if (first != s.end ()) {
	    ostringstream msg;
	    msg << "ESDecision: failed to parse tree; remainder: " << string(first,s.end());
	    throw xml_scenario_error (msg.str());
	}
    }
    
// -----  CMNode derivatives  -----
/*
ESCaseManagement::CMPBranchSet::CMPBranchSet (const scnXml::CM_pBranchSet::CM_pBranchSequence& branchSeq) {
    double pAccumulation = 0.0;
    branches.resize (branchSeq.size());
    for (size_t i = 0; i < branchSeq.size(); ++i) {
	branches[i].outcome = branchSeq[i].getOutcome ();
	pAccumulation += branchSeq[i].getP ();
	branches[i].cumP = pAccumulation;
    }
    // Test cumP is approx. 1.0 (in case the XML is wrong).
    if (pAccumulation < 0.999 || pAccumulation > 1.001)
	throw util::xml_scenario_error ("EndPoint probabilities don't add up to 1.0 (CaseManagementTree)");
    // In any case, force it exactly 1.0 (because it could be slightly less,
    // meaning a random number x could have cumP<x<1.0, causing index errors.
    branches[branchSeq.size()-1].cumP = 1.0;
}*/

} }