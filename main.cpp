// Includes
// ==========
// STL includes
#include <iostream>
#include <mpi.h>
#include <string>
#include <fstream>

// Boost includes
#include <boost/algorithm/string/join.hpp>

// Program includes
#include "lib/Process/const.h"
#include "lib/Process/ProcessMaster.h"
#include "lib/Process/ProcessSlave.h"

/**
 * Get command-line argument to program run.
 *
 * @param std::string arg Argument name (should be preceeded by a dash in the command line)
 * @param int argc Number of program call arguments
 * @param char* argv[] Program call arguments
 */
std::string getCommandArg( std::string arg, int argc, char* argv[] ) {
	for ( int i = 1; i < argc; i += 1 ) {
		if ( argv[ i ] == "-" + arg ) {
			return argv[ i + 1 ];
		}
	}

	return "";
}

/**
 * Main program.
 *
 * @param int argc Number of program call arguments
 * @param char* argv[] Program call arguments
 */
int main( int argc, char* argv[] ) {
	// Root process ID and process ID of current MPI process
	const int root_process_id = 0;
	int process_id;

	// Number of processes in use
	int num_processes;

	// Initialize MPI and fetch process ID
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &process_id );
	MPI_Comm_size( MPI_COMM_WORLD, &num_processes );

	// Create process
	Process* process;

	if ( process_id == root_process_id ) {
		process = new ProcessMaster( process_id, num_processes );
	}
	else {
		process = new ProcessSlave( process_id, num_processes );
	}

	// Handle command-line settings
	// =====
	
	// Verbosity
	bool verbose = false;

	if ( process->isMaster() || process_id == 1 ) {
		// Whether to output debugging info
		verbose = ( getCommandArg( "v", argc, argv ) != "0" );
		process->setVerbose( verbose );
	}

	// Settings regarding dynamic load balancing
	bool help_enabled = ( getCommandArg( "h", argc, argv ) != "0" );
	process->setEnableDynamicLoadBalancing( help_enabled );

	if ( help_enabled ) {
		// Help request degree threshold for requesting help from other processes
		int help_request_threshold = atoi( getCommandArg( "t", argc, argv ).c_str() );
		process->setHelpRequestThreshold( help_request_threshold );

		if ( process_id == 1 && verbose ) {
			printf( "Degree threshold: %d\n", help_request_threshold );
		}
	}

	// Output file
	if ( process->isMaster() ) {
		std::string output_fname = getCommandArg( "o", argc, argv );

		if ( output_fname == "1" ) {
			output_fname = "log/log.txt";
		}

		process->setResultsOutputFileName( output_fname );
	}

	// Input file path
	process->handleInputFilePath( getCommandArg( "d", argc, argv ), getCommandArg( "f", argc, argv ) );

	// Run each process
	// =====
	process->run();

	// Quit MPI
	MPI_Finalize();

	return 0;
}