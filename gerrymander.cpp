/* Christopher Lee (2022_08_07)
 *
 * This file is the file that is intended to be edited
 * by the user.
 *
 * By default the only functionality that will be in
 * the file will be the addArea() methods, constructor(),
 * static constants, and the setToVector() method.
 *
 * The functionality to be implemented is to generate
 * valid and/or gerrymandered district plans, verify that a given
 * plan has been gerrymandered, and verify that a given plan is
 * valid.
 */

#include "gerrymander.h"

#include <iostream>

#include "random.h"
#include "testing/SimpleTest.h"

#include <cstdlib>

// In actuality this number shuold be closer to 5% (what it seems to be in Texas),
// however it will potentially make testing harder since the margin is more sensative the larger the precincts (i.e. testing data would require a lot of small precincts)
// Therefore, to not waste a ton of time compiling huge datasets and shaping them into these datatypes, the population margin remains redicuously high
const double POPULATION_MARGIN = 0.2;
// Varibale that denotes that the method can not run properly for some given reason
const int NONE = -1;

// Default constructor
Gerrymander::Gerrymander() {}

// Calls the same function on its VotingMap
void Gerrymander::addArea(Area* newArea) {
    map.addArea(newArea);
}

// Adds an Area by explicitly inputing its variables
void Gerrymander::addArea(int id, int dem, int rep, int pop, Set<int> adjacency) {
    Area* loc = new Area(id, dem, rep, pop, adjacency);
    addArea(loc);
}

/*
 *  This method checks whether a plan of district is valid
 *
 *  First, it checks if the district is continuous,
 *  then it checks if the population of the district as a whole
 *  is within the mean population plus-minus margin
 */
bool Gerrymander::isValidPlan(Set<Set<int>>& districts, double margin) const {
    Set<int> allIDs = map.precinctSet();
    const int mean = map.totalPop() / districts.size();

    for (Set<int> district: districts) {    // O(n)
        // Checks continuity
        if (!isContinuous(district)) {  // O(n)
            return false;
        }

        // Aggregates population data
        int districtPop = 0;
        for (int precinct : district) {
            districtPop += map.getDemographic(precinct).pop;    // O(log n)
            allIDs.remove(precinct);                            // O(log n)
        }

        if (districtPop > mean * (1 + margin) || districtPop < mean * (1 - margin)) {
            return false;
        }
    }

    return allIDs.isEmpty();    // Checks if the plan uses all precicnts
}

/*
 * Helper function that returns true of the district is continuous
 */
bool Gerrymander::isContinuous(Set<int> district) const{
    isContinuousHelper(district, district.first());
    return district.isEmpty();
}

/*
 * Helper function that varifies continuity by starting at an arbitrary point of
 * the district then attempting to draw a path through every connected node wihtin the
 * district (uses DFS).
 *
 * It returns true if it covered all the precicnts in the district
 */
void Gerrymander::isContinuousHelper(Set<int>& district, int start) const {
    if (!district.contains(start)) {
        return;
    }

    district.remove(start);

    Set<int> next = map.getAdjacentPrecincts(start);


    for (int id : next) {
        isContinuousHelper(district, id);
    }
}

/*
 * Method that returns the degree of disproportionate voting using the
 * Efficiency Gap.
 *
 * Wasted votes are definied as votes that are surplus to a win, and any vote
 * of the losing block.
 *
 * These votes are counted for both sides within each district, and at the end
 * the values are plugged into the eq
 *
 * |Waste1 - Waste2|
 * -----------------
 *    total votes
 *
 * To produce a quantifiable number for gerrymandering
 */
int Gerrymander::howGerrymandered(Set<Set<int>>& districts) const {
    if (!isValidPlan(districts, POPULATION_MARGIN)) {
        return NONE;
    }

    int demWaste = 0;
    int repWaste = 0;
    int totalVotes = 0;
    for (Set<int> district: districts) {    // O(n)
        Demographic districtInfo = Demographic(0, 0, 0);

        for (int precinct : district) {
            Demographic loc = map.getDemographic(precinct); // O(log n)
            districtInfo.dem += loc.dem;
            districtInfo.rep += loc.rep;
        }

        demWaste += demWasted(districtInfo.dem, districtInfo.rep);
        repWaste += repWasted(districtInfo.dem, districtInfo.rep);

        totalVotes += districtInfo.dem + districtInfo.rep;

    }
    return 100 * abs(demWaste - repWaste) / totalVotes;
}

/*
 * Returns if a certain plan is more gerrymandered than a given Efficiency Gap
 */
bool Gerrymander::isGerrymandered(Set<Set<int>>& districts, int margin) const {
    return howGerrymandered(districts) > margin;
}

/*
 * A function that returns a gerrymandered plan by repeatedly looking at different
 * skewed plans and seeing if they are valid plans.
 */
Set<Set<int>> Gerrymander::gerrymander(int totalDistricts, bool favorRep) const {
    Set<Set<int>> districts;

    while (!isValidPlan(districts, POPULATION_MARGIN)) {

        Set<int> ids = map.precinctSet();                       // O(n)
        districts.clear();

        gerrymanderHelper(districts, totalDistricts, favorRep, ids);    // O(n)
    }

    return districts;
}

/*
 * A helper function that individually builds districts for a plan
 */
void Gerrymander::gerrymanderHelper(Set<Set<int>>& districts, int totalDistricts, bool favorRep, Set<int>& ids) const {
    const int maxPop = map.totalPop() / totalDistricts;

    Vector<int> all = setToVector(ids);

    while (!ids.isEmpty()) {                            // O(n log n)
        int index = randomInteger(0, all.size() - 1);
        if (ids.contains(all.get(index))) {
            Set<int> curDistrict;
            Set<int> adj;
            Demographic demo = Demographic(0, 0, 0);
            createGerrymanderedDistrict(curDistrict, demo, all.get(index), maxPop, favorRep, adj, ids);

            districts.add(curDistrict);
        }
        all.remove(index);
    }
}

/*
 * How each district is constructed.
 *
 * A random open starting location is chosen, then it employs a greedy algorithm to
 * maximize wasted votes (not caring about the validity of the plan as a whole), which
 * it then grows the district, and repeats the process for every adjacent precinct.
 *
 * When it reaches the mean population of the region, then it finishes building the district.
 *
 * @param distirct the set of precincts (by reference) that the method adds to
 * @param demo the demographics (democrat voters, republican voters, total population) of the current district
 * @param curID the id of the precinct that the method is reviewing
 * @param maxPop the mean populatin of the entire region, which the district may go above,
 *          but may not add precincts to it when it does
 * @param favorRep boolean that determines whether it attempts to gerrymander for a Democrat or Republican advantage
 *          true - Republican, false - democrat
 * @param adj every adjacent precinct to the current district
 * @param ids the collection of all ids that are not already reserved for a district (open/free districts)
 *
 */
void Gerrymander::createGerrymanderedDistrict(Set<int>& district, Demographic& demo, int curID, const int& maxPop, bool favorRep, Set<int>& adj, Set<int>& ids) const {
    // Base Case => when the population of the district exceeds the maximum given for creation
    if (demo.pop > maxPop) {
        return;
    }

    // Adding current district information to the respective variables
    district.add(curID);
    Demographic curDemo = map.getDemographic(curID);
    demo.pop += curDemo.pop;
    demo.dem += curDemo.dem;
    demo.rep += curDemo.rep;
    ids.remove(curID);


    // Expanding the adjacent ids
    Set<int> adjSet = map.getAdjacentPrecincts(curID);  // O(log n)
    adj += adjSet;
    adj *= ids;         // This doesnt work for some reason

    // Base Case => district doesnt border any open precincts
    if (adj.isEmpty()) {
        return;
    }

    int maxWaste = 0;
    int maxID = adj.first();

    if (favorRep) {
        maxWaste = demWasted(demo.dem, demo.rep) - repWasted(demo.dem, demo.rep);
    } else {
        maxWaste = repWasted(demo.dem, demo.rep) - demWasted(demo.dem, demo.rep);
    }

    // Finds "highest" priority precinct (the one that maximizes EG for the party)
    for (int id : adj) {
        if (ids.contains(id)) { // adj *= ids doesnt work????
            Demographic tempDemo = map.getDemographic(id);      // O(log n)
            int totalDem = demo.dem + tempDemo.dem;
            int totalRep = demo.rep + tempDemo.rep;

            int candidateWaste = 0;

            if (favorRep) {
                candidateWaste = demWasted(totalDem, totalRep) - repWasted(totalDem, totalRep);
            } else {
                candidateWaste = repWasted(totalDem, totalRep) - demWasted(totalDem, totalRep);
            }

            if (candidateWaste > maxWaste) {
                maxWaste = candidateWaste;
                maxID = id;
            } else if (candidateWaste == maxWaste && randomBool()) {    // if there are >1 equal priority precicnts, it randomly picks 1
                maxID = id;
            }
        }
    }

    // Recursive Call
    createGerrymanderedDistrict(district, demo, maxID, maxPop, favorRep, adj, ids);
}

/*
 * Returns a gerrymandered district that is above a certin Efficiency Gap.
 *
 * It repeatedly generates random plans until one turns out to be gerrymandered.
 */
Set<Set<int>> Gerrymander::naiveGerrymander(int totalDistricts, int margin) const {
    Set<Set<int>> districts;
    while (!isGerrymandered(districts, margin)) {
        districts = createRandomPlan(totalDistricts);
    }
    return districts;
}

/*
 * Creates a random plan by making different plans and checking if they are
 * valid plans
 */
Set<Set<int>> Gerrymander::createRandomPlan(int totalDistricts) const {
    Set<Set<int>> districts;

    while (!isValidPlan(districts, POPULATION_MARGIN)) {
        Set<int> ids = map.precinctSet();
        districts.clear();


        createRandomPlanHelper(districts, totalDistricts, ids);
    }

    return districts;
}

/*
 * Creates a random plan by starting the districts in random areas.
 */
void Gerrymander::createRandomPlanHelper(Set<Set<int>>& districts, int totalDistricts, Set<int>& ids) const {
    const int max = (map.totalPop() / totalDistricts);

    Vector<int> all = setToVector(ids);

    while (!ids.isEmpty()) {                    // O(n)
        // Randomly picks an element from the Vector, if it is invalid, it removes it from the vector anyways
        int index = randomInteger(0, all.size() - 1);
        if (ids.contains(all.get(index))) {
            Set<int> curDistrict;
            int districtPop = 0;
            createRandomDistrict(curDistrict, all.get(index), districtPop, max, ids);

            districts.add(curDistrict);
        }
        all.remove(index);
    }
}

/*
 * It recursively creates a district, first by adding the current precinct, then choosing a random
 * adjacent precinct then repeating the process on that precinct until the mean population is reached.
 *
 * Since it utilizes DFS, it tends to create more snakey districts which are more likely to produce
 * gerrymandered districts.
 *
 * @param distirct the set of precincts (by reference) that the method adds to
 * @param curID the id of the precinct that the method is reviewing
 * @param districtPop the number (by reference) of people that reside within the current districts
 * @param maxPop the mean populatin of the entire region, which the district may go above,
 *          but may not add precincts to it when it does
 * @param ids the collection of all ids that are not already reserved for a district (open/free districts)
 */
void Gerrymander::createRandomDistrict(Set<int>& district, int curID, int& districtPop, const int& maxPop, Set<int>& ids) const {
    // Base Case
    if (districtPop >= maxPop) {
        return;
    }

    // Adds the current precinct to the district
    district.add(curID);
    districtPop += map.getDemographic(curID).pop;       // O(log n)
    ids.remove(curID);

    Set<int> adjSet = map.getAdjacentPrecincts(curID);  // O(log n)
    Vector<int> adj = setToVector(adjSet);

    // Recursive Case => DFS with random valid adjacent IDs
    while (!adj.isEmpty()) {
        int index = randomInteger(0, adj.size() - 1);
        if (ids.contains(adj.get(index))) {
            createRandomDistrict(district, adj.get(index), districtPop, maxPop, ids);
        }
        adj.remove(index);

    }
}

/*
 * Calculates wasted votes for the Republican Party (US)
 */
int Gerrymander::repWasted(int dem, int rep) const {
    if (dem > rep) {
        return rep;
    }

    return rep - dem - 1;
}

/*
 * Calculates wasted votes for the Democratic Party (US)
 */
int Gerrymander::demWasted(int dem, int rep) const {
    if (dem > rep) {
        return dem - rep - 1;
    }

    return dem;
}

/*
 * Converts a Set of integers to a Vector of integers
 */
Vector<int>Gerrymander::setToVector(Set<int>& intSet) const {
    Vector<int> vec;
    for (int element : intSet) {
        vec.add(element);
    }
    return vec;
}

/*-------- Testing and functions related to testing--------*/

Set<int> adjacentWithinDefaultJerry(int id) {
    Set<int> adj;

    if (id > 4) {
        adj.add(id - 5);
    }

    if (id % 5 > 0) {
        adj.add(id - 1);
    }

    if (id % 5 < 4) {
        adj.add(id + 1);
    }

    if (id < 45) {
        adj.add(id + 5);
    }

    return adj;
}

Set<Area*> defaultMapJerry() {
    Set<Area*> areas;
    int count = 0;

    for (int i = 0; i < 50; i++) {

        Area* area = new Area(count, 0, 0, 1, adjacentWithinDefaultJerry(count));
        if (i % 5 < 2) {
            area->dem = 1;
        } else {
            area->rep = 1;
        }

        areas.add(area);
        count++;
    }

    return areas;
}


PROVIDED_TEST("Using cracked districts to test validation") {
    Set<Area*> areas = defaultMapJerry();

    Gerrymander map;
    for (Area* loc : areas) {
        map.addArea(loc);
    }

    // Cracking
    Set<Set<int>> districts;
    for (int i = 0; i < 5; i++) {
        Set<int> district;
        for (int j = 0; j < 10; j++) {
            district.add(i * 10 + j);
        }
        districts.add(district);
    }

    EXPECT(map.isGerrymandered(districts, 7));
    EXPECT(map.isValidPlan(districts, 0.1));
}

PROVIDED_TEST("Test Generating random maps") {
    Set<Area*> areas = defaultMapJerry();

    Gerrymander map;
    for (Area* loc : areas) {
        map.addArea(loc);
    }

    Set<Set<int>> districts = map.createRandomPlan(5);
    Set<Set<int>> jerrysDistricts = map.naiveGerrymander(5, 15);

    EXPECT(map.isValidPlan(districts, POPULATION_MARGIN));
    EXPECT(map.isGerrymandered(jerrysDistricts, 15));

    jerrysDistricts = map.naiveGerrymander(2, 17);
    EXPECT(map.isGerrymandered(jerrysDistricts, 17));
}

PROVIDED_TEST("Test Generating intentionally gerrymandered maps") {
    Set<Area*> areas = defaultMapJerry();

    Gerrymander map;
    for (Area* loc : areas) {
        map.addArea(loc);
    }

    Set<Set<int>> jerrysDistricts = map.gerrymander(5, true);

    EXPECT(map.isGerrymandered(jerrysDistricts, 7));

    jerrysDistricts = map.gerrymander(2, false);
    EXPECT(map.isGerrymandered(jerrysDistricts, 7));

}

PROVIDED_TEST("Custom Precincts for verification (Vaguely based off of a small area in TX)") {


    Gerrymander map;
    map.addArea(new Area(50001, 121, 162, 636, {50002, 50007}));
    map.addArea(new Area(50002, 1011, 351, 2837, {50001, 50003, 50004}));
    map.addArea(new Area(50003, 234, 1141, 2527, {50002, 50005}));
    map.addArea(new Area(50004, 366, 452, 1223, {50002, 50005}));
    map.addArea(new Area(50005, 468, 611, 2168, {50002, 50004, 50003, 50006}));
    map.addArea(new Area(50006, 51, 275, 619, {50002, 50005, 50007}));
    map.addArea(new Area(50007, 121, 909, 2918, {50001, 50006}));

    Set<Set<int>> badPlan = {{50001, 50005}};
    EXPECT(!map.isValidPlan(badPlan, POPULATION_MARGIN));
    EXPECT_EQUAL(map.howGerrymandered(badPlan), NONE);
}

PROVIDED_TEST("Custom Precincts for plan generation (Vaguely based off of a small area in TX)") {

    Gerrymander map;
    map.addArea(new Area(50001, 121, 162, 636, {50002, 50007}));
    map.addArea(new Area(50002, 1011, 351, 2837, {50001, 50003, 50004}));
    map.addArea(new Area(50003, 234, 1141, 2527, {50002, 50005}));
    map.addArea(new Area(50004, 366, 452, 1223, {50002, 50005}));
    map.addArea(new Area(50005, 468, 611, 2168, {50002, 50004, 50003, 50006}));
    map.addArea(new Area(50006, 51, 275, 619, {50002, 50005, 50007}));
    map.addArea(new Area(50007, 121, 909, 2918, {50001, 50006}));


    Set<Set<int>> districts = map.createRandomPlan(3);
    Set<Set<int>> jerrysDistricts = map.naiveGerrymander(3, 7);

    EXPECT(map.isValidPlan(districts, POPULATION_MARGIN));
    EXPECT(map.isGerrymandered(jerrysDistricts, 7));

    jerrysDistricts = map.gerrymander(2, 7);
    EXPECT(map.isGerrymandered(jerrysDistricts, 7));
}

PROVIDED_TEST("Time Generating intentionally gerrymandered maps") {
    Set<Area*> areas = defaultMapJerry();

    Gerrymander map;
    for (Area* loc : areas) {
        map.addArea(loc);
    }

    TIME_OPERATION(2, map.gerrymander(2, true));
    TIME_OPERATION(4, map.gerrymander(4, true));
    TIME_OPERATION(5, map.gerrymander(5, true));
}

PROVIDED_TEST("Time Generating randomly gerrymandered maps") {
    Set<Area*> areas = defaultMapJerry();

    Gerrymander map;
    for (Area* loc : areas) {
        map.addArea(loc);
    }


    TIME_OPERATION(2, map.naiveGerrymander(5, 15));
    TIME_OPERATION(3, map.naiveGerrymander(5, 15));
    TIME_OPERATION(4, map.naiveGerrymander(5, 15));
    TIME_OPERATION(5, map.naiveGerrymander(5, 15));
    TIME_OPERATION(5, map.naiveGerrymander(5, 16));
    TIME_OPERATION(5, map.naiveGerrymander(5, 18));
    TIME_OPERATION(5, map.naiveGerrymander(5, 20));
    TIME_OPERATION(5, map.naiveGerrymander(5, 25));
}




