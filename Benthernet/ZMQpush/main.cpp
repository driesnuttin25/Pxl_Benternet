#include <iostream>
#include <zmq.hpp>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif
using namespace std;
int main() {
    try {
        zmq::context_t context(1);

        zmq::socket_t pusher(context, ZMQ_PUSH);
        pusher.connect("tcp://benternet.pxl-ea-ict.be:24041");

        zmq::socket_t subscriber(context, ZMQ_SUB);
        subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");

        string responseTopic = "dries>correct>";
        subscriber.setsockopt(ZMQ_SUBSCRIBE, responseTopic.c_str(), responseTopic.length());
        cout << "[Write 'exit' to exit the code]" << endl;

        while(1){
            string variableWord;
            cout << endl << "Enter a word or a sentence: ";
            getline(cin, variableWord);
            if(variableWord == "exit"){
                cout << endl << "Thank you for using the spellings checker :)";
                return 0;
            }

            string message = "dries>spelling>" + variableWord + ">";
            pusher.send(message.c_str(), message.size());
            cout << "Sent: [" << message << "]" << endl;


            zmq::message_t response;
            subscriber.recv(&response);
            string receivedResponse(static_cast<char*>(response.data()), response.size());
            cout << "Response Received: [" << receivedResponse << "]" << endl;
        }

    }
    catch(zmq::error_t& e) {
        cerr << "Caught an exception: " << e.what() << endl;
    }

    return 0;
}
