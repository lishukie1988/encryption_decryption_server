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
// gethostbyname()
#include <netdb.h>     


// Set up the address struct used to store the server's address where the client will connect to
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "DEC_CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);

}


/*
- validate that there are no bad characters in a file
*/
int validateFile(char *file) { 

  char character;
  FILE* file_pointer = fopen(file, "r"); 

    // iterate through every character in the file
    for (character = fgetc(file_pointer); character != 10; character = fgetc(file_pointer)) {
        if (!((character > 64 && character < 91) || character == 32)) {
            // return 0 if a bad character is detected
            return 0;
        }
    }
    // Close the file 
    fclose(file_pointer); 
    // return 1 if the file doesn't contain bad characters
    return 1; 

}


/*
- return the word count of a file
*/
int charCount(char *file) {
    FILE* file_pointer; 
  
    int count = 0; 
    char character; 

    // Open the file 
    file_pointer = fopen(file, "r"); 
    // iterate through each character of the file
    for (character = fgetc(file_pointer); character != 10; character = fgetc(file_pointer)) {
  
        // Increment count for this character 
        count = count + 1; 
    }
  
    // Close the file 
    fclose(file_pointer); 
    // return the word count
    return count; 

}


/*
- checks to see a file is in the current directory
*/
int fileExists(char *file) {

  FILE* file_pointer; 
  // attempt to open the file
  file_pointer = fopen(file, "r"); 
  if (file_pointer == NULL) {
    // file doesn't exist
    return 0;
  }
  // file exists
  return 1;

} 


/*
- validate that:
 - both files exist
 - neither file contains bad characrters
 - key is at least as long as the plaintext
*/
int validate(char *cyphertext, char *key) {

  // check to see if both files exist
  if (fileExists(cyphertext) != 1 || fileExists(key) != 1) {
    fprintf(stderr, "dec_client error: cannot open input file(s)\n");
    // return 0 if not
    return 0;
  }
  // check to see if both files are free of bad chars
  int p_validated = validateFile(cyphertext);
  int k_validated = validateFile(key);
  if (p_validated != 1 || k_validated != 1) {
    fprintf(stderr, "dec_client error: input contains bad characters\n");
    // return 0 if not
    return 0;
  }
  // check if key is at least as long as plaintext
  if (charCount(key) < charCount(cyphertext)) {
    fprintf(stderr, "Error: key \'%s\' is too short\n", key);
    // return 0 if not
    return 0;
  }
  // return 1 if all criterias are satisfied
  return 1;

}


/*
- put the contents of a "\n" trailed file into a buffer 
*/
void putBuffer(char *buffer, char *file) {

  FILE* file_pointer; 
 
  size_t buffer_size = charCount(file) + 2;
  // open file
  file_pointer = fopen(file, "r");
  // get the one line consisting of the entire content of the file & store it in buffer
  getline(&buffer, &buffer_size, file_pointer);

  buffer[charCount(file)] = '\0';

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
      // print error to stderr & exit with value 1
      fprintf(stderr,"DEC_CLIENT: ERROR writing to socket\n"); 
      close(socket);
      exit(1);
    }
    // if number of chars sent is less than specified, display warning message
    if (charsWritten < 1){
      printf("DEC_CLIENT: WARNING: Not all data written to socket!\n");
    }
  }

}


/*
- receive characters from a socket & store it into a buffer
*/
int receiveMessage(int socket, char *buffer) {

  // number of characters read by current receive request
  int charsRead;
  // indicator for whether client has connected to the wrong server
  int wrong_server = 0;
  // buffer to store the characters received by the current receive request
  char tempBuffer[201];
  // continue creating receive requests until then termination character "@" is detected in the buffer
  while (strstr(tempBuffer, "@") == NULL) {
    // receive a maximum number of 200 characters
    memset(tempBuffer, '\0', 201);
    charsRead = recv(socket, tempBuffer, sizeof(tempBuffer) - 1, 0); 
    if (charsRead < 0){
      // if there is an error, report to stderr & exit with value 1
      perror("DEC_CLIENT");
      close(socket);
      exit(1);
    }
    // if "e" is detected in current message received, set wrong server indicator
    if (strstr(tempBuffer, "e") != NULL) {
      wrong_server = 1;
    }
    // concactenate current message to the target buffer
    strcat(buffer, tempBuffer);
  }
  // slice off the termination character "@"
  char *terminal_char = strstr(buffer, "@");
  memset(terminal_char, '\0', 1);
  if (wrong_server == 1) {
    // return 0 if client connected to the wrong server
    return 0;
  }
  else {
    // return 1 if client connected to the correct server
    return 1;
  }

}

/*
- verify that input files are valid, exit program otherwise
- establishes a connection socket with enc_server
- sends plaintext & key to enc_server
- receives ciphertext from enc_server
- prints ciphertext to stdout
*/
int main(int argc, char *argv[]) {

  // initialize variables for the socket file & specified port number
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s cyphertext key port\n", argv[0]); 
    exit(0); 
  } 
  // if either of the input files are invalid, prints appropriate error msg to stderr & exit with value 1
  if (validate(argv[1], argv[2]) == 0) {
    exit(1); 
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    fprintf(stderr,"DEC_CLIENT: ERROR opening socket\n"); 
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server with address specified with localhost as the target host & input port as the target port
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr,"Error: could not contact dec_server on port %s\n", argv[3]); 
    exit(2);
  }

  // initialize buffer for the input plaintext  
  char buffer_cyphertext[charCount(argv[1]) + 2];
  // initialize buffer for the input key
  char buffer_key[charCount(argv[2]) + 2];
  memset(buffer_cyphertext, '\0', charCount(argv[1]) + 2);
  memset(buffer_key, '\0', charCount(argv[2]) + 2);

  // put the input plaintext into its initialized buffer
  putBuffer(buffer_cyphertext, argv[1]);
  // put the input key into its initialized buffer
  putBuffer(buffer_key, argv[2]);

  // send "d" to indicate that this message is sent from the enc_client
  sendMessage(socketFD, "d");
  // send the plaintext
  sendMessage(socketFD, buffer_cyphertext);
  // send "#" to indicate the end of the plaintext
  sendMessage(socketFD, "#");
  // send the key
  sendMessage(socketFD, buffer_key);
  // send "@" to indicate the end of the key and the entire message transaction
  sendMessage(socketFD, "@");

  // initialize buffer for the ciphertext
  char buffer_plaintext[charCount(argv[1]) + 2];
  memset(buffer_plaintext, '\0', charCount(argv[1]) + 2);

  // receive message from the server
  int wrong_server = receiveMessage(socketFD, buffer_plaintext);
  // close the connection socket
  close(socketFD);
  
  // if message contains wrong server indicator "d", implying that dec_client attempted to connect to enc_server 
  if (wrong_server == 0) {
    // print error to stderr & exit with value 2
    fprintf(stderr,"Error: dec_client could not contact enc_server on port %s\n", argv[3]); 
    exit(2);
  }
  // or else, print plaintext to stdout
  else {
    printf("%s\n", buffer_plaintext);  
    // exit program with value 0
    return 0;
  }
  
}