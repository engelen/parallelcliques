// Includes
// ==========
// STL includes

// Program includes
#include "ProcessSlave.h"
#include "const.h"
#include "../Graph/Graph.h"
#include "../CliqueFinder/CliqueFinder.h"

void ProcessSlave::run() {
	// Initialize MPI settings
	initMPI();
	
	// Receive dataset input file path from master process
	receiveInputFilePath();

	// Create graph
	createGraph();

	// Create clique finder
	createCliqueFinder( graph );

	// Read graph
	readGraphFromInputFile();

	// Find cliques
	findCliques();
}

void ProcessSlave::handleInputFilePath( std::string dataset_name, std::string file_path ) {
	receiveInputFilePath();
}

void ProcessSlave::receiveInputFilePath() {
	MPI_Status status;
	int fname_length;
	std::string input_fname;

	// Buffer file name length
	MPI_Probe( 0, 0, MPI_COMM_WORLD, &status );
	MPI_Get_count( &status, MPI_CHAR, &fname_length );
	char buff[fname_length];
	
	// Receive file name from master process
	MPI_Recv( &buff, fname_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	input_fname = buff;

	// Store file name
	graph_input_fname = input_fname;
}

void ProcessSlave::createGraph() {
	graph = new Graph();
}

void ProcessSlave::readGraphFromInputFile() {
	if ( verbose ) pm.reset();
	graph->readEdgeList( graph_input_fname );

	if ( verbose ) {
		// Report information on graph
		printf( "Read graph: %d nodes, %d edges, %d degree threshold, estimated power-law exponent %.2f\n", graph->getNumNodes(), graph->getNumEdges(), cliquefinder->getHelpRequestThreshold(), graph->getPowerLawExponent() );
		printf( "Reading graph took %.2f seconds.\n", pm.lap() );
	}
}

void ProcessSlave::createCliqueFinder( Graph *g ) {
	cliquefinder = new CliqueFinder( this, g );
	cliquefinder->initParallelComputation( process_id, num_processes - 1, help_enabled );
	cliquefinder->setHelpRequestThreshold( help_request_threshold );
}

void ProcessSlave::findCliques() {
	// Perform clique finding on subtask assigned to this processor
	if ( verbose ) pm.reset();
	cliquefinder->findAllCliquesDFS();

	// Relay to master process that this slave process has completed its subtask
	transmitSubtaskCompleted();

	// Wait for a help request from another process
	standbyForHelpRequest();

	// At this point, all processes have completed their subtasks, and all cliques have been found
	if ( verbose ) {
		// Report information on time until completion
		printf( "Process %d: finding cliques took %.2f seconds (%d processes).\n", process_id, pm.lap(), num_processes );
	}

	// Report found cliques to master process
	transmitFindCliquesResult();
}

bool ProcessSlave::requestHelp() {
	// Request help from other process through master process
	int buff = 0;
	MPI_Send( &buff, 1, MPI_INT, 0, MPI_TAG_GRAPH_REQUEST_HELP, MPI_COMM_WORLD );

	// Retrieve response to help request
	int response_buff;
	MPI_Recv( &response_buff, 1, MPI_INT, 0, MPI_TAG_GRAPH_REQUEST_HELP_RESPONSE, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

	// If a positive number was returned, help is available
	if ( response_buff ) {
		helper_process_id = response_buff;
	}
	else {
		helper_process_id = -1;
	}

	return response_buff;
}

void ProcessSlave::grantHelp( int depth, std::vector<int> base_nodes, std::vector<int> potential_nodes, int helper_start, int helper_end ) {
	// Send DFS arguments to helper node
	MPI_Send( &depth, 1, MPI_INT, helper_process_id, MPI_TAG_GRAPH_HELP_DATA_DEPTH, MPI_COMM_WORLD );
	MPI_Send( &helper_start, 1, MPI_INT, helper_process_id, MPI_TAG_GRAPH_HELP_DATA_START, MPI_COMM_WORLD );
	MPI_Send( &helper_end, 1, MPI_INT, helper_process_id, MPI_TAG_GRAPH_HELP_DATA_END, MPI_COMM_WORLD );

	MPI_Send( &base_nodes[0], base_nodes.size(), MPI_INT, helper_process_id, MPI_TAG_GRAPH_HELP_DATA_BASE_NODES, MPI_COMM_WORLD );
	MPI_Send( &potential_nodes[0], potential_nodes.size(), MPI_INT, helper_process_id, MPI_TAG_GRAPH_HELP_DATA_POTENTIAL_NODES, MPI_COMM_WORLD );
}

void ProcessSlave::transmitSubtaskCompleted() {
	// Send notification to master process that node processing is done
	int buff = 0;
	MPI_Send( &buff, 1, MPI_INT, 0, MPI_TAG_GRAPH_PROCESSING_DONE, MPI_COMM_WORLD );
}

void ProcessSlave::standbyForHelpRequest() {
	while ( help_enabled ) {
		int probe_flag;
		MPI_Status probe_status;

		// Probe for any processes requesting this process's help
		MPI_Iprobe( MPI_ANY_SOURCE, MPI_TAG_GRAPH_HELP_DATA_DEPTH, MPI_COMM_WORLD, &probe_flag, &probe_status );

		// If the probe is successfull (i.e. a process has requested help), provide help. Otherwise, re-probe
		if ( probe_flag ) {
			// The process ID of the process requesting help
			int request_process_id = probe_status.MPI_SOURCE;
			//printf( "Process %d receiving from process %d.\n", process_id, request_process_id );

			int help_depth;
			std::vector<int> help_base_nodes;
			std::vector<int> help_potential_nodes;
			int help_start;
			int help_end;

			int num_help_base_nodes;
			int num_help_potential_nodes;

			// Retrieve: Simple values
			MPI_Recv( &help_depth, 1, MPI_INT, request_process_id, MPI_TAG_GRAPH_HELP_DATA_DEPTH, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			MPI_Recv( &help_start, 1, MPI_INT, request_process_id, MPI_TAG_GRAPH_HELP_DATA_START, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			MPI_Recv( &help_end, 1, MPI_INT, request_process_id, MPI_TAG_GRAPH_HELP_DATA_END, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

			// Retrieve: Base nodes
			// Buffer number of elements in base nodes list
			MPI_Probe( request_process_id, MPI_TAG_GRAPH_HELP_DATA_BASE_NODES, MPI_COMM_WORLD, &probe_status );
			MPI_Get_count( &probe_status, MPI_INT, &num_help_base_nodes );

			// Reserve space for cliques
			help_base_nodes.resize( num_help_base_nodes );

			// Receive number of cliques per clique size
			MPI_Recv( &help_base_nodes[0], num_help_base_nodes, MPI_INT, request_process_id, MPI_TAG_GRAPH_HELP_DATA_BASE_NODES, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

			// Retrieve: Potential nodes
			// Buffer number of elements in potential nodes list
			MPI_Probe( request_process_id, MPI_TAG_GRAPH_HELP_DATA_POTENTIAL_NODES, MPI_COMM_WORLD, &probe_status );
			MPI_Get_count( &probe_status, MPI_INT, &num_help_potential_nodes );

			// Reserve space for cliques
			help_potential_nodes.resize( num_help_potential_nodes );

			// Receive number of cliques per clique size
			MPI_Recv( &help_potential_nodes[0], num_help_potential_nodes, MPI_INT, request_process_id, MPI_TAG_GRAPH_HELP_DATA_POTENTIAL_NODES, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

			// Perform operations requested by other process
			cliquefinder->temporarilyDisableHelp();
			cliquefinder->findCliquesDFS( help_depth, help_base_nodes, help_potential_nodes, help_start, help_end );
			cliquefinder->setHelpEnabled( true );
			
			// Send notification to master process that node processing is done
			int buff = 0;
			MPI_Send( &buff, 1, MPI_INT, 0, MPI_TAG_GRAPH_PROCESSING_DONE, MPI_COMM_WORLD );
		}

		// Probe for master process signalling that all processing is completed
		MPI_Iprobe( 0, MPI_TAG_ALL_PROCESSING_COMPLETED, MPI_COMM_WORLD, &probe_flag, &probe_status );

		if ( probe_flag ) {
			break;
		}
	}
}

void ProcessSlave::transmitFindCliquesResult() {
	// List of clique counts per clique size
	std::vector<long long> cliques_counts_slave = cliquefinder->getCliquesCountsVector();

	// Number of help requests
	long long num_help_requests_sent, num_help_requests_sent_accepted, num_help_requests_sent_rejected;
	num_help_requests_sent = cliquefinder->getNumHelpRequestsSent();
	num_help_requests_sent_accepted = cliquefinder->getNumHelpRequestsSentAccepted();
	num_help_requests_sent_rejected = cliquefinder->getNumHelpRequestsSentRejected();

	// Send clique counts to master process
	MPI_Send( &cliques_counts_slave[0], cliques_counts_slave.size(), MPI_LONG_LONG, 0, MPI_TAG_CLIQUE_COUNTS, MPI_COMM_WORLD );
	MPI_Send( &num_help_requests_sent, 1, MPI_LONG_LONG, 0, MPI_TAG_HELP_REQUESTS_SENT, MPI_COMM_WORLD );
	MPI_Send( &num_help_requests_sent_accepted, 1, MPI_LONG_LONG, 0, MPI_TAG_HELP_REQUESTS_SENT_ACCEPTED, MPI_COMM_WORLD );
	MPI_Send( &num_help_requests_sent_rejected, 1, MPI_LONG_LONG, 0, MPI_TAG_HELP_REQUESTS_SENT_REJECTED, MPI_COMM_WORLD );

	if ( verbose ) {
		printf( "Process %d: %lld/%lld requests rejecected (%lld accepted)\n", process_id, num_help_requests_sent_rejected, num_help_requests_sent, num_help_requests_sent_accepted );
	}
}
