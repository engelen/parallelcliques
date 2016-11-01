#ifndef PROCESSSLAVE_H
#define PROCESSSLAVE_H

// Includes
// ==========
// STL includes

// Program includes
#include "Process.h"
#include "../Graph/Graph.h"
#include "../CliqueFinder/CliqueFinder.h"

// Temporary class definitions
// ==========
class Graph;
class CliqueFinder;

class ProcessSlave : public Process {
	protected:
		/**
		 * The graph on which calculations have to be done
		 */
		Graph *graph;
		
		/**
		 * The object implementing the clique finding using the graph
		 */
		CliqueFinder *cliquefinder;

		/**
		 * ID of the process currently helping or available for helping complete this process's subtask
		 */
		int helper_process_id;

	public:
		ProcessSlave( int process_id, int num_processes ) : Process( process_id, num_processes ) {}
		
		/**
		 * @see Process::run()
		 */
		void run();

		/**
		 * @see Process:handleInputFilePath()
		 */
		void handleInputFilePath( std::string dataset_name, std::string file_path );

		/**
		 * Receive file path from master process
		 */
		void receiveInputFilePath();

		/**
		 * Construct the graph object
		 */
		void createGraph();

		/**
		 * Read the graph data from the input file specified
		 */
		void readGraphFromInputFile();

		/**
		 * Construct the clique finder object using the constructed graph
		 *
		 * @param Graph *g Graph object to pass to the clique finder
		 */
		void createCliqueFinder( Graph *g );

		/**
		 * Start the initial run of the clique finding algorithm
		 */
		void findCliques();

		/**
		 * Send a help request to other processes through the main process. Returns true if other processes are available,
		 * false otherwise. If another process is available, it is immediately reserved by the master process. Thus, any help
		 * request formed through this method should be followed by granting the help request through the grantHelp() method.
		 *
		 * If a process is available for help, store the process ID in helper_process_id.
		 *
		 * @return bool Whether other processes are available
		 */
		bool requestHelp();

		/**
		 *
		 */
		void grantHelp( int depth, std::vector<int> base_nodes, std::vector<int> potential_nodes, int helper_start, int helper_end );

		/**
		 * Transmit a message to the master process that the slave process has completed its initially assigned subtask
		 */
		void transmitSubtaskCompleted();

		/**
		 * Continually probe the master process to see if the slave process can assist other processes. If it can, take
		 * over part of another process's subtask
		 */
		void standbyForHelpRequest();

		/**
		 * Transmit the list of cliques per clique size to the master process
		 */
		void transmitFindCliquesResult();
};

#endif
