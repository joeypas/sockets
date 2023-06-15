#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <buffer.hpp>
#include <snappy.h>

using namespace std;

int main() {

    vector<char> compressed_buf(0);
    vector<char> decompressed_buf(0);
    string scompressed_buf;
    string sdecompressed_buf;

    ifstream file;
    file.open("../README.MD", ifstream::in | ifstream::binary);
    vector<char> out_buf(0);

    if (!file.eof()) {
        file.seekg(0, std::ios_base::end);
        std::streampos fileSize(file.tellg());
        out_buf.resize(fileSize);

        file.seekg(0, std::ios_base::beg);
        file.read(&out_buf.front(), fileSize);
    }

    cout << "Uncompressed Size: " << out_buf.size() << endl;

    // Compress the buffer and store the compressed data in a new buffer
    buffer::compress_vector(out_buf, compressed_buf);

    snappy::Compress(out_buf.data(), out_buf.size(), &scompressed_buf);

    cout << "Compressed Size: " << compressed_buf.size() << endl;

    cout << "Snappy Compressed Size: " << scompressed_buf.size() << endl;

    // Decompress the buffer
    buffer::decompress_vector(compressed_buf, decompressed_buf);

    snappy::Uncompress(scompressed_buf.data(), scompressed_buf.size(), &sdecompressed_buf);

    cout << "Decompressed result: " << decompressed_buf.size() << endl;

    cout << "Snappy Decompressed result: " << sdecompressed_buf.size() << endl;

    return 0;
}
