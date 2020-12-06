// TODO @cris commentalo per bene
#include "encode_length_utils.h"

char* encode_length(long size) {
    char* result = new char[4];
    long length = htonl(size); // htonl serve per non avere problemi di endianess
    result[3] = (length & 0xFF);
    result[2] = (length >> 8) & 0xFF;
    result[1] = (length >> 16) & 0xFF;
    result[0] = (length >> 24) & 0xFF;
    
    return result;
}

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
