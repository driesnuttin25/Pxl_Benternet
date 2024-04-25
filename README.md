# Pxl_Benternet

## Overview
This service facilitates spelling corrections through message passing using ZeroMQ (ZMQ). The service architecture employs a client-server model where the server (ZMQsub) subscribes to spelling requests and sends corrected responses, while the client (ZMQpush) sends requests for corrections.

## Contents of Repository
- dictionary.txt: A text file containing valid words used for spell-checking.
- ZMQsub/: Folder containing the server component.
- ZMQpush/: Folder containing the client component.
- lib/: Folder containing ZeroMQ static libraries.
- include/: Folder containing ZeroMQ header files.

## Requirements
- C++ compiler (e.g., g++, clang)
- The entire Benthernet repository from GitHub, which includes all necessary ZMQ components.

## Installation
1) Clone the Benthernet repository:
```
git clone https://github.com/driesnuttin25/Pxl_Benternet.git
```
2) The repository includes all necessary ZMQ components, so no additional installation is required.

## Architecture
![ZMQ_diagram_1](https://github.com/driesnuttin25/Pxl_Benternet/assets/114076101/b4a2c417-9b47-45c5-8aef-ff6ad31a2d32)

### Server (ZMQsub)
The ZMQsub server listens for incoming spelling requests over a ZMQ SUB socket. It uses the dictionary.txt file to find the closest correct spelling for each word in a sentence. The server then sends back the corrected sentence over a ZMQ PUSH socket.

Here is a basic outline of how the server operates:


- Connects to the broker at tcp://benternet.pxl-ea-ict.be:24042 to subscribe to requests.
- Listens for messages on the dries>spelling>.
- Processes each word in the sentence, compares it to the dictionary.txt file, and constructs a corrected sentence.
- Sends the corrected sentence back to the broker on topic dries>correct> using the PUSH socket at tcp://benternet.pxl-ea-ict.be:24041.

### Client (ZMQpush)
The ZMQpush client allows users to send sentences to the ZMQsub server for spelling correction. It connects to a ZMQ PUSH socket and sends messages to the server through the broker.

Here's what the client does:


- Connects to the broker at tcp://benternet.pxl-ea-ict.be:24041 to push requests.
- The user inputs a word or sentence for correction.
- Sends the input to the server with the topic dries>spelling>.
- Receives the corrected sentence from the server over a SUB socket connected to tcp://benternet.pxl-ea-ict.be:24042 and displays it to the user.
  
## Usage Without ZMQpush
If users choose not to utilize the ZMQpush client, they can interact with the service as follows:

1) Establish a ZMQ PUSH socket in their own client implementation.
2) Connect to tcp://benternet.pxl-ea-ict.be:24041 and send messages with the topic dries>spelling> followed by the word or sentence to be corrected.
3) Create a ZMQ SUB socket, subscribe to the dries>correct> topic, and listen for the server's response with the corrected content.
   
## Example Requests and Responses
Request: 

``` dries>spelling>Could you correct this sentence?> ```

Response:

```dries>correct>Could you correct this sentence?>```

## Troubleshooting
If you encounter any issues, ensure that:

- You are connected to the correct broker address and port.
- The topics are correctly specified for both requests and responses.
- dictionary.txt is in the correct location and readable by the server.
## Contribution
Contributions to the project are welcome. Please follow the standard GitHub pull request process to submit your contributions.
