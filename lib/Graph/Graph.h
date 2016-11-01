#ifndef GRAPH_H
#define GRAPH_H

// Includes
// ==========
// STL includes
#include <vector>
#include <string>
#include <map>
#include <unordered_set>

// Boost includes
#include <boost/functional/hash.hpp>

// Program includes
#include "node.h"

// Temporary class definitions
// ==========
class ProcessSlave;

// Type definitions
// ==========
typedef std::pair<int, int> Edge;
typedef std::unordered_set<
	Edge,
	boost::hash< std::pair<int, int> >
> EdgeList;
typedef std::map<int, std::vector<int> > AdjacencyList;
typedef std::map<int, node> NodeList;

class Graph {

	private:
		/**
		 * Adjacency list of graph, i.e. list of neighbour nodes per node.
		 * Format of entry: [source_id] => ( [target_id-0], ..., [target_id-i] )
		 */
		AdjacencyList adjacencylist;

		/**
		 * Edge list of graph
		 * Format of entry: ([source_id], [target_id])
		 */
		EdgeList edgelist;

		/**
		 * List of nodes in graph
		 * Format of entry: [node_id] => [node_object]
		 */
		NodeList nodelist;

		/**
		 * List of degrees per node
		 * Format: [node_id] => [degree]
		 */
		std::map<int, int> nodes_degrees;

		/**
		 * Number of nodes and number of edges in the graph
		 */
		int num_nodes = 0;
		int num_edges = 0;

		/**
		 * Estimated power-law exponent of theg raph
		 */
		float powerlaw_exponent = 0;

	public:
		/**
		 * Read a list of edges from a file.
		 * Each line should contain a source node ID and a target node ID, separated by a tab character.
		 *
		 * @param std::string fname Input file name to read edges from
		 */
		bool readEdgeList( std::string fname );

		/**
		 * Add a node to the graph.
		 *
		 * @param int nid Node ID
		 */
		void addNode( int nid );

		/**
		 * Check whether a certain node exists.
		 *
		 * @param int nid Node ID
		 * @return bool Whether the node exists
		 */
		bool isNode( int nid );

		/**
		 * Retrieve a node object by its node ID
		 *
		 * @param int nid Node ID
		 * @return node Node object
		 */
		node getNode( int nid );

		/**
		 * Add an edge between two nodes in the graph.
		 *
		 * @param int node_source Source node ID
		 * @param int node_target Target node ID
		 */
		bool addEdge( int node_source, int node_target );
		
		/**
		 * Check whether an edge exists between two nodes.
		 *
		 * @param int node_source Source node ID
		 * @param int node_target Target node ID
		 * @return bool Whether an edge exists between the source and target node
		 */
		bool isEdge( int node_source, int node_target );

		EdgeList getEdgeList();

		/**
		 * Get the degree of a node.
		 *
		 * @param int nid Node ID
		 * @return int Node degree
		 */
		int getNodeDegree( int nid );

		/**
		 * Get the number of nodes in the graph
		 *
		 * @return int Number of nodes
		 */
		int getNumNodes();

		/**
		 * Get the number of edges in the graph
		 *
		 * @return int Number of edges
		 */
		int getNumEdges();

		/**
		 * Estimate the power-law exponent for the current network.
		 *
		 * @return float Estimated power-law exponent
		 */
		float getPowerLawExponent();
};

#endif
