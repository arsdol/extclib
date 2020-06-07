#include <stdio.h>
#include <string.h>

#include "extclib/crypto.h"

#define KEY_SIZE 16 // 128 bit for AES key
#define MAX_SIZE 32 // multiple 128 bit for AES block

void print_bytes(uint8_t * array, size_t length);

int main(void) {
    uint8_t keystr[KEY_SIZE+1] = "AES_key_128_bits";
    uint8_t buffer[MAX_SIZE+1] = "1234567890abcdefghij";

    uint8_t *ivector = "1234567890123456";

    Crypto params = {
        .mode = CBC_MODE,
        .data = {
            .size = strlen(buffer),
            .to = buffer,
            .from = buffer,
        },
        .key = {
            .iv = ivector,
            .size = (KEY_SIZE*8),
            .bytes = keystr,
        },
    };

    print_bytes(buffer, MAX_SIZE);

    params.option = ENCRYPT_OPTION;
    aes_crypto(params);
    print_bytes(buffer, MAX_SIZE);

    params.option = DECRYPT_OPTION;
    aes_crypto(params);
    print_bytes(buffer, MAX_SIZE);
    return 0;
}

void print_bytes(uint8_t * array, size_t length) {
    printf("[ ");
    for (size_t i = 0; i < length; ++i)
        printf("%x ", array[i]);
    printf("]\n");
}
