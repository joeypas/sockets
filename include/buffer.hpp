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
#include <string>


class buffer {
public:
    static void add_buffer_to_vector(std::vector<char> &vector, const char *buffer, uLongf length) {
        for (int character_index = 0; character_index < length; character_index++) {
            char current_character = buffer[character_index];
            vector.push_back(current_character);
        }
    }

    static int compress_vector(std::vector<char> source, std::vector<char> &destination) {
        int ret, flush;
        unsigned int have;
        z_stream strm;
        const int CHUNK = 16384;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];
        std::vector<char> compressed_data;
        destination.clear();

        memset(&strm, 0, sizeof(strm));
        ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
        if (ret != Z_OK) {
            std::cerr << "Error initializing deflate: " << ret << std::endl;
            return ret;
        }

        strm.avail_in = source.size();
        strm.next_in = reinterpret_cast<unsigned char *>(source.data());

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, Z_FINISH);
            have = CHUNK - strm.avail_out;
            if (have > 0) {
                compressed_data.insert(compressed_data.end(), out, out + have);
            }
        } while (strm.avail_out == 0);

        ret = deflateEnd(&strm);
        if (ret != Z_OK) {
            std::cerr << "Error ending deflate: " << ret << std::endl;
            return ret;
        }

        destination.swap(compressed_data);
        return Z_OK;
    }

    static int decompress_vector(std::vector<char> source, std::vector<char> &destination) {
        int ret, flush;
        unsigned int have;
        z_stream strm;
        const int CHUNK = 16384;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];
        std::vector<char> decompressed_data;
        destination.clear();

        memset(&strm, 0, sizeof(strm));
        ret = inflateInit(&strm);
        if (ret != Z_OK) {
            std::cerr << "Error initializing inflate: " << ret << std::endl;
            return ret;
        }

        strm.avail_in = source.size();
        strm.next_in = reinterpret_cast<unsigned char *>(source.data());

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
            }
            have = CHUNK - strm.avail_out;
            if (have > 0) {
                decompressed_data.insert(decompressed_data.end(), out, out + have);
            }
        } while (strm.avail_out == 0);

        ret = inflateEnd(&strm);
        if (ret != Z_OK) {
            std::cerr << "Error ending inflate: " << ret << std::endl;
            return ret;
        }

        destination.swap(decompressed_data);
        return Z_OK;
    }

    static void add_string_to_vector(std::vector<char> &uncompressed_data,
                            const std::string my_string) {
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