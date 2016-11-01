
ParallelCliques
===================
The **ParallelCliques** toolkit finds all cliques in an undirected graph using a parallelised algorithm. Its algorithm design and implementation were part of the course project for the Leiden University Computer Science Master's program 2015-2016 course [*Seminar in Distributed Data Mining*][1]. The algorithm was designed and implemented independently by the author of this repository, [@engelen][4].

-----

## Implementation & Requirements

The algorithm relies on [OpenMPI][2] for its parallel implementation and on the [Boost][3] hashing and string algorithm libraries. These have to be installed for successful compilation.

-----

## Download & Setup

The **ParallelCliques** toolkit can be installed by cloning the repository, i.e.

> `$ git clone https://github.com/engelen/parallelcliques.git`

or by downloading and extracting the zip file. The toolkit uses OpenMPI for its parallel implementation, which means the OpenMPI module must be loaded before compilation. This can be done through the following command:

> `$ module load openmpi/gcc/64`

Then, navigate to the folder where you cloned the repository or extracted the zip file, and run

> `$ make`

to compile the program.

-----

## Usage

To run the program in a parallelised fashion without any additional arguments for the toolkit, use `mpirun` as follows:

> $ mpirun [mpi arguments] main -d [dataset_id]

Here, `[dataset_id]` should be replaced by the name of your dataset, and there should be a file `[dataset_id].tsv` in the folder `data/input`. For information about the structure of these files, see the section [Structure of network dataset files][#structure-of-network-dataset-files].

### MPI arguments
To use multiprocessing, specify the number of processes MPI should use using its `-np`-argument. For example, calling
> $ mpirun -np 16 main -d mynetwork

finds the cliques in the network from `data/input/mynetwork.tsv` using 16 processes. Do note that one of these processes is used to govern the processing of the other nodes, and thus in effect 15 processes will take part in the actual computation. This also mandates that at least 2 processes have to be used to run the calculation, in which case no parallelisation will take place.

### Toolkit arguments
The toolkit features the following arguments, each of which can be set using `[arg] [value]` in the program call.

`-d`: **Dataset ID**
The toolkit looks for a network to do its computation on in the `data/input` folder for the file `[dataset_id].tsv`, if no full file path is specified (see the `-f` argument documentation).

`-f`: **File path**
Optional. Override the dataset ID and load the specified file directly from file path. Defaults to no override (empty string).

`-o`: **Output file**
Optional. Output file to write final results to. The output contains information about the assistance requests sent and the number of cliques found.

`-h`: **Dynamic load balancing enabled**
Optional. Whether to use dynamic load balancing. Defaults to 1.

`-v`: **Verbose**
Optional. Defaults to 1.

`-t`: **Help request threshold**
Optional. Help request threshold to be used in dynamic load balancing. Defaults to 0 (calculate automatically).

### Structure of network dataset files

Network dataset files should be structured as an edge list, where each line of the file corresponds to a single edge, specified by a source node and a target node, separated by a tab. Edges are treated as undirected edges; specifying both directions of an edge is not required.

An example edge file would be

> 0	1
> 1	2
> 2	0

specifying a graph with three nodes which are all connected by a direct edge.

-----
## Parallelisation and dynamic load balancing

The algorithm finds cliques using a parallel, depth-first search approach with dynamic load balancing. The algorithm is parallelised by evenly distributing all nodes based on their IDs to the processes specified using OpenMPI. To combat the imbalance of the branches of the search tree containing cliques, *dynamic load balancing* is applied. Whenever a process encounters a branch that potentially, based on the degree of the node currently evaluated, has a large amount of cliques, it requests help from another process via the master process. If another process is available, the master process delegates this part of the search tree to the available process, after which the process that requested help continues with the rest of its search tree.

[1]: https://studiegids.leidenuniv.nl/courses/show/54715/seminar-distributed-data-mining
[2]: https://www.open-mpi.org/
[3]: http://www.boost.org/
[4]: https://github.com/engelen
