#ifndef STORAGE_H
#define STORAGE_H

#include <time.h>

#define MESSAGE_TYPE_LIST 'l'
#define MESSAGE_TYPE_NUM_NOTES 'n'
#define MESSAGE_TYPE_RETRIEVE 'r'
#define MESSAGE_TYPE_ENC_MSG 'e'
#define MESSAGE_TYPE_ADD 'a'
#define MESSAGE_LEN_TYPE 8

typedef struct enc_note {
    char iv[16];
    char key[16];
    uint32_t len;
    char enc_msg[1028];
} enc_note;

typedef struct saved_note {
    time_t timestamp;
    char iv[16];
    char key[16];
    uint32_t len;
    char dec_msg[1028];
} saved_note;

void send_message(int sockfd, char type, char *message, int length);
void receive_message(int sockfd, char *type, char **message);
void get_line(char *buffer, size_t size);

#endif
