/*

 This file is part of OpenMalaria.

 Copyright (C) 2005,2006,2007,2008 Swiss Tropical Institute and Liverpool School Of Tropical Medicine
 
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
#include "Clinical/event.h"
#include "inputData.h"
#include "util/gsl.h"
#include <algorithm>
#include "Clinical/ClinicalModel.h"
#include "Surveys.h"

void Event::update(int simulationTime, int ageGroup, int diagnosis, int outcome){
  if ((diagnosis == Diagnosis::INDIRECT_MALARIA_DEATH) || (simulationTime>(_time + ClinicalModel::reportingPeriodMemory))){
    report();
    
    _time=simulationTime;
    _surveyPeriod=Surveys.getSurveyPeriod();
    _ageGroup=ageGroup;
    _diagnosis=diagnosis;
    _outcome=outcome;
    _recurrence=1;
  }
  else {
    _outcome=std::max(outcome, _outcome);
    _diagnosis=std::max(diagnosis, _diagnosis);
    _recurrence++;
  }
}

Event::~Event () {
  report();
}

void Event::report () {
  if (_time == TIMESTEP_NEVER)	// Nothing to report
    return;
  
  switch (_diagnosis) {
    case Diagnosis::NON_MALARIA_FEVER:
      Surveys.at(_surveyPeriod)
	.reportNonMalariaFevers (_ageGroup, 1);
      break;
    case Diagnosis::UNCOMPLICATED_MALARIA:
      Surveys.at(_surveyPeriod)
	.reportUncomplicatedEpisodes (_ageGroup, 1);
      break;
    case Diagnosis::SEVERE_MALARIA:
      Surveys.at(_surveyPeriod)
	.reportSevereEpisodes (_ageGroup, 1);
      break;
    case Diagnosis::INDIRECT_MALARIA_DEATH:
      Surveys.at(_surveyPeriod)
	.reportIndirectDeaths (_ageGroup, 1);
      break;
    default:
      throw invalid_argument ("Unsupported diagnosis");
  }
  switch (_outcome) {
    case Outcome::NO_CHANGE_IN_PARASITOLOGICAL_STATUS_NON_TREATED:
      //do nothing
      break;
    case Outcome::NO_CHANGE_IN_PARASITOLOGICAL_STATUS_OUTPATIENTS:
      //do nothing
      break;
    case Outcome::NO_CHANGE_IN_PARASITOLOGICAL_STATUS_INPATIENTS:
      //do nothing
      break;
    case Outcome::PARASITES_ARE_CLEARED_PATIENT_RECOVERS_NON_TREATED:
      //do nothing
      break;
    case Outcome::PARASITES_ARE_CLEARED_PATIENT_RECOVERS_OUTPATIENTS:
      //do nothing
      break;
    case Outcome::PARASITES_ARE_CLEARED_PATIENT_RECOVERS_INPATIENTS:
      Surveys.at(_surveyPeriod)
	.reportHospitalRecoveries (_ageGroup, 1);
      //TODO: we curr do not distinquish between treated vs untreated seqs.
      break;
    case Outcome::PARASITES_ARE_CLEARED_PATIENT_HAS_SEQUELAE_NON_TREATED:
      Surveys.at(_surveyPeriod)
	.reportSequelae (_ageGroup, 1);
      break;
    case Outcome::PARASITES_ARE_CLEARED_PATIENT_HAS_SEQUELAE_INPATIENTS:
      Surveys.at(_surveyPeriod)
	.reportSequelae (_ageGroup, 1)
	.reportHospitalSequelae (_ageGroup, 1);
      break;
    case Outcome::PARASITES_NOT_CLEARED_PATIENT_HAS_SEQUELAE_NON_TREATED:
      Surveys.at(_surveyPeriod)
	.reportSequelae (_ageGroup, 1);
      break;
    case Outcome::PARASITES_NOT_CLEARED_PATIENT_HAS_SEQUELAE_INPATIENTS:
      Surveys.at(_surveyPeriod)
	.reportSequelae (_ageGroup, 1)
	.reportHospitalSequelae (_ageGroup, 1);
      break;
    case Outcome::PATIENT_DIES_NON_TREATED:
      Surveys.at(_surveyPeriod)
	.reportDirectDeaths (_ageGroup, 1);
      break;
    case Outcome::PATIENT_DIES_INPATIENTS:
      Surveys.at(_surveyPeriod)
	.reportDirectDeaths (_ageGroup, 1)
	.reportHospitalDeaths (_ageGroup, 1);
      break;
    case Outcome::INDIRECT_DEATH:
      //do nothing
      break;
    default:
      throw invalid_argument ("Unsupported outcome");
  }
}


ostream& operator<<(ostream& out, const Event& event){
  out << event._time << endl;
  if (event._time == TIMESTEP_NEVER) return out;
  out << event._surveyPeriod << endl;
  out << event._ageGroup << endl;
  out << event._diagnosis << endl;
  out << event._outcome << endl;
  return out << event._recurrence << endl;
}

istream& operator>>(istream& in, Event& event){
  in >> event._time;
  if (event._time == TIMESTEP_NEVER) return in;
  in >> event._surveyPeriod;
  in >> event._ageGroup;
  in >> event._diagnosis;
  in >> event._outcome;
  in >> event._recurrence;
  return in;
}
