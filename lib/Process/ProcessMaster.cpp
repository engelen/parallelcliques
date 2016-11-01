// Includes
// ==========
// STL includes
#include <fstream>

// Program includes
#include "ProcessMaster.h"
#include "const.h"

void ProcessMaster::run() {
	// Initialize MPI settings
	initMPI();

	// Transmit dataset input file path to child processes
	transmitInputFilePath();

	// Oversee clique finding process
	manageFindCliques();
}

void ProcessMaster::manageFindCliques() {
	// Reset indicators of process status and number of active processes
	resetProcessesStatus();

	// Manage load balancing until clique finding process is complete
	manageLoadBalancing();

	// At this point, all processes have completed their jobs, and therefore the entire graph has
	// been processed. Relay this information to processes so they can finalize.
	transmitFindCliquesComplete();

	// Aggregate results
	aggregateResults();
}

void ProcessMaster::manageLoadBalancing() {
	int buff;

	// Main loop for governing processes requesting assistance from other processes to take over
	// part of their computational needs
	while ( num_processes_active > 0 ) {
		int probe_flag;
		MPI_Status probe_status;

		// Probe for any processes that have completed
		MPI_Iprobe( MPI_ANY_SOURCE, MPI_TAG_GRAPH_PROCESSING_DONE, MPI_COMM_WORLD, &probe_flag, &probe_status );

		if ( probe_flag ) {
			MPI_Recv( &buff, 1, MPI_INT, probe_status.MPI_SOURCE, probe_status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

			num_processes_active--;
			processes_status[ probe_status.MPI_SOURCE ] = PROCESS_IDLE;

			// Output new process statuses (should be and was commented out when running final experiments)
			if ( verbose ) {
				printf( "\r" );

				for ( int i = 1; i < num_processes; i++ ) {
					printf( "%d ", processes_status[ i ] );
				}

				fflush( stdout );
			}
		}

		// Probe for any processes requesting balancing
		MPI_Iprobe( MPI_ANY_SOURCE, MPI_TAG_GRAPH_REQUEST_HELP, MPI_COMM_WORLD, &probe_flag, &probe_status );

		if ( probe_flag ) {
			int response_buff = 0;
			MPI_Recv( &buff, 1, MPI_INT, probe_status.MPI_SOURCE, probe_status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

			// Check whether there are any idle processes
			if ( num_processes_active < num_processes - 1 ) {
				// Find the first idle process and change its status
				for ( int i = 1; i < num_processes; i++ ) {
					if ( processes_status[ i ] == PROCESS_IDLE ) {
						response_buff = i;
						processes_status[ i ] = PROCESS_HELPING;
						num_processes_active++;

						// Output new process statuses (should be and was commented out when running final experiments)
						if ( verbose ) {
							printf( "\r" );

							for ( int i = 1; i < num_processes; i++ ) {
								printf( "%d ", processes_status[ i ] );
							}

							fflush( stdout );
						}

						break;
					}
				}
			}

			// Tell the process requesting help whether its request has been granted, and if so, pass
			// the ID of the process that will help
			MPI_Send( &response_buff, 1, MPI_INT, probe_status.MPI_SOURCE, MPI_TAG_GRAPH_REQUEST_HELP_RESPONSE, MPI_COMM_WORLD );
		}
	}
}

void ProcessMaster::transmitFindCliquesComplete() {
	for ( int i = 1; i < num_processes; i++ ) {
		int buff = 1;
		MPI_Send( &buff, 1, MPI_INT, i, MPI_TAG_ALL_PROCESSING_COMPLETED, MPI_COMM_WORLD );
	}
}

void ProcessMaster::resetProcessesStatus() {
	// Set all process statuses to "running"
	processes_status.resize( num_processes );
	std::fill( processes_status.begin(), processes_status.end(), PROCESS_RUNNING );

	// Set the status of this process (the master process) to idle. The master process is regarded a process, but is
	// not involved in the actual computation of the number of cliques
	processes_status[0] = PROCESS_IDLE;

	// Update the number of currently active processes
	num_processes_active = num_processes - 1;
}

void ProcessMaster::handleInputFilePath( std::string dataset_name, std::string file_path ) {
	// Construct the file path and transmit it to all child processes
	constructInputFilePath( dataset_name, file_path );
	transmitInputFilePath();
}

void ProcessMaster::constructInputFilePath( std::string dataset_name, std::string file_path ) {
	// Construct file name
	std::string fname_base = "data/input/";
	std::string input_fname = "";

	// Construct from dataset id
	if ( dataset_name != "" ) {
		input_fname = fname_base + dataset_name + ".tsv";
	}

	// Construct if full file path is given
	if ( file_path != "" ) {
		input_fname = file_path;
	}

	// Store file name
	graph_input_fname = input_fname;

	// Output debugging information
	if ( verbose ) {
		printf( "Graph file path constructed: \"%s\"\n", graph_input_fname.c_str() );
	}
}

void ProcessMaster::transmitInputFilePath( std::string file_path ) {
	// Default file path
	if ( file_path == "" ) {
		file_path = graph_input_fname;
	}

	// Send file name to all slave processes
	for ( int i = 1; i < num_processes; i++ ) {
		MPI_Send( file_path.c_str(), strlen( file_path.c_str() ) + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD );
	}
}

void ProcessMaster::aggregateResults() {
	// Total clique counts for graph
	std::map<int,long long> cliques_counts;

	// List of clique counts per clique size
	std::vector<long long> cliques_counts_slave;

	// Help request counts
	long long num_help_requests_sent = 0;
	long long num_help_requests_sent_accepted = 0;
	long long num_help_requests_sent_rejected = 0;

	// Add clique counts for slave processes
	for ( int i = 1; i < num_processes; i++ ) {
		MPI_Status status;
		int max_clique_size;

		// Buffer number of clique sizes
		MPI_Probe( i, MPI_TAG_CLIQUE_COUNTS, MPI_COMM_WORLD, &status );
		MPI_Get_count( &status, MPI_LONG_LONG, &max_clique_size );

		// Reserve space for cliques
		cliques_counts_slave.resize( max_clique_size );

		// Receive number of cliques per clique size
		MPI_Recv( &cliques_counts_slave[0], max_clique_size, MPI_LONG_LONG, i, MPI_TAG_CLIQUE_COUNTS, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		for ( int j = 0; j < max_clique_size; j++ ) {
			cliques_counts[ j ] += cliques_counts_slave[ j ];
		}

		// Receive help request counts
		long long num_help_requests_sent_slave;
		long long num_help_requests_sent_accepted_slave;
		long long num_help_requests_sent_rejected_slave;
		MPI_Recv( &num_help_requests_sent_slave, 1, MPI_LONG_LONG, i, MPI_TAG_HELP_REQUESTS_SENT, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &num_help_requests_sent_accepted_slave, 1, MPI_LONG_LONG, i, MPI_TAG_HELP_REQUESTS_SENT_ACCEPTED, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &num_help_requests_sent_rejected_slave, 1, MPI_LONG_LONG, i, MPI_TAG_HELP_REQUESTS_SENT_REJECTED, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		num_help_requests_sent += num_help_requests_sent_slave;
		num_help_requests_sent_accepted += num_help_requests_sent_accepted_slave;
		num_help_requests_sent_rejected += num_help_requests_sent_rejected_slave;
	}

	// Report clique counts
	long long num_cliques_total = 0;

	for ( int i = 0; i < cliques_counts.size(); i++ ) {
		if ( verbose ) {
			printf( "Number of %d-cliques: %lld\n", i, cliques_counts[ i ] );
		}

		num_cliques_total += cliques_counts[ i ];
	}

	float time_past = pm.stop();
	
	// Write results to file
	if ( results_output_fname != "" && results_output_fname != "0" ) {
		// Log process
		char output_line_buffer[2048];

		sprintf( output_line_buffer, "%d\t%d\t%.5f\t%lld\t%lld\t%lld\t%lld", num_processes, help_request_threshold, time_past, num_cliques_total, num_help_requests_sent, num_help_requests_sent_accepted, num_help_requests_sent_rejected );
		std::string output_line = output_line_buffer;

		std::ofstream fh_log_output( results_output_fname, std::fstream::app|std::fstream::out );
		fh_log_output << output_line << "\n";
	}

	// Write final results
	printf( "Total number of cliques: %lld\n", num_cliques_total );

	if ( verbose ) {
		printf( "Requests: %lld/%lld accepted (%lld rejected)\n", num_help_requests_sent_accepted, num_help_requests_sent, num_help_requests_sent_rejected );
	}

	printf( "%.5f seconds past\n", time_past );
	printf( "\n" );
}

bool ProcessMaster::isMaster() {
	return true;
}
