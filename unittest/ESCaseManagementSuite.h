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
// Unittest for the EventScheduler case management

#ifndef Hmod_ESCaseManagementSuite
#define Hmod_ESCaseManagementSuite

#include <cxxtest/TestSuite.h>
#include "Clinical/ESCaseManagement.h"
#include "ExtraAsserts.h"
#include <list>
#include <boost/assign/std/vector.hpp> // for 'operator+=()'

using namespace OM::Clinical;
using namespace boost::assign;

class ESCaseManagementSuite : public CxxTest::TestSuite
{
public:
    //TODO: test tree execution (that all necessary decisions are evaluated and outputs conglomerated as expected)
    
    //TODO: test trees handle "void" output correctly
    
    void setUp () {
	vector<string> vals;
	vals += "extra","poor";
	dMap.dvMap.add_decision_values( "modQty", vals );
	vals.clear();
	vals += "0","5";
	dMap.dvMap.add_decision_values( "modD1", vals );
	vals.clear();
	vals += "B2", "B5";
	dMap.dvMap.add_decision_values( "modD2", vals );
	vals.clear();
	vals += "all", "selective";
	dMap.dvMap.add_decision_values( "modSTR", vals );
	
	// We need to add a "treatment" decision to make the thing work, though we won't be using it:
	xsd::cxx::tree::sequence<scnXml::HSESDecision, false> decisionSeq;
	decisionSeq.push_back(
	    scnXml::HSESDecision(
		"void",	// tree
		"treatment",	// decision
		"",	// depends
		"treatment1"	// values
	    )
	);
	scnXml::HSESDecisions decisions;
	decisions.setDecision( decisionSeq );
	
	// Base schedule
	xsd::cxx::tree::sequence<scnXml::Medicate, false> medicateSeq;
	medicateSeq.push_back( scnXml::Medicate( "A", 1000.0, 0 ) );
	medicateSeq.push_back( scnXml::Medicate( "B", 3000.0, 0 ) );
	medicateSeq.push_back( scnXml::Medicate( "B", 3000.0, 12 ) );
	scnXml::HSESTreatmentSchedule treatSched;
	treatSched.setMedicate( medicateSeq );
	
	// Modifiers
	xsd::cxx::tree::sequence<scnXml::HSESTMMultiplyQty, false> modQtySeq;
	modQtySeq.push_back( scnXml::HSESTMMultiplyQty( "2,1.3", "extra", "A,B" ) );
	modQtySeq.push_back( scnXml::HSESTMMultiplyQty( "0.5,0.2", "poor", "A,B" ) );
	scnXml::HSESTreatmentModifier modQty( "modQty" );
	modQty.setMultiplyQty( modQtySeq );
	
	xsd::cxx::tree::sequence<scnXml::HSESTMDelay, false> modD1Seq;
	modD1Seq.push_back( scnXml::HSESTMDelay( "0,0", "0", "A,B" ) );
	modD1Seq.push_back( scnXml::HSESTMDelay( "5,5", "5", "A,B" ) );
	scnXml::HSESTreatmentModifier modD1( "modD1" );
	modD1.setDelay( modD1Seq );
	
	xsd::cxx::tree::sequence<scnXml::HSESTMDelay, false> modD2Seq;
	modD2Seq.push_back( scnXml::HSESTMDelay( "2,0", "B2", "B,A" ) );	// note: backwards
	modD2Seq.push_back( scnXml::HSESTMDelay( "0,5", "B5", "A,B" ) );
	scnXml::HSESTreatmentModifier modD2( "modD2" );
	modD2.setDelay( modD2Seq );
	
	xsd::cxx::tree::sequence<scnXml::HSESTMSelectTimeRange, false> modSTRSeq;
	modSTRSeq.push_back( scnXml::HSESTMSelectTimeRange( "0-100,0-100", "all", "A,B" ) );
	modSTRSeq.push_back( scnXml::HSESTMSelectTimeRange( "0-0,2-100", "selective", "A,B" ) );
	scnXml::HSESTreatmentModifier modSTR( "modSTR" );
	
	xsd::cxx::tree::sequence<scnXml::HSESTreatmentModifier, false> modifierSeq;
	modifierSeq.push_back( modQty );
	modifierSeq.push_back( modD1 );
	modifierSeq.push_back( modD2 );
	modifierSeq.push_back( modSTR );
	
	// Treatment
	scnXml::HSESTreatment treatment1( treatSched, "treatment1" );
	treatment1.setModifier( modifierSeq );
	
	scnXml::HSESTreatments treatments;
	xsd::cxx::tree::sequence<scnXml::HSESTreatment, false> treatmentSeq( 1, treatment1 );
	treatments.setTreatment( treatmentSeq );
	
	// Final CaseManagement element
	::scnXml::HSESCaseManagement xmlCM( decisions, treatments );
	
	// use complicated tree, because it doesn't add so many unwanted decisions
	dMap.initialize( xmlCM, true );
    }
    
    void testTreatments() {
	ESDecisionValue treatment1 = dMap.dvMap.get( "treatment", "treatment1" );
	treatment1 |= dMap.dvMap.get( "modQty", "poor" );
	treatment1 |= dMap.dvMap.get( "modD1", "5" );
	treatment1 |= dMap.dvMap.get( "modD2", "B2" );
	treatment1 |= dMap.dvMap.get( "modSTR", "selective" );
	
	const CaseTreatment *ct = dMap.getTreatment( treatment1 );
	TS_ASSERT( ct != NULL );
	TS_ASSERT_EQUALS( ct->medications.size(), 1u );
	
	const MedicateData& md = ct->medications[0];
	TS_ASSERT_EQUALS( md.abbrev, "B" );
	TS_ASSERT_EQUALS( md.qty, 600.0 );
	TS_ASSERT_EQUALS( md.time, 19.0 );
    }
    
private:
    ESDecisionMap dMap;
};

#endif
