/* This file is part of OpenMalaria.
 * 
 * Copyright (C) 2005-2009 Swiss Tropical Institute and Liverpool School Of Tropical Medicine
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

#ifndef Hmod_Survey
#define Hmod_Survey

#include "Global.h"

/** Enumeration of reporting options
 *
 * Most are reported per age-group. */
enum Measure {
  /// Total number of humans
  nHost = 0,
  /// number of infected hosts 
  nInfect = 1,
  /// expected number of infected hosts
  nExpectd= 2,
  /// number of patent hosts
  nPatent= 3,
  /// sum of log of pyrogen threshold
  sumX= 4,
  /// sum of the logarithm of the density
  sumlogDens= 5,
  /// Total infections
  totalInfs= 6,
  /** Infectiousness of human population to mosquitoes
   *
   * Number of hosts transmitting to mosquitoes (i.e. sum of proportion of
   * mosquitoes that get infected). We don't want this by age. */
  nTransmit= 7,
  /// Total patent infections
  totalPatentInf= 8,
  /// Contribution to immunity functions
  contrib= 9,
  /// Sum of pyrogenic threshold
  pyrogenThrs= 10,
  /// number of treatments (1st line)
  nTreatments1= 11,
  /// number of treatments (2nd line)
  nTreatments2= 12,
  /// number of treatments (inpatient)
  nTreatments3= 13,
  /// number of episodes (uncomplicated)
  nUncomp= 14,
  /// number of episodes (severe)
  nSevere= 15,
  /// cases with sequelae
  nSeq= 16,
  /// deaths in hospital
  nHospitalDeaths= 17,
  /// number of deaths (indirect)
  nIndDeaths= 18,
  /// number of deaths (direct)
  nDirDeaths= 19,
  /// number of EPI vaccine doses given
  nEPIVaccinations= 20,
  //all cause infant mortality rate
  imr_summary= 21,
  /// number of Mass / Campaign vaccine doses given
  nMassVaccinations= 22,
  /// recoveries in hospital
  nHospitalRecovs= 23,
  /// sequelae in hospital
  nHospitalSeqs= 24,
  /// number of IPT Doses
  nIPTDoses= 25,
  /** Annual Average Kappa
   *
   * Calculated once a year as sum of human infectiousness divided by initial
   * EIR summed over a year. */
  annAvgK= 26,
  /// Number of episodes (non-malaria fever)
  nNMFever= 27,
  /// Innoculations per human (all ages) per day of year, over the last year.
  innoculationsPerDayOfYear = 28,
  /// Kappa (human infectiousness) weighted by availability per day-of-year for the last year.
  kappaPerDayOfYear = 29,
  /** The total number of innoculations per age group, summed over the
   * reporting period. */
  innoculationsPerAgeGroup = 30,
  // TODO: Replace encoding within XML with something more extensible.
  // Can't use 31 without being very careful, because we're using a *signed* 32-bit int.
  // Note: can't use values greater than 31 without forcing a 64-bit type
  NUM_SUMMARY_OPTIONS	// must be hightest value above plus one
};

/// Data struct for a single survey.
class Survey {
  ///@brief Static members (options from XML)
  //@{
  public:
    /** Returns the age group/interval for a given age
     *
     * @param age Given age
     * @return age interval for a given age */
    static int ageGroup(double age);
    
    /// Get the total number of age categories (inc. one for indivs. not in any
    /// category given in XML).
    static int getNumOfAgeGroups() {return _upperbound.size();};
    
  private:
    /// Initialize static parameters.
    static void init();
    
    static double _lowerbound; //!< Lower boundary of the youngest agegroup
    /** Upper boundary of agegroups, in years.
     *
     * These are age-groups given in XML plus one with no upper limit for
     * individuals outside other bounds. */
    static vector<double> _upperbound; 
    
    /// Encoding of which summary options are active in XML is converted into
    /// this array for easier reading (and to make changing encoding within XML easier).
    static bool active[NUM_SUMMARY_OPTIONS];
    
    /** Assimilator mode
     *
     * If true, skip the first 3 columns of output to reduce file size. */
    static bool _assimilatorMode; 
  //@}
  
public:
  /// @brief reportXXX functions to report val more of measure XXX within age-group ageGroup. Returns this allowing chain calling.
  //Note: generate this list from variable definitions by regexp search-replacing using the following:
  //Search: vector<(\w+)> _num(\w+)\;
  //Replace: Survey& report\2 (size_t ageGroup, \1 val) {\n    _num\2[ageGroup] += val;\n    return *this;\n  }
  //@{
  Survey& reportHosts (size_t ageGroup, int val) {
    _numHosts[ageGroup] += val;
    return *this;
  }
  Survey& reportInfectedHosts (size_t ageGroup, int val) {
    _numInfectedHosts[ageGroup] += val;
    return *this;
  }
  Survey& reportExpectedInfected (size_t ageGroup, double val) {
    _numExpectedInfected[ageGroup] += val;
    return *this;
  }
  Survey& reportPatentHosts (size_t ageGroup, int val) {
    _numPatentHosts[ageGroup] += val;
    return *this;
  }
  Survey& reportTreatments1 (size_t ageGroup, int val) {
    _numTreatments1[ageGroup] += val;
    return *this;
  }
  Survey& reportTreatments2 (size_t ageGroup, int val) {
    _numTreatments2[ageGroup] += val;
    return *this;
  }
  Survey& reportTreatments3 (size_t ageGroup, int val) {
    _numTreatments3[ageGroup] += val;
    return *this;
  }
  Survey& reportUncomplicatedEpisodes (size_t ageGroup, int val) {
    _numUncomplicatedEpisodes[ageGroup] += val;
    return *this;
  }
  Survey& reportSevereEpisodes (size_t ageGroup, int val) {
    _numSevereEpisodes[ageGroup] += val;
    return *this;
  }
  Survey& reportSequelae (size_t ageGroup, int val) {
    _numSequelae[ageGroup] += val;
    return *this;
  }
  Survey& reportHospitalDeaths (size_t ageGroup, int val) {
    _numHospitalDeaths[ageGroup] += val;
    return *this;
  }
  Survey& reportIndirectDeaths (size_t ageGroup, int val) {
    _numIndirectDeaths[ageGroup] += val;
    return *this;
  }
  Survey& reportDirectDeaths (size_t ageGroup, int val) {
    _numDirectDeaths[ageGroup] += val;
    return *this;
  }
  Survey& reportEPIVaccinations (size_t ageGroup, int val) {
    _numEPIVaccinations[ageGroup] += val;
    return *this;
  }
  Survey& reportMassVaccinations (size_t ageGroup, int val) {
    _numMassVaccinations[ageGroup] += val;
    return *this;
  } 
  Survey& reportHospitalRecoveries (size_t ageGroup, int val) {
    _numHospitalRecoveries[ageGroup] += val;
    return *this;
  }
  Survey& reportHospitalSequelae (size_t ageGroup, int val) {
    _numHospitalSequelae[ageGroup] += val;
    return *this;
  }
  Survey& reportIPTDoses (size_t ageGroup, int val) {
    _numIPTDoses[ageGroup] += val;
    return *this;
  }
  Survey& reportNonMalariaFevers (size_t ageGroup, int val) {
    _numNonMalariaFevers[ageGroup] += val;
    return *this;
  } 
  //@}
  /// Report a first or second line, or inpatient treatment
  void reportTreatment(int ageGroup, int regimen);

  /** Update(add) the number of total infections
   *
   * @param age Given age
   * @param value Value to add
   */
  void addToTotalInfections(double age, int value);
  
  /** Update(add) the number of total patent infections
   *
   * @param age Given age
   * @param value Value to add
   */
  void addToTotalPatentInfections(double age, int value);
  
  /** Update(add) the table for the sum of the logarithm of the density
   *
   * @param age Given age
   * @param value Value to add
   */
  void addToSumLogDensity(double age, double value);
  
  /** Update(add) the table for the pyrogenic threshold
   *
   * @param age Given age
   * @param value Value to add
   */
  void addToPyrogenicThreshold(double age, double value);
  
  /** Update(add) the table for the sum of log of pyrogen threshold
   *
   * @param age Given age
   * @param value Value to add
   */
  void addToSumX(double age, double value);
  
  void setAnnualAverageKappa(double kappa);
  void setNumTransmittingHosts(double value);
  
  void setInnoculationsPerDayOfYear (vector<double>& v) {
    _innoculationsPerDayOfYear = v;
  }
  void setKappaPerDayOfYear (vector<double>& v) {
    _kappaPerDayOfYear = v;
  }
  void setInnoculationsPerAgeGroup (vector<double>& v) {
    _innoculationsPerAgeGroup = v;	// copies v, not just its reference
  }
  
private:
  /// Resize all vectors
  void allocate ();
  
  /** Write out arrays
   * @param outputFile Stream to write to
   * @param survey Survey number (starting from 1) */
  void writeSummaryArrays (ostream& outputFile, int survey);
  
  vector<int> _numHosts;
  vector<int> _numInfectedHosts;
  vector<double> _numExpectedInfected;
  vector<int> _numPatentHosts;
  vector<double> _sumX;
  vector<double> _sumLogDensity;
  vector<int> _totalInfections;
  double _numTransmittingHosts;
  vector<int> _totalPatentInfections;
  vector<double> _contributionImmunity;
  vector<double> _pyrogenicThreshold;
  vector<int> _numTreatments1;
  vector<int> _numTreatments2;
  vector<int> _numTreatments3;
  vector<int> _numUncomplicatedEpisodes;
  vector<int> _numSevereEpisodes;
  vector<int> _numSequelae;
  vector<int> _numHospitalDeaths;
  vector<int> _numIndirectDeaths;
  vector<int> _numDirectDeaths;
  vector<int> _numEPIVaccinations;
  vector<int> _numMassVaccinations; 
  vector<int> _numHospitalRecoveries;
  vector<int> _numHospitalSequelae;
  vector<int> _numIPTDoses;
  double _annualAverageKappa;
  vector<int> _numNonMalariaFevers; 
  vector<double> _innoculationsPerDayOfYear;
  vector<double> _kappaPerDayOfYear;
  vector<double> _innoculationsPerAgeGroup;
  
  friend class SurveysType;
};

/** Line end character. Use Unix line endings to save a little size. */
const char lineEnd = '\n';

template <class T>
void writeArray(ostream& file, int measure, bool assimilationMode, int survey, vector<T>& array);

template <class T>
void writeArray(ostream& file, int measure, bool assimilationMode, int survey, T& value);

#endif
