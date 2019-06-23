# Interactive display simulation

This repository contains the implementation of a simulation where customers may select products and interact with those products on a display. These interactions are cataloged on the backend and later can be analysed.

## Overview of the architectural design

<img width="1024" alt="Architectural overview" src="https://user-images.githubusercontent.com/204792/59983345-6d4b9080-95ec-11e9-9faf-ab3374f43817.jpg">

The technologies chosen to implement the system were:

* [Protocol buffers](https://github.com/protocolbuffers/protobuf) (protobuf) to represent and serialize/deserialize the data structures for transmission over the network
* [gRPC](https://grpc.io) to implement the endpoint on the server side and the stub on the client side. Remote Procedure Calls are strongly typed and can offer several advantages over pure GET/POST requests
* [C++](https://isocpp.org) to implement the code, given the language is very fast, portable, and has seen a renaissance
* [Docker](https://www.docker.com) to encapsulate and deploy both server and client. The images have been configured to contain all the required dependencies

## Clone the repository to your local machine

This project assumes you have Docker installed and running on youur computer. It also assumes you have a working internet connection.

```
git clone https://github.com/dcirne/perch.git
```

## Building and running

```
cd perch
```

Build the server
```
docker build -f Dockerfile.backend -t backend .
```

Build the simulation (client)
```
docker build -f Dockerfile.simulation -t simulation .
```

Create a network where client and server are visible, and referrable, to each other
```
docker network create --subnet 172.20.0.0/16 --ip-range 172.20.240.0/20 perch_demo
```

Running: Open two terminals. On the first termianl run the server container
```
docker run --net=perch_demo --ip 172.20.128.2 backend
```

On the second terminal run the simulation container
```
docker run --net=perch_demo --ip 172.20.128.3 simulation
```

## Stopping

The simulation client is time bount and will exit by itself. The server container will not exit automatically and needs to be terminated manually.

List the containers
```
docker container ls
```

Stop the `backend` container
```
docker kill <CONTAINER ID>
```
