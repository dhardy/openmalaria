#!/usr/bin/python
# -*- coding: utf-8 -*-
#FIXME: won't compare one file equal to itself now.
import sys
import string
import math
import unittest

totalRelDiff = 0.0

# Could use math.isinf, but it's not present in python 2.5
def isinf (a):
    return (a-a) != 0

# Careful with NaN, +/- inf and 0 values! Note: inf == inf
# Check a and b are approximately equal. Return true if:
#   a equals b to at least log10(relPrecision) significant figures
#   or at least log10(absPrecision) decimal places.
# This should work for small and large values, when one is zero, and when
# either is infinite or an NaN.
# 
# What it is limited by, is testing of small values with relative precision
# when it should also allow for large values being rounded to zero.
# Possibly considering the case where a or b is zero explicitly would help, but
# might lead to confusing outcomes.
def approx_equal (a,b, relPrecision, absPrecision):
    if isinf(a) or isinf(b):
        relDiff = 1e1000-1e1000
    else:
        tolerance = relPrecision * max(math.fabs(a),math.fabs(b))
        tolerance = max(tolerance, absPrecision)
        relDiff = math.fabs(a-b) / tolerance
    global totalRelDiff
    totalRelDiff += relDiff
    return (relDiff < 1.0)

def approx_equal_6 (a,b):
    return approx_equal (a, b, 1e-6, 1e-6)

class TestApproxEqual (unittest.TestCase):
    def testNaN (self):
        Inf = 1e10000
        NaN = Inf * 0
        self.assert_ (not approx_equal_6 (NaN, 1))
        self.assert_ (not approx_equal_6 (NaN, 0))
        self.assert_ (not approx_equal_6 (NaN, NaN))
        self.assert_ (not approx_equal_6 (NaN, Inf))
    
    def testInf (self):
        Inf = 1e10000
        self.assert_ (not approx_equal_6 (Inf, 1))
        self.assert_ (not approx_equal_6 (Inf, 0))
        self.assert_ (not approx_equal_6 (Inf, Inf))
        self.assert_ (not approx_equal_6 (Inf, -Inf))
    
    def testZero (self):
        self.assert_ (not approx_equal_6 (0, 1e-6))
        self.assert_ (    approx_equal_6 (0, 1e-7))
    
    def testRegular (self):
        self.assert_ (not approx_equal_6 (1, 0))
        self.assert_ (not approx_equal_6 (1, 0.999999))
        self.assert_ (    approx_equal_6 (1, 0.9999995))
        self.assert_ (not approx_equal_6 (1000000, 999999))
        self.assert_ (    approx_equal_6 (1000000, 999999.5))
        # these are considered equal because of absolute precision limitation rather than relative:
        self.assert_ (    approx_equal_6 (0.000001, 0.0000005))
        # this is roughly on the verge of what isn't considered equal:
        self.assert_ (not approx_equal_6 (0.000001, 0.000002))
        # if we only want to test relative precision:
        self.assert_ (not approx_equal (0.000001, 0.000000999999,  1e-6, 0))
        self.assert_ (    approx_equal (0.000001, 0.0000009999995, 1e-6, 0))


class ValIdentifier:
    def __init__(self,s,g,m):
        self.survey = s
        self.ageGroup = g
        self.measure = m
    def __eq__(self,other):
        return (self.survey != other.survey) or (self.ageGroup != other.ageGroup) or (self.measure != other.measure)
    def __hash__(self):
        return self.survey.__hash__() ^ self.ageGroup.__hash__() ^ self.measure.__hash__()


def main(*args):
    maxDiffsToPrint=6
    if (len(args) == 4):
        maxDiffsToPrint=int(args[3])
    elif (len(args) == 3):
        pass
    else:
        print "Usage: "+args[0]+" logfile1 logfile2 [max different lines to print]"
        return 1
    
    print args[0]+" "+args[1]+" "+args[2]+" "+str(maxDiffsToPrint)
    
    values=dict()
    values2=dict()
    file1=open(args[1], 'r')
    for line1 in file1:
        line_items1=string.split(line1)
        key=ValIdentifier(int(line_items1[0]),int(line_items1[1]),int(line_items1[2]))
        values[key]=float(line_items1[3])
    
    file2=open(args[2], 'r')
    numDiffs=0
    perMeasureNumDiff = dict()
    perMeasureDiffSum = dict()
    perMeasureDiffAbsSum = dict()
    for line2 in file2:
        line_items2=string.split(line2)
        survey=int(line_items2[0])
        ageGroup=int(line_items2[1])
        measure=int(line_items2[2])
        key2=ValIdentifier(survey,ageGroup,measure)
        value2=float(line_items2[3])
        
        if not key2 in values:
            values2[key2]=value2
            continue
        value1=values[key2]
        # Remove the value, so we can check all entries in values are in second file:
        del values[key2]
        
        # Compare with relative precision
        if not approx_equal_6 (value1, value2):
            numDiffs += 1
            perMeasureNumDiff[measure] = perMeasureNumDiff.get(measure,0) + 1;
            if (numDiffs <= maxDiffsToPrint):
                print "survey {1:>3}, age group {2:>3}, measure {3:>3}:{4:>12.5f} ->{5:>12.5f}".format(0,survey,ageGroup,measure,value1,value2)
                if (numDiffs == maxDiffsToPrint):
                    print "[won't print any more line-by-line diffs]"
        
        # Sum up total difference per measure
        perMeasureDiffSum[measure]    = perMeasureDiffSum.get(measure,0.0)    + value2 - value1
        perMeasureDiffAbsSum[measure] = perMeasureDiffAbsSum.get(measure,0.0) + math.fabs(value2-value1)
    
    if len(values):
        print "{2} entries in {0} but not {1} (unordered excerpt):".format(args[1],args[2],len(values))
        count = 0
        for (key,val) in values.iteritems():
            if count >= maxDiffsToPrint:
                break
            print "survey {1:>3}, age group {2:>3}, measure {3:>3}:{0:>12.5f}".format(val,key.survey,key.ageGroup,key.measure)
            count += 1
    if len(values2):
        print "{2} entries in {0} but not {1} (unordered excerpt):".format(args[2],args[1],len(values2))
        count = 0
        for (key,val) in values2.iteritems():
            if count >= maxDiffsToPrint:
                break
            print "survey {1:>3}, age group {2:>3}, measure {3:>3}:{0:>12.5f}".format(val,key.survey,key.ageGroup,key.measure)
            count += 1
    
    if len(values) or len(values2):
        return 3
    
    for (measure,val) in perMeasureDiffAbsSum.iteritems():
        if val > 1e-6:
            diff=perMeasureDiffSum[measure]
            print "Diff sum for measure {0: >3}:{1: >12.5f}\tabs: {2: >12.5f}\t(ratio: {3: >9.5f}; from {4:>3} diffs)".format(measure,diff,val,diff/val,perMeasureNumDiff.get(measure,0))
    
    # We print total relative diff here: 1.0 should mean roughly, one parameter is twice what it should be.
    if numDiffs == 0:
        print "No significant differences (total relative diff: {0}), ok...".format(totalRelDiff/1.e6)
        return 0
    else:
        print "{0} significant differences (total relative diff: {1})!".format(numDiffs,totalRelDiff/1.e6)
        return 1

if __name__ == '__main__':
    #uncomment to run unittests:
    #unittest.main()
    sys.exit(main(*sys.argv))
