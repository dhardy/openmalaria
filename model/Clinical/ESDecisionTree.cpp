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

#include "Clinical/ESDecisionTree.h"
#include "util/random.h"
#include "util/errors.hpp"

#include <set>
#include <boost/format.hpp>

namespace OM { namespace Clinical {
    using namespace OM::util;
    
    void ESDecisionTree::setValues (ESDecisionValueMap& dvMap, const vector< string >& valueList) {
	mask = dvMap.add_decision_values (decision, valueList);
	values.resize (valueList.size()+1);
	values[0] = ESDecisionValue();	// "void" option
	for( size_t i = 0; i < valueList.size(); ++i ) {
	    values[i+1] = dvMap.get (decision, valueList[i]);
	}
    }
    
    ESDecisionValue ESDecisionRandom::determine (const ESDecisionValue input, ESHostData& hostData) const {
	map_cum_p_t::const_iterator it = map_cum_p.find (input);
	if (it == map_cum_p.end())	// should only happen when "void" was specified as an output
	    return ESDecisionValue();	// no decision
	double sample = random::uniform_01 ();
	size_t i = 0;
	while (it->second[i] < sample)
	    ++i;
	return values[i];
    }
    
    ESDecisionUC2Test::ESDecisionUC2Test (ESDecisionValueMap& dvMap) {
	decision = "case";
	vector< string > valueList (2, "UC1");
	valueList[1] = "UC2";
	setValues (dvMap, valueList);
    }
    ESDecisionValue ESDecisionUC2Test::determine (const ESDecisionValue, ESHostData& hostData) const {
	assert (hostData.pgState & Pathogenesis::SICK && !(hostData.pgState & Pathogenesis::COMPLICATED));
	if (hostData.pgState & Pathogenesis::SECOND_CASE)	//TODO: check this actually gets set
	    return values[1];
	else
	    return values[0];
    }
    
    ESDecisionAge5Test::ESDecisionAge5Test (ESDecisionValueMap& dvMap) {
	decision = "age5Test";
	vector< string > valueList (2, "under5");
	valueList[1] = "over5";
	setValues (dvMap, valueList);
    }
    ESDecisionValue ESDecisionAge5Test::determine (const ESDecisionValue input, ESHostData& hostData) const {
	if (hostData.ageYears >= 5.0)
	    return values[1];
	else
	    return values[0];
    }
    
    ESDecisionParasiteTest::ESDecisionParasiteTest (ESDecisionValueMap& dvMap) {
	decision = "result";
	
	string vs[] = { "none", "microscopy", "RDT" };
	// Add values, which (1) lets us create test_none, etc., and (2) introduces a
	// check when the "test" decision is added later.
	dvMap.add_decision_values ("test", vector<string>(vs, vs+3));
	test_none = dvMap.get("test","none");
	test_microscopy = dvMap.get("test", "microscopy");
	test_RDT = dvMap.get("test","RDT");
	depends.resize (1, "test");	// Note: we check in add_decision_values() that this test has the outcomes we expect
	
	vector< string > valueList (2, "negative");
	valueList[1] = "positive";
	setValues (dvMap, valueList);
    }
    
    ESDecisionValue ESDecisionParasiteTest::determine (const ESDecisionValue input, ESHostData& hostData) const {
	if (input == test_microscopy) {
	    // if (hostData.withinHost.getDensity () > ...)
	    //FIXME: or values[0]
	    return values[1];
	} else if (input == test_RDT) {
	    //FIXME: or values[0]
	    return values[0];
	} else	// if output was void
	    return ESDecisionValue();	// 0, no decision
    }
    

std::size_t hash_value(ESDecisionValue const& b) {
    boost::hash<int> hasher;
    return hasher(b.id);
}
ESDecisionValue ESDecisionValueMap::add_decision_values (const string& decision, const std::vector< string > values) {
    pair<id_map_type::iterator,bool> dec_pair = id_map.insert( make_pair (decision, make_pair ( ESDecisionValue(), value_map_t() ) ) );
    value_map_t& valMap = dec_pair.first->second.second;	// alias new map
    if (dec_pair.second) {	// new entry; fill it
	
	// got length l = values.size() + 1 (default, "no outcome"); want minimal n such that: 2^n >= l
	// that is, n >= log_2 (l)
	// so n = ceil (log_2 (l))
	uint32_t n_bits = std::ceil (log (values.size() + 1) / log(2.0));
	if (n_bits+next_bit>=(sizeof(next_bit)*8))	// (only valid on 8-bit-per-byte architectures)
	    throw runtime_error ("ESDecisionValue design: insufficient bits");
	
	// Now we've got enough bits to represent all outcomes, starting at next_bit
	// Zero always means "missing value", so text starts at our first non-zero value:
	id_type next=(1<<next_bit), step;
	step=next;
	BOOST_FOREACH ( const string& value, values ) {
	    if( value == "void" )
		throw xml_scenario_error( "void can not be a declared output of a decision" );
// 	    cout<<"ESDecisionValue: "<<decision<<'('<<value<<"): "<<next<<endl;
	    valMap[value] = ESDecisionValue(next);
	    next += step;
	}
	next_bit += n_bits;
	assert (next <= (1u<<next_bit));
	
	// Set mask so bits which are used by values are 1:
	ESDecisionValue mask;
	for (value_map_t::const_iterator cur_val = valMap.begin(); cur_val != valMap.end(); ++cur_val)
	mask |= cur_val->second;
// 	cout<<"Mask for "<<decision<<": "<<mask<<endl;
	dec_pair.first->second.first = mask;
	
    } else {	// decision already exists; confirm values match
	
	set<string> new_values (values.begin(), values.end());
	for (value_map_t::const_iterator cur_val = valMap.begin(); cur_val != valMap.end(); ++cur_val) {
	    set<string>::iterator it = new_values.find (cur_val->first);
	    if (it == new_values.end())
		throw xml_scenario_error ((boost::format("CaseManagement: %1% values don't match (expected): %2%") % decision % cur_val->first).str());
	    else
		new_values.erase (it);
	}
	if (!new_values.empty ()) {
	    ostringstream msg;
	    msg << "CaseManagement: "<<decision<<" values don't match (unexpected):";
	    BOOST_FOREACH ( const string& v, new_values )
		msg << ' ' << v;
	    throw xml_scenario_error (msg.str());
	}
	
    }
    
    return dec_pair.first->second.first;
}
ESDecisionValue ESDecisionValueMap::get (const string& decision, const string& value) const {
    if( value == "void" )
	return ESDecisionValue();	// void always maps to 0
    
    id_map_type::const_iterator it = id_map.find (decision);
    if (it == id_map.end())
	throw runtime_error ((boost::format("ESDecisionValueMap::get(): no decision %1%") %decision).str());
    
    value_map_t::const_iterator it2 = it->second.second.find (value);
    if (it2 == it->second.second.end())
	throw runtime_error ((boost::format("ESDecisionValueMap::get(): no value %1%(%2%)") %decision %value).str());
    
    //cout << "ESDecisionValueMap::get ("<<decision<<", "<<value<<"): "<<it2->second.id<<endl;
    return it2->second;
}
const pair< ESDecisionValue, ESDecisionValueMap::value_map_t > ESDecisionValueMap::getDecision (const string& decision) const {
    id_map_type::const_iterator it = id_map.find (decision);
    if (it == id_map.end ())
	throw invalid_argument ((boost::format ("ESDecisionValueMap: no decision \"%1%\" known") %decision).str());
    return it->second;
}

void ESDecisionValueMap::format( const ESDecisionValue v, ostream& stream ) const {
    bool second = false;	// prepend second, third, etc., with ", "
    for( id_map_type::const_iterator dec_it = id_map.begin(); dec_it != id_map.end(); ++dec_it ) {
	ESDecisionValue masked = v & dec_it->second.first;
	if( !(masked == ESDecisionValue()) ) {	// if not 0
	    for( value_map_t::const_iterator it = dec_it->second.second.begin(); it != dec_it->second.second.end(); ++it ) {
		if( masked == it->second ){
		    if( second )
			stream << ", ";
		    stream << dec_it->first<<'('<<it->first<<')';
		    second = true;
		    goto foundValue;
		}
	    }
	    assert( false );	// v matched mask but no value: this shouldn't happen!
	    foundValue:;
	}
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