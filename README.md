# TIVIDY: Workflow Management System

TIVIDY is a generic, robust workflow management system that controls process flows. By separating workflow definitions from runtime instances, it is highly adaptable for any industry. It demonstrates two distinct scenarios using the exact same engine. The  Campus Portal Recovery(Education) and  ProjectApproval(Technology)

## Team

| Name | Student Number |
| :--- | :--- |
| Chavonne Makurira | u25020880 |
| Felicity Moyo | u24702791 |
| Nico Sibiya | u24667642 |
| Nicole Bare | u25443705 |
| Heinrich Klopper | u25030932 |
| Tafadzwa Manyengawana | u25370325 |
| Deon-Moller Steenkamp | u25135742 |

## Dependencies
C++ Compiler: g++
Build the system: make
Testing Framework: GoogleTest v1.12.1
Documentation: Doxygen
Containerisation:Doocker

* Docker

## Build
**Local Build**
make clean
make all

**Docker build**
docker build -t tividy .

## Run
Run the demo locally:
make live

**Run the demo with docker:**
docker run --rm tividy ./main

**Interactive Shell (for debugging/inspection):**
If you want to explore the container, run GDB, or run Valgrind:

docker run -it --rm tividy bash


## Test
**Locally:**
make tests

**Execute the test suite from inside the running container:**
docker run --rm tividy ./run_tests

##Documentation:
doxygen Doxyfile
