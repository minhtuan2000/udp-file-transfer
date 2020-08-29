# udp-file-transfer
A simple and robust file-sharing system, using both TCP and UDP.

The implementation have been tested in Ubuntu 20.04

## 1. Build the client and server

To build both client and server, open a terminal at the root folder of the project (where this readme.txt and Makefile is located) and run:

```bash
make clean
make
```

To build the client separately, open a terminal at the root folder of the project (where this readme.txt and Makefile is located) and run:

```bash
make clean
make client/client
```

To build the server separately, open a terminal at the root folder of the project (where this readme.txt and Makefile is located) and run:

```bash
make clean
make server/server
```
        
## 2. Run the server

From the root folder of the project (where this readme.txt and Makefile is located), run:
    
```bash
./server/server port [option]
```

```
port: The port number where the server will be listening
option:
    --log, -l   Set log level (DEBUG, INFO, ERROR, NONE). Default: INFO
```

**NOTE**: The files that the server will serve are to be located in the /files subdirectory.

## 3. Run the client

From the root folder of the project (where this readme.txt and Makefile is located), run:

```bash
./client/client request_url [option]
```

```
request_url:
    http://<server_address>:<server_port>/sendfile/<filename>/<dest_address>/<dest_port_number>
    where:
        <server_address> is the address of the server
        <server_port> is the port number of the server
        <filename> is the name of the file to send
        <dest_address> is the (IPv4) address of the client, to which the file should be sent
        <dest_port_number> is the UDP port number of the client, to which the file should be sent
option:
    --log, -l   Set log level (DEBUG, INFO, ERROR, NONE). Default: INFO
```

**NOTE**: 

The downloaded files will be assembled and located in the /download subdirectory. If this subdirectory does not exist, please create it first.
    
Only the original request URL is needed, the client will automatically send follow-up requests for the missing file chunks if needed.
    
The <dest_address> should be where the client have access to host a server to receive files (eg. localhost, 127.0.0.1).
