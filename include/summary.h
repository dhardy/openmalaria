/* This file is part of OpenMalaria.
 * 
 * Copyright (C) 2005,2006,2007,2008 Swiss Tropical Institute and Liverpool School Of Tropical Medicine
 * 
 * OpenMalaria is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef Hmod_summary
#define Hmod_summary
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Global.h"
#include <vector>
#include <fstream>

using namespace std;

// Forward Declaration
class Event;

//! It stores survey data and performes file I/O 
/*!
  It manages data from parasitological surey.
*/
class Summary {

 public:
  
  //! Initialisation Routine. This will have to be moved to the class constructor
  void initialiseSummaries ();

  //! Clearing Routine. This will have to be moved to the class destructor
  void clearSummaryParameters();


  //! It returns the time interval for a given survey
  /*!
    \param survey Given survey
  */
  int getSurveyTimeInterval(int survey) {return _surveysTimeIntervals[survey];};


  
 private:
  
  
};

#endif
