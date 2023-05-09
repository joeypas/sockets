#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <iosfwd>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <zconf.h>
#include <zlib.h>
#include <iomanip>
#include <cassert>


class buffer {
public:
    static void add_buffer_to_vector(std::vector<char> &vector, const char *buffer, uLongf length) {
        for (int character_index = 0; character_index < length; character_index++) {
            char current_character = buffer[character_index];
            vector.push_back(current_character);
        }
    }

    static int compress_vector(std::vector<char> source, std::vector<char> &destination) {
        unsigned long source_length = strlen(source.data());
        uLongf destination_length = compressBound(source_length);

        char *destination_data = (char *) malloc(destination_length);
        if (destination_data == nullptr) {
            return Z_MEM_ERROR;
        }

        Bytef *source_data = (Bytef *) source.data();
        int return_value = compress2((Bytef *) destination_data, &destination_length, source_data, source_length,
                                    Z_BEST_COMPRESSION);
        add_buffer_to_vector(destination, destination_data, destination_length);
        free(destination_data);
        return return_value;
    }

    static int decompress_vector(std::vector<char> source, std::vector<char> &destination) {
        unsigned long source_length = source.size();
        uLongf destination_length = 0x1000;

        char *destination_data = (char *) malloc(destination_length);
        if (destination_data == nullptr) {
            return Z_MEM_ERROR;
        }

        Bytef *source_data = (Bytef *) source.data();
        int return_value = uncompress((Bytef *) destination_data, &destination_length, source_data, source.size());
        add_buffer_to_vector(destination, destination_data, destination_length);
        free(destination_data);
        return return_value;
    }

    static void add_string_to_vector(std::vector<char> &uncompressed_data,
                            const char *my_string) {
        int character_index = 0;
        while (true) {
            char current_character = my_string[character_index];
            uncompressed_data.push_back(current_character);

            if (current_character == '\00') {
                break;
            }

            character_index++;
        }
    }

    static void print_bytes(std::ostream &stream, const unsigned char *data, size_t data_length, bool format = true) {
        stream << std::setfill('0');
        for (size_t data_index = 0; data_index < data_length; ++data_index) {
            stream << std::hex << std::setw(2) << (int) data[data_index];
            if (format) {
                stream << (((data_index + 1) % 16 == 0) ? "\n" : " ");
            }
        }
        stream << std::endl;
    }
};

#endif