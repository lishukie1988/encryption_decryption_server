// include all the header files for commands used
#define  _GNU_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// header file for file/directory statistics
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>


int main(int argc, char *argv[]){

    // Check usage & args
    if (argc != 2) { 
        fprintf(stderr,"USAGE: %s keylength\n", argv[0]);
        exit(1);
    }

    else {
        for (int i = 0; i < strlen(argv[1]); i++) {
            if (isdigit(argv[1][i]) == 0) {
                fprintf(stderr, "%s is not an integer\n", argv[1]);
                exit(1);
            };
        };
    };

    int key_length = atoi(argv[1]);
    srand((unsigned) time(0));
    char output_key[key_length+1];
    memset(output_key, '\0', key_length+1);
    for (int i = 0; i < key_length; i++) {
        int random_number = rand() % 27;
        if (random_number == 26) {
            output_key[i] = 32;
        }
        else {
            output_key[i] = random_number + 65;
        };
    };
    printf("%s\n", output_key);




    
  

    return 0;
};
