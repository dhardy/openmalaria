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

#include "WithinHost/Infection.h"

float Infection::cumulativeYstar;
float Infection::cumulativeHstar;

Infection::Infection (istream& in) {
  in >> _duration;
  in >> _startdate;
  in >> _density;
  if (Global::modelVersion & INCLUDES_PK_PD) {
    int proteomeID;
    in >> proteomeID;
    _proteome = ProteomeInstance::getProteome(proteomeID);
  } else
    _proteome = NULL;
}
void Infection::writeInfection (ostream& out) const {
  out << _duration << endl; 
  out << _startdate << endl; 
  out << _density << endl; 
  if (Global::modelVersion & INCLUDES_PK_PD) {
    out << _proteome->getProteomeID() << endl; 
  }
}
