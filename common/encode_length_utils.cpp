#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-narrowing-conversions"
#include "encode_length_utils.h"

// codifica un numero in 4 byte
char* encode_length(long size) {
    char* result = new char[4];
    long length = htonl(size); // htonl serve per non avere problemi di endianess
    result[3] = (length & 0xFF); // vengono presi gli ultimi 8 bit del numero e inseriti in un char
    result[2] = (length >> 8) & 0xFF; // viene shiftato il numero di 8 bit, dopo di che si ripete l'operazione sopra
    result[1] = (length >> 16) & 0xFF;
    result[0] = (length >> 24) & 0xFF;
    
    return result;
}

// decodifica 4 byte in un numero
long decode_length(char* message_size_arr) {
    long message_size = 0;
    int shift_value = 24;

    for (int i=0; i<4; i++) {
        char x = (char)message_size_arr[i];
        //
        message_size += ((char)message_size_arr[i]) << shift_value;
        shift_value -= 8;
    }

    return ntohl(message_size);
}

#pragma clang diagnostic pop