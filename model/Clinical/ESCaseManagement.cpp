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

#include "Clinical/ESCaseManagement.h"
#include "Clinical/ESDecisionTree.h"
#include "inputData.h"
#include "util/errors.hpp"

#include <set>
#include <boost/format.hpp>

namespace OM { namespace Clinical {
    using namespace OM::util;
    using boost::format;


// -----  ESTreatmentSchedule  -----

ESTreatmentSchedule::ESTreatmentSchedule (const scnXml::HSESTreatmentSchedule& sched) {
    const ::scnXml::HSESTreatmentSchedule::MedicateSequence& mSeq = sched.getMedicate();
    medications.resize (mSeq.size ());
    for (size_t j = 0; j < mSeq.size(); ++j) {
	medications[j].abbrev = mSeq[j].getDrug();
	medications[j].qty = mSeq[j].getMg();
	medications[j].time = mSeq[j].getHour() / 24.0;	// convert from hours to days
    }
}

ESTreatmentSchedule* ESTreatmentSchedule::modify (const scnXml::HSESTMMultiplyQty&) const {
    
}


// -----  ESTreatment  -----

ESTreatment::ESTreatment(const ESDecisionValueMap& dvMap, const scnXml::HSESTreatment& elt) {
    typedef ESDecisionValueMap::value_map_t value_map_t;
    
    schedules[ ESDecisionValue() ] = new ESTreatmentSchedule( elt.getSchedule() );
    Schedules startSchedules;
    
    BOOST_FOREACH( const scnXml::HSESTreatmentModifier& modifier, elt.getModifier() ){
	schedules.swap( startSchedules );
	for( Schedules::iterator it = schedules.begin(); it != schedules.end(); ++it ) {
	    delete it->second;
	}
	schedules.clear();
	
	value_map_t decVals = dvMap.getDecision( modifier.getDecision() ).second;	// copy
	schedules.rehash( decVals.size() / schedules.max_load_factor() + 1 );
	
	if( modifier.getMultiplyQty().size() ) {
	    BOOST_FOREACH( const scnXml::HSESTMMultiplyQty& mod, modifier.getMultiplyQty() ){
		value_map_t::iterator it = decVals.find( mod.getValue() );
		if( it == decVals.end() )
		    throw xml_scenario_error( (boost::format("modifier sub-element for non-existant decision value %1%(%2%)") %modifier.getDecision() %mod.getValue()).str() );
		ESDecisionValue val = it->second;
		decVals.erase( it );
		
		for( Schedules::iterator s = startSchedules.begin(); s != startSchedules.end(); ++s ){
		    schedules[ s->first | val ] = s->second->modify( mod );
		}
	    }
	} else if( modifier.getDelay().size() ) {
	} else if( modifier.getSelectTimeRange().size() ) {
	} else
	    throw xml_scenario_error( "modifier element without any sub-elements" );
	
	if( !decVals.empty() ){
	    //TODO: error msg
	}
    }
}

ESTreatment::~ESTreatment() {
    for( Schedules::iterator it = schedules.begin(); it != schedules.end(); ++it ) {
	delete it->second;
    }
}

ESTreatmentSchedule* ESTreatment::getSchedule (ESDecisionValue& outcome) const {
    outcome = outcome & schedulesMask;
    Schedules::const_iterator it = schedules.find (outcome);
    if (it == schedules.end ())
	return NULL;
    else
	return it->second;
}

// -----  ESDecisionMap  -----

inline bool hasAllDependencies (const ESDecisionTree* decision, const set<string>& dependencies) {
    BOOST_FOREACH ( const string& n, decision->depends ) {
	if (dependencies.count(n) == 0) {
	    return false;
	}
    }
    return true;
}
inline ESDecisionValue treatmentGetValue (const ESDecisionValueMap::value_map_t& vmap, const string& value) {
    ESDecisionValueMap::value_map_t::const_iterator it = vmap.find (value);
    if (it == vmap.end())
	// value is "void" or something unknown; neither is acceptable
	throw xml_scenario_error((format("Treatment description given for treatment %1% which isn't an output of \"treatment\" decision") %value).str());
    return it->second;
}
void ESDecisionMap::initialize (const ::scnXml::HSESCaseManagement& xmlCM, bool complicated) {
    // Assemble a list of all tests we need to add
    list<ESDecisionTree*> toAdd;
    toAdd.push_back (new ESDecisionAge5Test (dvMap));
    if (!complicated) {
	toAdd.push_back (new ESDecisionUC2Test (dvMap));
	toAdd.push_back (new ESDecisionParasiteTest (dvMap));
    }
    BOOST_FOREACH ( const ::scnXml::HSESDecision& xmlDc, xmlCM.getDecisions().getDecision() ) {
	toAdd.push_back (new ESDecisionRandom (dvMap, xmlDc));
    }
    
    decisions.resize (toAdd.size());
    size_t i = 0;
    set<string> added;	// names of decisions added; used since it's faster to lookup decision names here than linearly in "decisions"
    for (list<ESDecisionTree*>::iterator it = toAdd.begin(); ;) {
	if (it == toAdd.end ()) {
	    if (toAdd.empty ())	// good, we're done
		break;
	    // else: some elements had unresolved dependencies
	    cerr << "ESCaseManagement: some decisions have unmeetable dependencies (for "<<(complicated?"":"un")<<"complicated tree):";
	    BOOST_FOREACH ( ESDecisionTree* decision, toAdd ) {
		cerr << "\ndecision: "<<decision->decision;
		cerr << "\thas unmet dependency decisions:";
		BOOST_FOREACH ( string& dep, decision->depends ) {
		    if (added.count(dep))
			cerr << " (" << dep << ')';
		    else
			cerr << " " << dep;
		}
	    }
	    cerr << endl;
	    throw xml_scenario_error("ESCaseManagement: some decisions have unmeetable dependencies (see above)");
	}
	//cout << "Considering " << (*it)->decision << " with " << (*it)->depends.size()<<" dependencies"<<endl;
	if (hasAllDependencies (*it, added)) {
	    decisions[i++] = *it;
	    added.insert ((*it)->decision);
	    toAdd.erase (it);
	    it = toAdd.begin ();	// restart from beginning; affects order and means we know if we reach the end there shouldn't be any elements left
	} else
	    ++it;
    }
    
    
    // Read treatments:
    pair< ESDecisionValue, ESDecisionValueMap::value_map_t > mask_vmap_pair = dvMap.getDecision("treatment");
    const ESDecisionValueMap::value_map_t& treatmentCodes = mask_vmap_pair.second;
    BOOST_FOREACH( const ::scnXml::HSESTreatment& treatment, xmlCM.getTreatments().getTreatment() ){
	treatments[treatmentGetValue( treatmentCodes, treatment.getName() )] = new ESTreatment( dvMap, treatment );
    }
    // Associate "void" input with an ESTreatment which has just one, empty, ESTreatmentSchedule:
    treatments[ESDecisionValue()] = new ESTreatment(
	dvMap,
	scnXml::HSESTreatment(
	    scnXml::HSESTreatmentSchedule(),	// empty base schedule
	    "void"		// name of treatment
	)
    );
    
    treatmentsMask = mask_vmap_pair.first;
    
    
    // TODO: could check at this point that all possible combinations of
    // decision outputs (including void) resolve a treatment (so we don't get
    // exceptions when running big workunits on BOINC).
}

ESDecisionMap::~ESDecisionMap () {
    BOOST_FOREACH ( ESDecisionTree* d, decisions ) {
	delete d;
    }
    for( Treatments::iterator it = treatments.begin(); it != treatments.end(); ++it )
	delete it->second;
}

ESDecisionValue ESDecisionMap::determine (OM::Clinical::ESHostData& hostData) const {
    ESDecisionValue outcomes;	// initialized to 0
    // At this point, decisions is ordered such that all dependencies should be
    // met if evaluated in order, so we just do that.
    BOOST_FOREACH ( const ESDecisionTree* decision, decisions ) {
	// Pass determine the outcomes of previous decisions, filtered to the decisions it depends on.
	// Get back another outcome and add it into the outcomes set.
	outcomes |= decision->determine (outcomes & decision->mask, hostData);
    }
    return outcomes;
}
ESTreatmentSchedule* ESDecisionMap::getSchedule (ESDecisionValue outcome) const {
    ESDecisionValue masked = outcome & treatmentsMask;
    Treatments::const_iterator it = treatments.find (masked);
    if (it != treatments.end ()) {
	ESTreatmentSchedule* ret = it->second->getSchedule( outcome );
	if( ret != NULL )
	    return ret;
	masked = masked & outcome;
    }
    
    ostringstream msg;
    msg<<"decision outcome "<<dvMap.format( masked )<<" not found in list of treatments";
    throw xml_scenario_error (msg.str());
}


// -----  ESCaseManagement  -----

ESDecisionMap ESCaseManagement::uncomplicated, ESCaseManagement::complicated;
ESTreatmentSchedule* ESCaseManagement::mdaDoses;

void ESCaseManagement::init () {
    // Assume EventScheduler data was checked present:
    const scnXml::HSEventScheduler& xmlESCM = InputData().getHealthSystem().getEventScheduler().get();
    uncomplicated.initialize (xmlESCM.getUncomplicated (), false);
    complicated.initialize (xmlESCM.getComplicated (), true);
    
    // MDA Intervention data
    const scnXml::Interventions::MDADescriptionOptional mdaDesc = InputData().getInterventions().getMDADescription();
    if (mdaDesc.present()) {
	if( !mdaDesc.get().getSchedule().present() )
	    throw xml_scenario_error( "MDA description requires a treatment schedule with ES case management" );
	mdaDoses = new ESTreatmentSchedule ( mdaDesc.get().getSchedule().get() );
    } else
	mdaDoses = NULL;
}

void ESCaseManagement::massDrugAdministration(list<MedicateData>& medicateQueue) {
    if (mdaDoses == NULL)
	throw util::xml_scenario_error ("MDA intervention without description");
    else
	mdaDoses->apply(medicateQueue);
}

void ESCaseManagement::execute (list<MedicateData>& medicateQueue, Pathogenesis::State pgState, WithinHost::WithinHostModel& withinHostModel, double ageYears, SurveyAgeGroup ageGroup) {
    ESDecisionMap* map;
    assert (pgState & Pathogenesis::SICK);
    if (pgState & Pathogenesis::COMPLICATED)
        map = &complicated;
    else
        map = &uncomplicated;
    
    ESHostData hostData (ageYears, withinHostModel, pgState);
    
    ESDecisionValue outcome = map->determine (hostData);
    
    ESTreatmentSchedule* schedule = map->getSchedule(outcome);
    
    // We always remove any queued medications.
    medicateQueue.clear();
    schedule->apply (medicateQueue);
}

} }