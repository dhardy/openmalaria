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

#include "CaseManagementModel.h"
#include "OldCaseManagement.h"
#include "NewCaseManagement.h"
#include "inputData.h"
#include "simulation.h"
#include "summary.h"


// -----  init  -----

CaseManagementModel::CaseManagementModel (double tSF) :
    _treatmentSeekingFactor(tSF), _tLastTreatment(TIMESTEP_NEVER)
{}

CaseManagementModel::CaseManagementModel (istream& in)
{
  in >> _treatmentSeekingFactor; 
  in >> _tLastTreatment; 
}

CaseManagementModel::~CaseManagementModel ()
{}


// -----  other  -----

bool CaseManagementModel::recentTreatment() {
  return (Simulation::simulationTime-_tLastTreatment >= 1 &&
  Simulation::simulationTime-_tLastTreatment <= 4);
}


// -----  checkpointing  -----

void CaseManagementModel::write(ostream& out) const {
  out << _treatmentSeekingFactor << endl; 
  out << _tLastTreatment << endl; 
}
