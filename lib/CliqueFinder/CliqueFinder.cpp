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

// Boost includes
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/join.hpp>

// Program includes
#include "CliqueFinder.h"

void CliqueFinder::initParallelComputation( int process_id_local, int num_processes_local, bool help_enabled_local ) {
	process_id = process_id_local;
	num_processes = num_processes_local;
	help_enabled = help_enabled_local;
}

void CliqueFinder::findAllCliquesDFS() {
	// Store adjacency list of edges in increasing order (i.e. no edge 5-2 exists)
	std::map<int, std::vector<int> > baselist;

	// Loop over edges to construct lists of base nodes and potential nodes
	for ( const auto& edge : graph->getEdgeList() ) {
		int node_source_id = edge.first;
		int node_target_id = edge.second;

		// Ensure that the source node should be processed in this process
		if ( node_source_id % num_processes != process_id - 1 ) {
			continue;
		}
		
		// Maintain node ordering
		if ( node_source_id < node_target_id ) {
			baselist[ node_source_id ].push_back( node_target_id );
		}
	}

	// Outer call of first iteration of the main algorithm
	for ( const auto &node : baselist ) {
		if ( node.second.size() <= 1 ) {
			continue;
		}

		std::vector<int> base_nodes;
		base_nodes.push_back( node.first );
		findCliquesDFS( 3, base_nodes, node.second );
	}
}

std::vector<long long> CliqueFinder::getCliquesCountsVector() {
	int num_sizes_processed = 0;
	int i = 0;
	std::vector<long long> cliques_counts;

	while ( num_sizes_processed < dfs_cliques_count.size() ) {
		if ( dfs_cliques_count.find( i ) == dfs_cliques_count.end() ) {
			cliques_counts.push_back( 0 );
			i++;
			continue;
		}

		cliques_counts.push_back( dfs_cliques_count[ i ] );
		i++;
		num_sizes_processed++;
	}

	return cliques_counts;
}

void CliqueFinder::temporarilyDisableHelp() {
	help_disable_switch = true;
	help_enabled = false;
}

void CliqueFinder::setHelpEnabled( bool enable ) {
	help_enabled = enable;
}

int CliqueFinder::getHelpRequestThreshold() {
	if ( help_degree_threshold == 0 ) {
		calculateHelpRequestThreshold();
	}

	return help_degree_threshold;
}

void CliqueFinder::setHelpRequestThreshold( int threshold ) {
	help_degree_threshold = threshold;
}

void CliqueFinder::calculateHelpRequestThreshold() {
	setHelpRequestThreshold( std::max( 2, int( graph->getNumEdges() / graph->getNumNodes() * 5 ) ) );
}

void CliqueFinder::findCliquesDFS( int depth, std::vector<int> base_nodes, std::vector<int> potential_nodes, int start, int end ) {
	if ( start == -1 ) {
		start = 0;
		end = potential_nodes.size();
	}

	// Loop over potential nodes
	for ( int i = start; i < end; i++ ) {
		// If the help disable switch is turned on, no help will be requested. This ensures that no infinite loop
		// will occur where help is continually requested
		if ( help_disable_switch ) {
			help_enabled = true;
			help_disable_switch = false;
		}
		else if ( help_enabled && end - i > getHelpRequestThreshold() ) {
			num_help_requests_sent++;

			// Request help through process
			if ( process->requestHelp() ) {
				// Help has been granted, so we can pass the remaining part of this DFS branch to the helper process
				// through the current slave process
				process->grantHelp( depth, base_nodes, potential_nodes, i, end );
				num_help_requests_sent_accepted++;
				return;
			}

			num_help_requests_sent_rejected++;
		}

		// Core of algorithm
		std::vector<int> new_base_nodes;
		std::vector<int> new_potential_nodes;

		// Internal loop: check for edges between potential nodes
		for ( int j = i + 1; j < potential_nodes.size(); j++ ) {
			if ( graph->isEdge( potential_nodes[ i ], potential_nodes[ j ] ) ) {
				// Clique was found
				new_potential_nodes.push_back( potential_nodes[ j ] );
				dfs_cliques_count[ depth ]++;
			}
		}

		// Move down the BFS tree
		if ( new_potential_nodes.size() > 1 ) {
			new_base_nodes.push_back( potential_nodes[ i ] );
			findCliquesDFS( depth + 1, new_base_nodes, new_potential_nodes );
		}
	}
}

long long CliqueFinder::getNumHelpRequestsSent() {
	return num_help_requests_sent;
}

long long CliqueFinder::getNumHelpRequestsSentAccepted() {
	return num_help_requests_sent_accepted;
}

long long CliqueFinder::getNumHelpRequestsSentRejected() {
	return num_help_requests_sent_rejected;
}
