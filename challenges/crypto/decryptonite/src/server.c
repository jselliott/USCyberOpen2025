#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <sys/personality.h>
#include <sys/wait.h>

#include "storage.h"

#define DEF_PORT 8888   // Default port to listen on
#define DEF_BACKLOG 100 // Default number of pending connections queue will hold
#define DEF_TIMEOUT 30  // Default timeout
#define MAX_SAVED_NOTES 10

int connection_socket;      // Holds the connection socket file descriptor
char connection_addr[1024]; // Holds the client address

saved_note notes_buf[MAX_SAVED_NOTES] = {};
int num_notes = 0;
int notes_start_idx = 0;

void handle_alarm(int signum) {
    printf("Client %s disconnected.\n", connection_addr);
    close(connection_socket);
    exit(0);
}

void add_message(char *msg) {
    EVP_CIPHER_CTX ctx;
    enc_note enc;
    EVP_CIPHER_CTX_init(&ctx);

    memcpy(&enc, msg, sizeof(enc_note));
    uint32_t enc_len = ((enc.len / 16) + 1) * 16;
    char *dec_msg = malloc(enc_len);
    uint32_t dec_len = 0;
    uint32_t total_size = 0;

    // Decrypt message
    EVP_DecryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, enc.key, enc.iv);
    if (!EVP_DecryptUpdate(&ctx, dec_msg, &dec_len, enc.enc_msg, enc_len)) {
        perror("Decryption failed");
        return;
    }
    total_size += dec_len;
    dec_len = 0;
    EVP_DecryptFinal_ex(&ctx, dec_msg + total_size, &dec_len);
    total_size += dec_len;

    memcpy(enc.enc_msg, dec_msg, total_size);
    EVP_CIPHER_CTX_cleanup(&ctx);
    free(dec_msg);

    // Add to note list
    saved_note *saved = &notes_buf[(num_notes + notes_start_idx) % MAX_SAVED_NOTES];
    saved->timestamp = time(NULL);
    memcpy(&saved->iv, &enc, sizeof(enc_note));
    
    if (num_notes == MAX_SAVED_NOTES) {
        notes_start_idx = (notes_start_idx + 1) % MAX_SAVED_NOTES;
    }
    else {
        num_notes++;
    }
}

void encrypt_msg(saved_note *in, enc_note *out) {
    memcpy(out->iv, in->iv, sizeof(out->iv));
    memcpy(out->key, in->key, sizeof(out->key));
    int enc_length = 0;

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, out->key, out->iv);

    if (!EVP_EncryptUpdate(&ctx, out->enc_msg, &enc_length, in->dec_msg, in->len)) {
        perror("Encryption failed");
        exit(1);
    }
    out->len = enc_length;
    if (!EVP_EncryptFinal_ex(&ctx, out->enc_msg + enc_length, &enc_length)) {
        perror("Encryption of final block failed");
        exit(1);
    }
    out->len += enc_length;
    EVP_CIPHER_CTX_cleanup(&ctx);
}

void send_note(int sock, saved_note *note) {
    enc_note to_send;
    encrypt_msg(note, &to_send);
    send_message(sock, MESSAGE_TYPE_ENC_MSG, (char *)&to_send, sizeof(to_send));
}

void send_note_list(int sock) {
    send_message(sock, MESSAGE_TYPE_NUM_NOTES, (char *)&num_notes, sizeof(char *));
    for (int i = 0; i < num_notes; ++i) {
        int note_idx = (notes_start_idx + i) % MAX_SAVED_NOTES;
        send_note(sock, &notes_buf[note_idx]);
    }
}

void run_server(int socket, unsigned int timeout) {
    while (1) {
        alarm(timeout);
        char msg_type;
        char *msg;

        receive_message(socket, &msg_type, &msg);

        switch (msg_type) {
            case MESSAGE_TYPE_ADD:
                add_message(msg);
                break;
            case MESSAGE_TYPE_LIST:
                send_note_list(socket);
                break;
        }
    }
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char **argv)
{
    int c;
    int port = DEF_PORT;
    int backlog = DEF_BACKLOG;
    int timeout = DEF_TIMEOUT;

    int sockfd;                     // Listen on sock_fd, new connection on connection_socket
    struct sockaddr_in server_addr; // Server address
    struct sockaddr_in client_addr; // Client address
    socklen_t sin_size;

    // Force disable ASLR
    const int old_personality = personality(ADDR_NO_RANDOMIZE);
    if (!(old_personality & ADDR_NO_RANDOMIZE)) {
        const int new_personality = personality(ADDR_NO_RANDOMIZE);
        if (new_personality & ADDR_NO_RANDOMIZE) {
            execv(argv[0], argv);
        }
    }

    // Clean up zombie processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Parse command-line arguments
    while ((c = getopt(argc, argv, "p:b:t:h")) != -1) {
        switch (c) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'b':
                backlog = atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            case 'h':
                puts(
                    "Usage: ./server [OPTION]...\n"
                    "\n"
                    "Runs the secure notes server\n"
                    "\n"
                    "  -h            Show this message and exit\n"
                    "  -p <port>     Specify the port the server should listen on (default 8888)\n"
                    "  -b <backlog>  Specify how many connections the queue should hold (default 100)\n"
                    "  -t <timeout>  Close connection after a user has been inactive for the specified number of seconds (default 30)\n"
                );
                return 0;
            case '?':
                if (optopt == 'p' || optopt == 'b' || optopt == 't') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option -%c.\n", optopt);
                }
                return 1;
            default:
                abort();
        }
    }

    // Set signal handler
    signal(SIGALRM, handle_alarm);

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
    int reuseaddr = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(sockfd, backlog) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("Listening on port %d\n", port);

    // Accept incoming connections and fork a new process to handle each one
    while (1)
    {
        sin_size = sizeof(client_addr);
        if ((connection_socket = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }

        if (!fork()) // This is the child process
        {
            // Close the listen socket, as the child only needs the connection socket
            close(sockfd);
            strncpy(connection_addr, inet_ntoa(client_addr.sin_addr), 1024);
            printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr)); // Print the client's IP address
            // Seed the random generator for this child process
            srand(time(0));
            // Run the server
            run_server(connection_socket, timeout);
            // Close the connection socket
            close(connection_socket);
            exit(0);
        }
        else // This is the parent process
        {
            // Close the connection socket, as the parent only needs the listen socket
            close(connection_socket);
        }
    }

    // Close the listen socket
    close(sockfd);
    return 0;
}
