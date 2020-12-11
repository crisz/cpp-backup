#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-narrowing-conversions"
#include "encode_length_utils.h"
#include <iostream>
#include <iomanip>

// codifica un numero in 4 byte
char* encode_length(long size) {
    std::cout << "...encoding " << std::hex << size << std::endl;
    char* result = new char[4];
    long length = htonl(size); // htonl serve per non avere problemi di endianess
    result[3] = (length & 0xFF); // vengono presi gli ultimi 8 bit del numero e inseriti in un char
    result[2] = (length >> 8) & 0xFF; // viene shiftato il numero di 8 bit, dopo di che si ripete l'operazione sopra
    result[1] = (length >> 16) & 0xFF;
    result[0] = (length >> 24) & 0xFF;

    std::cout << "encode: " << std::hex << (unsigned  int)result[0] << std::endl;
    std::cout << "encode: " << std::hex << (unsigned  int)result[1] << std::endl;
    std::cout << "encode: " << std::hex <<  (unsigned  int)result[2] << std::endl;
    std::cout << "encode: " << std::hex << (unsigned  int)result[3] << std::endl;

    return result;
}

// decodifica 4 byte in un numero
long decode_length(const char* message_size_arr) {
    long message_size = 0;
    int shift_value = 24;

    std::cout << "decode: " << std::hex << ((unsigned  int)message_size_arr[0] & 0xFF) << std::endl;
    std::cout << "decode: " << std::hex << ((unsigned  int)message_size_arr[1] & 0xFF) << std::endl;
    std::cout << "decode: " << std::hex << (unsigned  int)message_size_arr[2] << std::endl;
    std::cout << "decode: " << std::hex << (unsigned  int)message_size_arr[3] << std::endl;
    message_size += (message_size_arr[0] & 0xFF); // vengono presi gli ultimi 8 bit del numero e inseriti in un char
    message_size += (message_size_arr[1] & 0xFF) << 8; // viene shiftato il numero di 8 bit, dopo di che si ripete l'operazione sopra
    message_size += (message_size_arr[2] & 0xFF) << 16;
    message_size += (message_size_arr[3] & 0xFF) << 24;
    return message_size;

}

#pragma clang diagnostic pop