# Power Plant Placement Solver
This project solves the Minimum Dominating Set problem to determine optimal power plant placements in a graph, ensuring every city (vertex) is covered by at least one power plant (either directly or via a neighboring city). The solution combines a greedy heuristic for fast, near-optimal results and an Integer Linear Programming (ILP) approach using the HiGHS solver for optimal solutions. The implementation is optimized for performance, with preprocessing, warm-starting, and parallelization using OpenMP.
Features

## Prerequisites

- C++ Compiler: GCC or Clang with C++17 support (e.g., g++ >= 7.0).
- HiGHS Solver: Open-source linear programming solver (see installation).
- OpenMP: For parallelization (included with most compilers, enabled with -fopenmp).
- Docker: For create a containerized environment.
- Make: For automate building and running the project.

## Installation

Clone the Repository:
```
git clone https://github.com/your-username/power-plant-solver.git
cd power-plant-solver
```
or pull from dockerhub
```
docker pull thanaphom/hpa-powerplant-solver
```

### Folder Directory:

This project requires to read/write within the `data` folder, create the folder directory as below before running the program.
```
/root
├── data/
│   ├── input/
│   ├── output/
├── Dockerfile
└── run.cpp
```

### Install HiGHS:

Follow the HiGHS installation guide.
On Ubuntu, you can build from source:git clone https://github.com/ERGO-Code/HiGHS.git
```
cd HiGHS
mkdir build
cd build
cmake ..
make
sudo make install
```
In this project, HiGHS is installed within docker.


### Install OpenMP:

Typically included with GCC/Clang. Verify with:g++ --version

If missing, install on Ubuntu:sudo apt-get install libomp-dev



## Build the Project:

Using a Makefile to build and run the project, change the Makefile local value before run
```
make run-docker
```
Or run directly
```
docker run --rm -v ./data/input:/input  -v ./data/output:/output ${DOCKER_IMAGE_NAME} /input/{INPUT_FILE} /output/{OUTPUT_FILE}
```
