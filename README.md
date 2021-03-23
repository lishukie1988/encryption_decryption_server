# encryption_decryption_server

- Summary:
    - This is an encryption and decryption application written in C using the one-time pad encryption and decryption algorithm.
    - The program consists of 5 files:
        - enc_server.c
        - dec_server.c
        - enc_client.c
        - dec_client.c
        - keygen.c

- Usage details:
    - Please have the C gcc compiler installed on your system.
    - Please run the Makefile file to have the c files compiled into executable files.
    - Once the executable files are created, please initial the program in the following order:
    - Start the enc_server file by entering the following command into a terminal window:
        - ./enc_server 4321 &
        - Note: This starts the encryption program at port 4321 in the background. It is possible to use any other port as the input port.
    - Start the dec_server file by entering the following command into a terminal window:
        - ./dec_server 1234 &
        - Note: this starts the decryption program at port 1234 in the background. It is possible to use any other port as the input port.
    - Create a text file containing a key by running the keygen file using the following command:
        - ./keygen 99999 > sample_key
        - Note: this creates a file named "sample_key" containing a key consisting of uppercase alphabet letters and the space character of length 99999
    - Encrypt a plaintext file using the enc_client program by entering the following command:
        - ./enc_client plaintext1 sample_key 4321 > sample_cipher
        - Note: this creates a text file named "sample_cipher" containing the ciphertext corresponding to the sample plaintext file named "plaintext1" using the encryption server initialized and key created in the previous steps. It is also possible to enter a custom plaintext file consisting of only uppercase alphabet letters and the space character.
    - Decrypt a ciphertext file using the dec_client program by entering the following command:
        - ./dec_client sample_cipher sample_key 1234 > sample_result
        - Note: this creates a text file named "sample_result" containing the resulting plaintext corresponding to the previously created sample ciphertext file named "sample_cipher" using the decryption server initialized and key created in the previous steps. The sample_result file should be identical to the original plaintext1 file if every step has been executed correctly.