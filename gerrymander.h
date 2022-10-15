/* Christopher Lee (2022_08_07)
 *
 * This file outlines a the Gerrymander class.
 *
 * In the public sectino, everything there is either already
 * given or needs to be implemented.
 *
 * In the private section, the VotingMap and the
 * VectorToSet() method is there by default, and
 * extra methods may be added.
 */

#pragma once

#ifndef GERRYMANDER_H
#define GERRYMANDER_H

#include "votingmap.h"
#include "set.h"
#include "priorityqueue.h"



class Gerrymander
{
public:
    // Default constructor that implicitly initializes the VotingMap;
    Gerrymander();

    // Adding an Area to the VotingMap
    void addArea(Area* newArea);
    void addArea(int id, int dem, int rep, int pop, Set<int> adjacency);


    // Determines if the proposed plan qualifies demographic constraints (contunious and similar population)
    bool isValidPlan(Set<Set<int>>& districts, double margin) const;

    // Dermines how gerrymandered a plan is through the "Efficiency Gap"
    // https://www.quantamagazine.org/the-mathematics-behind-gerrymandering-20170404/
    int howGerrymandered(Set<Set<int>>& district) const;

    // Determines whether the degree of inpoportionality (Efficiency Gap) is greater than some number
    bool isGerrymandered(Set<Set<int>>& districts, int margin) const;


    // Gerrymanders a given region for a certain party
    Set<Set<int>> gerrymander(int totalDistricts, bool favorRep) const;

    // Will generate a gerrymandered plan by randomly generating plans and seeing if they are gerrymandered or not.
    Set<Set<int>> naiveGerrymander(int totalDistricts, int margin) const;

    // Generates a random valid plan for a given number of districts
    Set<Set<int>> createRandomPlan(int totalDistricts) const;

private:
    // The only member variable, which holds a VotingMap (basically an adjacency graph)
    VotingMap map;

    // Checks whether a given district is continuous, used in ""isValidPlan(Set, double)"
    bool isContinuous(Set<int> district) const;
    void isContinuousHelper(Set<int>& district, int start) const;

    // Intermediate steps that are used for the "gerrymander(int, bool)" method
    void gerrymanderHelper(Set<Set<int>>& districts, int totalDistricts, bool favorRep, Set<int>& ids) const;
    void createGerrymanderedDistrict(Set<int>& district,  Demographic& demo, int curID, const int& maxPop, bool favorRep, Set<int>& adj, Set<int>& ids) const;


    // Intermediate steps that are used for the "createRandomPlan(int)" method
    void createRandomPlanHelper(Set<Set<int>>& districts, int totalDistricts, Set<int>& ids) const;
    void createRandomDistrict(Set<int>& district, int curID, int& districtPop, const int& maxPop, Set<int>& ids) const;


    // 2 utility functions that calculate "waste" for the Efficiency Gap
    int repWasted(int dem, int rep) const;
    int demWasted(int dem, int rep) const;

    // A utility function that transforms a given set into a vector (to randomize selection)
    Vector<int> setToVector(Set<int>& intSet) const;
};

#endif // GERRYMANDER_H
