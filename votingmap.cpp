/* Christopher Lee (2022_08_07)
 *
 * This file is the implentatino of the
 * VotingMap interface.
 *
 * It has the functionality to read demographic
 * information from specific precincts, and
 * geographical informatino (adjacency).
 */

#include "votingmap.h"

#include <fstream>

#include "error.h"
#include "random.h"
#include "strlib.h"
#include "testing/SimpleTest.h"

#include "set.h"

// Default constructor
VotingMap::VotingMap() {}

/*
 * Destructor that goes through all IDs and deletes their associated Areas
 */
VotingMap::~VotingMap() {
    Vector<int> currentAreas = precinctVector();
    for (int key : currentAreas) {
        delete graph[key];
    }
}

/*
 * Adds an area to the map using {Area*->id : Area*}
 *
 * Updates totalPopulation as well as the total amount of elements the map holds
 */
void VotingMap::addArea(Area* loc) {
    if (contains(loc->id)) {
        return;
    }

    graph.put(loc->id, loc);

    numAreas++;
    totalPopulation += loc->pop;
}

/*
 * Returns number of elements in the map
 */
int VotingMap::size() const {
    return numAreas;
}

/*
 * Returns true if the map contains 0 elements
 */
bool VotingMap::isEmpty() const {
    return size() == 0;
}

/*
 * returns the total population within the map
 */
int VotingMap::totalPop() const {
    return totalPopulation;
}

/*
 * Returns a Set of ids contained in the map
 */
Set<int> VotingMap::precinctSet() const {
    Set<int> ids;
    Vector<int> fromGraph = precinctVector();
    for (int id : fromGraph) {
        ids.add(id);
    }
    return ids;
}

/*
 * Returns a Vector of ids contained in the map
 */
Vector<int> VotingMap::precinctVector() const {
    return graph.keys();
}

/*
 * Returns the Demographic Struct using data from the given Area from the given id
 */
Demographic VotingMap::getDemographic(int id) const {
    Area* cur = getArea(id);
    if (!cur) {
        return Demographic{-1, -1, -1};
    }

    return Demographic{cur->dem, cur->rep, cur->pop};
}

/*
 * Returns a Set of ids of the Areas that border the given Area
 */
Set<int> VotingMap::getAdjacentPrecincts(int id) const {
    Area* loc = getArea(id);

    return loc->adjAreas;
}

/*
 * Returns true if the Set of adjacent ids in a given Area (from id)
 * contains the id from adj (int adj)
 */
bool VotingMap::isAdjacdent(int id, int adj) const {
    Area* cur = getArea(id);
    if (!cur) {
        return false;
    }

    return cur->adjAreas.contains(adj);
}

/*
 * Returns true if the id is contained within the set
 */
bool VotingMap::contains(int id) const {
    return graph.containsKey(id);
}

/*
 * Returns a given area by its ID number, if no such number exists,
 * it throws an error.
 */
Area* VotingMap::getArea(int id) const {
    if (!graph.containsKey(id)) {
        error("No area for given id: " + integerToString(id));
    }

    return graph[id];
}


/************** TESTS **************/



Set<int> adjacentWithinDefault(int id) {
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

Set<Area*> defaultMap() {
    Set<Area*> areas;
    int count = 0;

    for (int i = 0; i < 50; i++) {

        Area* area = new Area(count, 0, 0, 1, adjacentWithinDefault(count));
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

PROVIDED_TEST("Testing default map contructor (i.e. the 10x5 one in all the infographics)") {
    //Hardcoded areas
    Set<Area*> areas = defaultMap();

    VotingMap map;
    for (Area* loc : areas) {
        map.addArea(loc);
    }

    EXPECT_EQUAL(map.getDemographic(5).dem, 1);
    EXPECT_EQUAL(map.getDemographic(2).dem, 0);
    EXPECT_EQUAL(map.getDemographic(2).rep, 1);
    EXPECT_EQUAL(map.getDemographic(0).dem, 1);
    EXPECT_EQUAL(map.getDemographic(0).rep, 0);

    EXPECT_EQUAL(map.isAdjacdent(0, -1), false);
    EXPECT_EQUAL(map.isAdjacdent(4, 5), false);
    EXPECT_EQUAL(map.isAdjacdent(12, 7), true);
};

