// include all the header files for commands used
#define  _GNU_SOURCE
#include <dirent.h>
// header file for input/output system calls
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// header file for file/directory statistics
#include <sys/stat.h>
// header file for wait system calls for clearing child processes
#include <sys/wait.h> 
#include <time.h>
#include <unistd.h>
// header file for signal-related system calls & modifcations
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


// Error function used for reporting issues
void error(const char *msg) {

  perror(msg);
  exit(1);

} 


// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;

}


/*
- send a message from a buffer to a socket
*/
void sendMessage(int socket, char *message) {

  // number of chars sent
  int sent = 0;
  // total of chars in the buffer to be sent
  int total = strlen(message);
  // number of chars sent by current message packet
  int charsWritten;
  while(1) {
    // if remaining characters < 200
    if ((total - sent) < 200) {
      // send all of them & terminate the function
      charsWritten = send(socket, message + sent, total - sent, 0); 
      break;
    }
    // if remaining characters > 200
    else {
      // send the next 200 characters in the buffer
      charsWritten = send(socket, message + sent, 200, 0);
      sent = sent + 200;
    }
    // if there's a problem with the send request
    if (charsWritten < 0){
      // print error to stderr
      fprintf(stderr,"DEC_SERVER: ERROR writing to socket\n"); 
    }
    // if number of chars sent is less than specified, display warning message
    if (charsWritten < 1){
      printf("DEC_SERVER: WARNING: Not all data written to socket!\n");
    }
  }

}


/*
- receive characters from a socket & store it into a buffer
*/
int receiveMessage(int socket, char *buffer) {

  // number of characters read by current receive request
  int charsRead;
  // indicator for whether client has connected to the wrong client
  int wrong_client = 0;
  // buffer to store the characters received by the current receive request
  char tempBuffer[201];
  // continue creating receive requests until then termination character "@" is detected in the buffer
  while (strstr(tempBuffer, "@") == NULL) {
    // receive a maximum number of 200 characters
    memset(tempBuffer, '\0', 201);
    charsRead = recv(socket, tempBuffer, sizeof(tempBuffer) - 1, 0); 
    // if there is an error, report to stderr & close connection socket
    if (charsRead < 0){
      fprintf(stderr,"DEC_SERVER: ERROR reading from socket\n"); 
      close(socket);
    }
    // if "d" is detected in current message received, set wrong client indicator
    if (strstr(tempBuffer, "e") != NULL) {
      wrong_client = 1;
    }
    // concactenate current message to the target buffer
    strcat(buffer, tempBuffer);
  
  }
  if (wrong_client == 1) {
    // return 0 if client connected to the wrong server
    return 0;
  }
  else {
    // return 1 if client connected to the correct server
    return 1;
  }

}


/*
- put the plaintext received from the client into a buffer
*/
void put_cyphertext(char *buffer, char *cyphertext) {

  char *end_of_cyphertext;
  // retrieve pointer to the termination indicator succeeding the plaintext
  end_of_cyphertext = strstr(buffer, "#");
  // calculate length of plaintext by subtracting pointer to plaintext's terminator indicator by its starting pointer
  int length = end_of_cyphertext - buffer - 1;
  // copy plaintext to the target buffer
  strncpy(cyphertext, buffer + 1, length);

}


/*
- put the key received from the client into a buffer
*/
void put_key(char *buffer, char *key) {

  char *end_of_key;
  char *end_of_cyphertext;
  // retrieve the pointer to the termination indicator of the key in the message received from the client
  end_of_key = strstr(buffer, "@");
  // retrieve the pointer to the termination indicator of the plaintext in the message received from the client
  end_of_cyphertext = strstr(buffer, "#");
  // calcualte lnegth of key by subtracting pointer to key's termination indicator by pointer to plaintext's termination indicator
  int length = end_of_key - end_of_cyphertext - 1; 
  // copy key to the target buffer
  strncpy(key, end_of_cyphertext + 1, length);

}


/*
- convert the ASCII value of a character to its respective integer used in the encrpytion process
*/
int asciiToNum(char *character) {

  int num;
  // if the ASCII value of the character is not the special character " "
  if (*character > 64 && *character < 91) {
    // subtract its ASCII value by 65
    num = *character - 65;
  }
  // or else
  else {
    // set it to 26
    num = 26;
  }
  // return integer
  return num;

}


/*
- convert number representing a character in the encryption process to its ASCII value
*/
char *numToAscii(int num) {

  char *character = malloc(1);
  // if number != 26, ie not the special character " "
  if (num < 26) {
    // increment the number by 65
    *character = num + 65;
  }
  else {
    // set the number to 32, ie the ASCII value for " "
    *character = 32;
  }
  // return the derived ASCII value as a character
  return character;

}


/*
- encrypt the given plaintext using the given key
- return the derived ciphertext
*/
char *decryptCyphertext(char *cyphertext, char *key) {
  
  // initialize buffer for the ciphertext
  char *plain_text = malloc(strlen(cyphertext) + 1);
  memset(plain_text, '\0', strlen(cyphertext) + 1);
  // iterate through the plaintext
  for (int x = 0; x < strlen(cyphertext); x++) {
    // retrieve the representative encryption numbers for the current character in the plaintext & key
    int cyphertext_num = asciiToNum(cyphertext + x);
    int key_num = asciiToNum(key + x);
    // derive the resulted encrypted value
    int outcome_num = cyphertext_num - key_num;
    // if value > 26
    if (outcome_num < 0) {
      // subtract it by 27, the number of available characters
      outcome_num += 27;
    }
    // put completed character into the buffer for the plaintext
    plain_text[x] = *numToAscii(outcome_num);
  }
  // return the ciphrtext
  return plain_text;

}


/*
- initialize a listening socket using localhost as the host that receives requests from all clients
- initializes a pool of 5 children processes that continuously:
 - initializes a new connection socket from the listening socket & processes any available encryption requests from clients
 - sends back the resulting ciphertext to the client
 - closes the connection socket
*/
int main(int argc, char *argv[]){

  // number of children processes created so far
  int num_processes = 0;
  // returned integer for the created connection socket
  int connectionSocket;
  // address objects for the server & the client
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
           sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);


  // continuously fork off a child process
  while(1){
    
    // until 5 child processes have been created
    if (num_processes == 5) {
      while(1) {}
    }
    // increment num_processes each time a child process is about to be forked
    num_processes++;
    // fork off a child process
    pid_t child_ID = fork();
    switch (child_ID){
      // if fork failed, display error message & move on to next prompt
      case -1: {
        error("fork() failed!");
      }
      // if child_ID == 0, the child process will run the following code
      case 0: {
        // continuously accept a new client connection & process its encryption request
        while(1) {
          // create a connection socket for the next available client connection 
          connectionSocket = accept(listenSocket, 
                                    (struct sockaddr *)&clientAddress, 
                                    &sizeOfClientInfo); 
          if (connectionSocket < 0){
            // display error if connection socket fails
            perror("ERROR on accept");
          }
          // initialize buffer for client message
          char buffer[999999];
          memset(buffer, '\0', 999999);

          // receive message from client
          int wrong_client = receiveMessage(connectionSocket, buffer);

          // initialize buffers for the plaintext & key
          char cyphertext[499999];
          char key[499999];
          memset(cyphertext, '\0', 499999);
          memset(key, '\0', 499999);
          // put the plaintext in the client's message into a buffer
          put_cyphertext(buffer, cyphertext);
          // put the plaintext in the client's message into a buffer
          put_key(buffer, key);

          // encrypt the given plaintext with the provided key
          char *plain_text = decryptCyphertext(cyphertext, key);

          // if received message contains wrong client indicator
          if (wrong_client == 0) {
            // send wrong server indicator to client
            sendMessage(connectionSocket, "d@");
          }
          else {
            // else, send ciphertext to client
            sendMessage(connectionSocket, plain_text);
            sendMessage(connectionSocket, "@");
          }

          free(plain_text);
          // close connection socket with current client request
          close(connectionSocket);
        }                    
      }
      // if current process is the shell's process
      default: {
      }    
    }
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
  
}
