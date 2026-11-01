# TIVIDY: Workflow Management System

TIVIDY is a generic, robust workflow management system that controls process flows. By separating workflow definitions from runtime instances, it is highly adaptable for any industry.

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

* Docker

## Build

```
docker build -t [image_name] .
```

## Run

Start the container interactively:

```
docker run -it [image_name]
```

## Test

Execute the test suite from inside the running container:

```
./run_tests
```
