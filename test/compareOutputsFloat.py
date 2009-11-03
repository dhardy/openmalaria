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
        return (self.survey == other.survey) and (self.ageGroup == other.ageGroup) and (self.measure == other.measure)
    def __hash__(self):
        return self.survey.__hash__() ^ self.ageGroup.__hash__() ^ self.measure.__hash__()

class TestValIdentifier (unittest.TestCase):
    def setUp(self):
        self.a1 = ValIdentifier(2,4,0);
        self.a2 = ValIdentifier(2,4,0);
        self.a3 = ValIdentifier(2,2,0);
        self.b1 = ValIdentifier(0,16,5);
        self.b2 = ValIdentifier(0,16,5);
    def testEq (self):
        self.assert_ (self.a1.survey == self.a2.survey)
        self.assert_ (self.a1 == self.a2)
        self.assert_ (self.b1 == self.b2)
        self.assert_ (self.a1 != self.a3)
    def testHash (self):
        self.assert_ (self.a1.__hash__ == self.a2.__hash__)
        self.assert_ (self.b1.__hash__ == self.b2.__hash__)


def ReadEntries (fname):
    values=dict()
    fileObj = open(fname, 'r')
    for line in fileObj:
        items=string.split(line)
        key=ValIdentifier(int(items[0]),int(items[1]),int(items[2]))
        values[key]=float(items[3])
    return values

def main(*args):
    ret=0
    maxDiffsToPrint=6
    if (len(args) == 4):
        maxDiffsToPrint=int(args[3])
    elif (len(args) == 3):
        pass
    else:
        print "Usage: "+args[0]+" logfile1 logfile2 [max different lines to print]"
        return 1
    
    fn1 = args[1]
    fn2 = args[2]
    print args[0]+" "+fn1+" "+fn2+" "+str(maxDiffsToPrint)
    
    # Read both files and combine into a map of key to pairs (v1, v2)
    try:
        values1=ReadEntries(fn1)
        values2=ReadEntries(fn2)
    except IOError as e:
        print str(e)
        return 1
    values=dict()
    for (k,v1) in values1.iteritems():
        v2=None
        if (k in values2):
            v2=values2[k]
            del values2[k]
        values[k] = (v1,v2)
    for (k,v2) in values2.iteritems():
        values[k] = (None,v2)
    
    # Go through all values:
    numPrinted=0
    numDiffs=0
    numMissing1=0
    numMissing2=0
    perMeasureNumDiff = dict()
    perMeasureDiffSum = dict()
    perMeasureDiffAbsSum = dict()
    for (k,(v1,v2)) in values.iteritems():
        if v1==None:
            numMissing1 += 1
        elif v2==None:
            numMissing2 += 1
        # Compare with relative precision
        elif not approx_equal_6 (v1, v2):
            numDiffs += 1
        else:
            continue
        
        numPrinted += 1
        perMeasureNumDiff[k.measure] = perMeasureNumDiff.get(k.measure,0) + 1;
        if (numPrinted <= maxDiffsToPrint):
            print "survey {1:>3}, age group {2:>3}, measure {3:>3}:{4:>12.5f} ->{5:>12.5f}".format(0,k.survey,k.ageGroup,k.measure,v1,v2)
            if (numPrinted == maxDiffsToPrint):
                print "[won't print any more line-by-line diffs]"
        
        # Sum up total difference per measure
        perMeasureDiffSum[k.measure]    = perMeasureDiffSum.get(k.measure,0.0)    + v2 - v1
        perMeasureDiffAbsSum[k.measure] = perMeasureDiffAbsSum.get(k.measure,0.0) + math.fabs(v2-v1)
    
    if (numMissing1 > 0) or (numMissing2 > 0):
        print "{0} entries missing from first file, {1} from second".format(numMissing1,numMissing2)
        ret = 3
    
    for (k.measure,val) in perMeasureDiffAbsSum.iteritems():
        if val > 1e-6:
            diff=perMeasureDiffSum[k.measure]
            print "Diff sum for measure {0: >3}:{1: >12.5f}\tabs: {2: >12.5f}\t(ratio: {3: >9.5f}; from {4:>3} diffs)".format(k.measure,diff,val,diff/val,perMeasureNumDiff.get(k.measure,0))
    
    # We print total relative diff here: 1.0 should mean roughly, one parameter is twice what it should be.
    if numDiffs == 0:
        print "No significant differences (total relative diff: {0}), ok...".format(totalRelDiff/1.e6)
        return ret
    else:
        print "{0} significant differences (total relative diff: {1})!".format(numDiffs,totalRelDiff/1.e6)
        return 1

if __name__ == '__main__':
    #uncomment to run unittests:
    #unittest.main()
    sys.exit(main(*sys.argv))
