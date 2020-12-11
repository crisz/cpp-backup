#pragma once
#include <arpa/inet.h>

char* encode_length(long size);
long decode_length(const char* message_size_arr);