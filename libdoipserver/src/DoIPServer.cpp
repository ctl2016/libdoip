#include "DoIPServer.h"

/*
 * Set up a socket till the point where the server waits till a client
 * approaches to make a connection
 */
void DoIPServer::setupSocket() {
    
    sockfd_receiver = socket(AF_INET, SOCK_STREAM, 0);
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAdress.sin_port = htons(_ServerPort);
    
    //binds the socket to the address and port number
    bind(sockfd_receiver, (struct sockaddr *)&serverAdress, sizeof(serverAdress));     
    //waits till client approach to make connection
    listen(sockfd_receiver, 5);                                                          
    
    sockfd_sender = accept(sockfd_receiver, (struct sockaddr*) NULL, NULL);
}

/*
 * Closes the socket for this server
 */
void DoIPServer::closeSocket() {
    close(sockfd_receiver);
    close(sockfd_sender);
}

/*
 * Receives a message from the client and determine how to process the message
 */
void DoIPServer::receiveMessage() {
    
    int readedBytes;
    while(true) {
        readedBytes = recv(sockfd_sender, data, _MaxDataSize, 0);

        if(readedBytes > 0) {

            GenericHeaderAction action = parseGenericHeader(data, readedBytes);
           
            switch(action.type) {
                case PayloadType::NEGATIVEACK: {
                    //send NACK
                    unsigned char* message = createGenericHeader(action.type, _NACKLength);
                    message[8] = action.value;
                    sendMessage(message, _GenericHeaderLength + _NACKLength);
                    
                    if(action.value == 0x00 || action.value == 0x04) {
                        closeSocket();
                    } else {
                        //discard message when value 0x01, 0x02, 0x03
                    }
                    break;
                }
                
                case PayloadType::ROUTINGACTIVATIONREQUEST: {
                    //start routing activation handler with the received message
                    unsigned char result = parseRoutingActivation(data);
                    unsigned char clientAddress [2] = {data[8], data[9]};
                    //clientAddress[0] = data[8];
                    //clientAddress[1] = data[9];
                    
                    unsigned char* message = createRoutingActivationResponse(clientAddress, result);
                    sendMessage(message, _GenericHeaderLength + _ActivationResponseLength);
                    
                    if(result == 0x00 || result == 0x06) {
                        closeSocket();
                    }
                    
                    break;
                }
            }    
        } 
    } 
}

/**
 * Sends a message back to the connected client
 * @param message   contains generic header and payload specific content
 * @param messageLength 
 */
void DoIPServer::sendMessage(unsigned char* message, int messageLength) {
    write(sockfd_sender, message, messageLength);
}


