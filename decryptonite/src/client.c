#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <openssl/evp.h>

#include "storage.h"

char *menu[] = {
    "List notes",
    "Add a note"
};

int get_choice_from_menu(char *prompt, char **opts, int num_opts) {
    char input_buf[16];
    if (num_opts == 0) {
        printf("Empty.\n");
        return -1;
    }
    for (int i = 0; i < num_opts; ++i) {
        printf("%d. %s\n", i+1, opts[i]);
    }

    while (1) {
        printf("%s (1-%d): ", prompt, num_opts);
        fgets(input_buf, 16, stdin);
        
        // If input didn't contain a newline, flush the rest of the line
        if (!strchr(input_buf, '\n')) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }

        int choice = atoi(input_buf);
        if (choice > 0 && choice <= num_opts) {
            return choice;
        }
        printf("Invalid input.\n");
    }
}

void get_rand_str(int len, char *buf) {
    for (int i = 0; i < len; ++i) {
        buf[i] = rand() % 256;
    }
}

void encrypt_msg(char *msg, enc_note *enc_msg) {
    int enc_length = 0;
    int total_length = 0;
    get_rand_str(16, enc_msg->iv);
    get_rand_str(16, enc_msg->key);
    enc_msg->len = strlen(msg) + 1;

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, enc_msg->key, enc_msg->iv);

    if (!EVP_EncryptUpdate(&ctx, enc_msg->enc_msg, &enc_length, msg, enc_msg->len)) {
        perror("Encryption failed");
        exit(1);
    }
    total_length += enc_length;
    if (!EVP_EncryptFinal_ex(&ctx, enc_msg->enc_msg + enc_length, &enc_length)) {
        perror("Encryption of final block failed");
        exit(1);
    }
    total_length += enc_length;
    EVP_CIPHER_CTX_cleanup(&ctx);
}

char *decrypt_message(enc_note *msg) {
    enc_note enc;
    EVP_CIPHER_CTX ctx;
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
        return NULL;
    }
    total_size += dec_len;
    EVP_DecryptFinal_ex(&ctx, dec_msg + total_size, &dec_len);
    total_size += dec_len;

    EVP_CIPHER_CTX_cleanup(&ctx);
    return dec_msg;
}

void start_client(char *address, int port) {
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Could not connect to server");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char msg_buf[1000];
    enc_note enc_msg = {};
    char recvd_msg_type;
    char *recvd_msg;
    int num_notes;
    while(1) {
        int choice = get_choice_from_menu("Choose an option", menu, sizeof(menu) / sizeof(char*));

        switch (choice) {
            case 1: {
                // List notes
                send_message(sock, MESSAGE_TYPE_LIST, "a", 1);
                receive_message(sock, &recvd_msg_type, &recvd_msg);
                if (recvd_msg_type != MESSAGE_TYPE_NUM_NOTES) {
                    fprintf(stderr, "Error retrieving messages from server: invalid message type");
                    close(sock);
                    exit(1);
                }
                printf("------------- Notes -------------\n");
                num_notes = *(int *)recvd_msg;
                if (num_notes == 0) {
                    printf("None.\n");
                }
                for (int i = 0; i < num_notes; ++i) {
                    receive_message(sock, &recvd_msg_type, &recvd_msg);
                    char *dec_msg = decrypt_message((enc_note *)recvd_msg);
                    printf("%d. %s\n", i+1, dec_msg);
                    free(dec_msg);
                }
                printf("---------------------------------\n\n");

                break;
            }
            case 2: {
                // Add note
                printf("Enter note to save: ");
                get_line(msg_buf, sizeof(msg_buf));
                encrypt_msg(msg_buf, &enc_msg);
                send_message(sock, MESSAGE_TYPE_ADD, (char *)&enc_msg, sizeof(enc_note));
                break;
            }
        }
    }
}

void print_usage() {
    puts(
        "Usage: ./client [OPTION]...\n"
        "\n"
        "Runs the secure notes client\n"
        "\n"
        "  -h            Show this message and exit\n"
        "  -c <port>     Specify the secure notes server address\n"
        "  -p <port>     Specify the secure notes port\n"
    );
}

int main(int argc, char **argv) {
    int c;
    int port = 0;
    char *address = NULL;

    while ((c = getopt(argc, argv, "p:c:h")) != -1) {
        switch (c) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'c':
                address = optarg;
                break;
            case 'h':
                print_usage();
                return 0;
            case '?':
                if (optopt == 'p' || optopt == 'c') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option -%c.\n", optopt);
                }
                return 1;
            default:
                abort();
        }
    }

    if (address == NULL || port == 0) {
        print_usage();
        return 0;
    }

    srand(time(NULL));

    start_client(address, port);
    return 0;
}