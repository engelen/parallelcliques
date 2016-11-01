#ifndef PROCESS_H
#define PROCESS_H

// Includes
// ==========
// MPI includes
#include <mpi.h>

// Program includes
#include "../ProgressMonitor/ProgressMonitor.h"

class Process {
	protected:
		/**
		 * MPI ID of the process
		 */
		int process_id;

		/**
		 * Number of processes in use
		 */
		int num_processes;

		/**
		 * MPI processor name of the process
		 */
		char process_processor_name[MPI_MAX_PROCESSOR_NAME];

		/**
		 * File name of the graph data file
		 */
		std::string graph_input_fname;

		/**
		 * Whether debugging output should be presented
		 */
		bool verbose = false;

		/**
		 * Whether dynamic load balancing is enabled
		 */
		bool help_enabled = false;

		/**
		 * The degree threshold for dynamic load balancing
		 */
		int help_request_threshold;

		/**
		 * The file name to which any results should be written
		 */
		std::string results_output_fname = "";

		/**
		 * Progress monitor object to track time
		 */
		ProgressMonitor pm;

	public:
		Process( int process_id, int num_processes ): process_id(process_id), num_processes(num_processes) {}

		/**
		 * Run the core of the process. Should be implemented by child and master, respectively
		 */
		virtual void run() = 0;

		/**
		 * Construct or fetch the input file path
		 *
		 * @param std::string dataset_name Data set name used in file name
		 * @param std::string file_path Optional. Full file path to the input file
		 */
		virtual void handleInputFilePath( std::string dataset_name, std::string file_path = "" ) = 0;

		/**
		 * Returns whether this process is the master process
		 *
		 * @return bool
		 */
		virtual bool isMaster();

		/**
		 * Fetch remaining necessary MPI details
		 */
		void initMPI();

		/**
		 * Change the verbose setting. Use true if debugging information should be outputted for this process, false otherwise
		 *
		 * @param bool v New verbose setting
		 */
		void setVerbose( bool v );

		/**
		 * Enable/disable dynamic load balancing
		 *
		 * @param bool Whether dynamic load balancing should be enabled (true) or disabled (false)
		 */
		void setEnableDynamicLoadBalancing( bool enabled );

		/**
		 * Change the degree threshold at which a help request for dynamic load balancing may be initiated
		 *
		 * @param int threshold New threshold
		 */
		void setHelpRequestThreshold( int threshold );

		/**
		 * Change the output file name to which any results should be written. Leave empty or at "0" to
		 * prevent writing output to any file
		 *
		 * @param std::string output_fname Path of output file
		 */
		void setResultsOutputFileName( std::string output_fname );
};

#endif
