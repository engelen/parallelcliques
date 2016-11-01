// Includes
// ==========
// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_set>
#include <math.h>
#include <algorithm>

// Program includes
#include "Graph.h"
#include "node.h"

bool Graph::readEdgeList( std::string fname ) {
	// Open input file
	std::ifstream fl_input( fname );

	if ( ! fl_input ) {
		printf( "ERROR: The input file '%s' does not exist.\n", fname.c_str() );
		return false;
	}

	// Delimiters for rows and fields
	char const row_delimiter = '\n';
	char const field_delimiter = '\t';

	// Loop over lines
	for ( std::string row; std::getline( fl_input, row, row_delimiter ); ) {
		std::istringstream row_fields( row );
		std::vector<std::string> fields;

		for ( std::string row_field; getline( row_fields, row_field, field_delimiter ); ) {
			fields.push_back( row_field );
		}

		int node_source = std::atoi( fields[0].c_str() );
		int node_target = std::atoi( fields[1].c_str() );

		addEdge( node_source, node_target );
	}

	return true;
}

void Graph::addNode( int nid ) {
	node nd;
	nd.id = nid;
	nd.degree = 0;

	if ( ! isNode( nid ) ) {
		nodelist[ nid ] = nd;
		num_nodes++;
	}
}

bool Graph::isNode( int nid ) {
	return ( nodelist.find( nid ) != nodelist.end() );
}

node Graph::getNode( int nid ) {
	return nodelist[ nid ];
}

bool Graph::addEdge( int node_source, int node_target ) {
	if ( isEdge( node_source, node_target ) ) {
		return false;
	}

	// Add nodes to graph
	addNode( node_source );
	addNode( node_target );

	adjacencylist[ node_source ].push_back( node_target );
	adjacencylist[ node_target ].push_back( node_source );

	edgelist.insert( std::make_pair( node_source, node_target ) );
	edgelist.insert( std::make_pair( node_target, node_source ) );

	node n1 = getNode( node_source );
	node n2 = getNode( node_target );
	n1.degree++;
	n2.degree++;

	num_edges++;

	return true;
}

bool Graph::isEdge( int node_source, int node_target ) {
	if ( edgelist.count( std::make_pair( node_source, node_target ) ) == 1 ) {
		return true;
	}

	return false;
}

EdgeList Graph::getEdgeList() {
	return edgelist;
}

int Graph::getNodeDegree( int nid ) {
	return adjacencylist[ nid ].size();
}

int Graph::getNumNodes() {
	return num_nodes;
}

int Graph::getNumEdges() {
	return num_edges;
}

float Graph::getPowerLawExponent() {
	// Return the existing power-law exponent if it was estimated already
	if ( powerlaw_exponent > 0 ) {
		return powerlaw_exponent;
	}

	// Estimated power-law exponent
	powerlaw_exponent = 0;

	// Minimal degree found in the network
	int mindegree = -1;

	for ( int i = 0; i < nodelist.size(); i++ ) {
		int degree = getNodeDegree( i );

		if ( degree < mindegree || mindegree == -1 ) {
			mindegree = degree;
		}
	}

	// Calculate power-law exponent
	for ( int i = 0; i < nodelist.size(); i++ ) {
		powerlaw_exponent += log( ( (float) getNodeDegree( i ) ) / mindegree );
	}

	powerlaw_exponent = 1 + nodelist.size() / powerlaw_exponent;

	return powerlaw_exponent;
}
