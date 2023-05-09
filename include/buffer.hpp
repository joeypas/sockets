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
        unsigned long source_length = source.size();
        uLongf destination_length = compressBound(source_length);

        char *destination_data = (char *) malloc(destination_length);
        if (destination_data == nullptr) {
            return Z_MEM_ERROR;
        }

        Bytef *source_data = (Bytef *) source.data();
        int return_value = compress2((Bytef *) destination_data, &destination_length, source_data, source_length,
                                    Z_BEST_SPEED);
        add_buffer_to_vector(destination, destination_data, destination_length);
        free(destination_data);
        return return_value;
    }

    static int decompress_vector(std::vector<char> source, std::vector<char> &destination) {
        unsigned long source_length = source.size();
        uLongf destination_length = compressBound(source_length);

        char *destination_data = (char *) malloc(destination_length);
        if (destination_data == nullptr) {
            return Z_MEM_ERROR;
        }

        Bytef *source_data = (Bytef *) source.data();
        int return_value = uncompress((Bytef *) destination_data, &destination_length, source_data, source.size());
        std::cout << destination_data << std::endl;
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
};

#endif