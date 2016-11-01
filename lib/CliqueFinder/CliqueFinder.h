#ifndef CLIQUEFINDER_H
#define CLIQUEFINDER_H

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
#include "../Process/ProcessSlave.h"
#include "../Graph/Graph.h"

// Temporary class definitions
// ==========
class ProcessSlave;

class CliqueFinder {

	private:
		/**
		 * Corresponding multiprocessing process
		 */
		ProcessSlave *process;

		/**
		 * The graph on which calculations have to be done
		 */
		Graph *graph;

		/**
		 * Number of unique cliques per clique size
		 * Format: [clique_size] => [num_cliques]
		 */
		std::map<int,long long> dfs_cliques_count;

		/**
		 * Parallel computation settings
		 */
		int process_id;
		int num_processes;

		/**
		 * Dynamic load balancing settings
		 */
		bool help_enabled;
		bool help_disable_switch = false;
		int help_degree_threshold = 0;

	public:
		// Help requests statistics
		long long num_help_requests_sent = 0;
		long long num_help_requests_sent_rejected = 0;
		long long num_help_requests_sent_accepted = 0;
		
		/**
		 * Constructor
		 *
		 * @param SlaveProcess *process Process associated with this part of calculations
		 */
		CliqueFinder( ProcessSlave *process, Graph *graph ) : process(process), graph(graph) {}

		/**
		 * Initialize parallel computation configuration for determining the part of computation that
		 * should be covered by each particular process.
		 *
		 * @param int process_id_local Process ID
		 * @param int num_processes_local Number of processes working on the calculations in parallel
		 * @param bool help_enabled_local Whether dynamic load balancing is enabled
		 */
		void initParallelComputation( int process_id_local, int num_processes_local, bool help_enabled_local );

		/**
		 * Find all cliques using a distributed depth-first search approach.
		 */
		void findAllCliquesDFS();

		/**
		 * Traverse down the clique-finding depth-first search tree. Given a base (k-1)-clique and all nodes that can be added to form a k-clique,
		 * it traverses down the clique tree and finds (k+1)-cliques.
		 *
		 * @param int depth Depth of new search (k+1)
		 * @param vector<int> base_nodes Node list of known base clique (k-1)
		 * @param vector<int> potential_nodes List of nodes that can, individually, all be added to the (k-1)-clique to form a k-clique.
		 */
		void findCliquesDFS( int depth, std::vector<int> base_nodes, std::vector<int> potential_nodes, int start = -1, int end = -1 );

		/**
		 * Retrieve a vector of clique counts. This should be called after processing has completed.
		 *
		 * @return vector<long long> Vector of clique counts (keys are clique sizes, values are counts)
		 */
		std::vector<long long> getCliquesCountsVector();

		/**
		 * Activate a temporary switch to disable help requests. When the switch is activated, no help
		 * will be requested for the next node to be evaluated, after which help requests will be enabled
		 * again. This ensures that no infinite loop will occur where help is continually requested.
		 */
		void temporarilyDisableHelp();

		/**
		 * Enable or disable dynamic load balancing.
		 *
		 * @param bool enable Whether to enable (true) or disable (false) help requests
		 */
		void setHelpEnabled( bool enable );

		/**
		 * Get the threshold of the number of nodes that have to be in a node list for a DFS process to request
		 * help from another process.
		 *
		 * @return int Threshold number of nodes
		 */
		int getHelpRequestThreshold();

		/**
		 * Set the help request node list size threshold.
		 *
		 * @param int threshold Help request threshold
		 */
		void setHelpRequestThreshold( int threshold );

		/**
		 * Calculate and subsequently set the help request node list size threshold.
		 */
		void calculateHelpRequestThreshold();

		/**
		 * Get the number of help requests that were sent by this process
		 *
		 * @return int Number of help requests sent
		 */
		long long getNumHelpRequestsSent();

		/**
		 * Get the number of help requests that were sent by this process and then accepted by another process
		 *
		 * @return int Number of help requests sent and accepted
		 */
		long long getNumHelpRequestsSentAccepted();

		/**
		 * Get the number of help requests that were sent by this process and then rejected by all other processes
		 *
		 * @return int Number of help requests sent and rejected
		 */
		long long getNumHelpRequestsSentRejected();
};

#endif
