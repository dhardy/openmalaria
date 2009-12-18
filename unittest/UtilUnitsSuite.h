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
// Unittest for the drug model

#ifndef Hmod_UtilUnitsSuite
#define Hmod_UtilUnitsSuite

#include <cxxtest/TestSuite.h>
#include "util/units.hpp"
#include <iostream>

using namespace OM::util::units;
using namespace boost::units;

class UtilUnitsSuite : public CxxTest::TestSuite
{
public:
  void setUp () {
  }
  void tearDown () {
  }
  
  void testNone () {
      quantity<infections,double> infs = 10. * innoculation;
      quantity<time,double> y = 1.0 * year;
      quantity <infections_per_second,double> eir = infs / y;
      std::cout << "10 innocs per year: " << eir << std::endl;
  }
};

#endif
