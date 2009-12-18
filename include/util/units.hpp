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

#ifndef Hmod_util_units
#define Hmod_util_units

#include <boost/units/base_dimension.hpp>
#include <boost/units/base_unit.hpp>
#include <boost/units/dim.hpp>
#include <boost/units/systems/si/time.hpp>

namespace OM { namespace util { namespace units {
    using namespace boost::units;
    
    /** Base dimension for EIR. */
    struct infections_base_dimension : boost::units::base_dimension<infections_base_dimension, 1> {};
    
    /** Dimension for entomological infections. */
    typedef infections_base_dimension::dimension_type infections_dimension;
    
    /** Dimension for EIR (ento inf rate). */
    typedef boost::units::make_dimension_list<
	boost::mpl::list<
	    boost::units::dim< infections_base_dimension,boost::units::static_rational<1> >,
	    boost::units::dim< boost::units::time_base_dimension,boost::units::static_rational<-1> >
	>
    >::type   infection_rate_dimension;
    
    /** Base unit (1 infection) */
    struct infections_base_unit : public boost::units::base_unit<infections_base_unit, infections_dimension, 1>
    {
	static std::string name()               { return "entomological infections"; }
	static std::string symbol()             { return "innocs"; }
    };
    
    /** Our unit system. */
    typedef boost::units::make_system<infections_base_unit>::type om_system;
    
    /** Unit: number of infections */
    typedef boost::units::unit<infections_dimension,om_system> infections;
    
    /** Unit: infections per second */
    typedef boost::units::unit<infection_rate_dimension,om_system> infections_per_second;
    
    BOOST_UNITS_STATIC_CONSTANT(innoculation, infections); 
    BOOST_UNITS_STATIC_CONSTANT(innocs_per_sec, infections_per_second);
} } }
#endif