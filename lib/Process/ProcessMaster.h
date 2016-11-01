#ifndef PROCESSMASTER_H
#define PROCESSMASTER_H

// Includes
// ==========
// STL includes
#include <vector>

// Program includes
#include "Process.h"

class ProcessMaster : public Process {
	protected:
		/**
		 * The status of each process, either PROCESS_IDLE, PROCESS_RUNNING or PROCESS_HELPING
		 * PROCESS_IDLE: Process has completed its assigned part of the task, and is waiting for help requests
		 * PROCESS_RUNNING: Process is performing its originally assigned part of the task
		 * PROCESS_HELPING: Process is assisting another process with its assigned task
		 */
		std::vector<int> processes_status;

		/**
		 * Number of processes that are currently active (i.e. either running or helping)
		 */
		int num_processes_active;

	public:
		ProcessMaster( int process_id, int num_processes ) : Process( process_id, num_processes ) {}

		/**
		 * @see Process::run()
		 */
		void run();

		/**
		 * @see Process::isMaster()
		 */
		bool isMaster();

		/**
		 * @see Process:handleInputFilePath()
		 */
		void handleInputFilePath( std::string dataset_name, std::string file_path = "" );

		/**
		 * Construct file path from dataset name or full. If a full file path is provided, it is used to load the graphs.
		 * Otherwise, the file path is constructed from the base data files folder (./data/input/) and the dataset name.
		 *
		 * @param std::string dataset_name Data set name used in file name
		 * @param std::string file_path Optional. Full file path to the input file
		 */
		void constructInputFilePath( std::string dataset_name, std::string file_path = "" );

		/**
		 * Transmit an input file path to all slave processes
		 *
		 * @param std::string file_path Optional. File path. Defaults to graph_input_fname
		 */
		void transmitInputFilePath( std::string file_path = "" );

		/**
		 * Manage the clique-finding process. Handle and relay help requests from processes, and maintain status of processes
		 */
		void manageFindCliques();

		/**
		 * Reset variables indicating the status of each process and the number of active processes
		 */
		void resetProcessesStatus();

		/**
		 * Manage load balancing, receiving and relaying messages from slave processes requesting help or indicating
		 * idleness and availability for helping other processes
		 */
		void manageLoadBalancing();

		/**
		 * Transmit messages to all slave processes that all clique finding has been completed
		 */
		void transmitFindCliquesComplete();

		/**
		 * Aggregate results of child processes
		 */
		void aggregateResults();
};

#endif
