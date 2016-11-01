// Includes
// ==========
// STL includes

// Program includes
#include "Process.h"

bool Process::isMaster() {
	return false;
}

void Process::initMPI() {
	// Get MPI process processor name and store it
	int process_processor_name_length;
	MPI_Get_processor_name( process_processor_name, &process_processor_name_length );
}

void Process::setVerbose( bool v ) {
	verbose = v;
}

void Process::setEnableDynamicLoadBalancing( bool enabled ) {
	help_enabled = enabled;
}

void Process::setHelpRequestThreshold( int threshold ) {
	help_request_threshold = threshold;
}

void Process::setResultsOutputFileName( std::string output_fname ) {
	results_output_fname = output_fname;
}