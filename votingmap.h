/* Christopher Lee (2022_08_07)
 *
 * This file outlines the VotingMap interface (for storing data)
 * and the Demographic, Area structs
 *
 * The VotingMap interface vaguely resembles an adjacency graph,
 * which has been altered to include demographical and identification
 * information.
 *
 * The Demographic struct holds party affiliation and totalPopulation data.
 *
 * The Area struct holds data for a given precinct => Demographic data,
 * identification number, and the Set of precincts that are adjacent/border
 * the given precinct
 */

#pragma once

#ifndef VOTINGMAP_H
#define VOTINGMAP_H


#include "map.h"
#include "set.h"

struct Demographic {
    // Votes for the Democratic Party
    int dem;
    // Votes for the Republican Party
    int rep;
    // Total population in the Area
    int pop;

    // Constructor that initializes every variable to the User's specification
    Demographic(int democrat, int republican, int population) {
        dem = democrat;
        rep = republican;
        pop = population;
    };
};

struct Area
{
    // An integer that identifies the Area for lookup
    int id;
    // Votes for the Democratic Party
    int dem;
    // Votes for the Republican Party
    int rep;
    // Total population in the Area
    int pop;
    // A collection of all the Areas that physically border this Area
    Set<int> adjAreas;

    // Constructor that initializes every variable to the User's specification
    Area(int idNum, int democrat, int republican, int population, Set<int> adjacentAreas) {
        id = idNum;
        dem = democrat;
        rep = republican;
        pop = population;
        adjAreas = adjacentAreas;
    };

    /* Constructor that initializes every variable to the User's specification,
     * using a Demographic struct
     */
    Area(int idNum, Demographic demo, Set<int> adjacentAreas) {
        Area(idNum, demo.dem, demo.rep, demo.pop, adjacentAreas);
    };
};

class VotingMap
{
public:
    // Default constructor that implicitly initializes member variables with their default values
    VotingMap();

    // Destructor that deletes all the used memory on the heap
    ~VotingMap();

    // Adds an area to the Graph
    void addArea(Area* loc);

    // Returns how many elements are in the graph
    int size() const;
    // Returns if there are no elements in the graph
    bool isEmpty() const;
    // returns the total population of all Areas combined
    int totalPop() const;
    // Returns the id numbers of all the elements in the graph, transformed into a set
    Set<int> precinctSet() const;
    // returns the id number of all the elements in the graph
    Vector<int> precinctVector() const;

    // Returns a Demographic struct of a given Area with an id of the input
    Demographic getDemographic(int id) const;
    // Returns the adjacent Precincts/Areas of given Area with an id of the input
    Set<int> getAdjacentPrecincts(int id) const;
    // Returns whether 2 Areas border each other
    bool isAdjacdent(int id, int adj) const;
    // Returns whether the given element exists in the graph
    bool contains(int id) const;
private:
    /* A adjanency Graph containing Area structs
     *
     * The only caveat is that it doesnt immediately relate the element to elements that are adjacent;
     * instead, it relates an id to an Area that contains information of what Areas are adjacent
     *
     * id => Area* => Set<id> AdjacentElements
     */
    Map<int, Area*> graph;
    // Totoal number of elements in the graph
    int numAreas;
    // The combined population of all the element in the graph
    int totalPopulation;

    // Returns a given Area from the graph
    Area* getArea(int id) const;
};

#endif // VOTINGMAP_H
