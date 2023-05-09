#include <buffer.hpp>
#include <string>

using namespace std;

int main() {

    std::vector<char> uncompressed(0);
    auto *my_string = (char *) "Hello, world!";
    buffer::add_string_to_vector(uncompressed, my_string);

    std::vector<char> compressed(0);
    int compression_result = buffer::compress_vector(uncompressed, compressed);
    assert(compression_result == F_OK);

    std::vector<char> decompressed(0);
    int decompression_result = buffer::decompress_vector(compressed, decompressed);
    assert(decompression_result == F_OK);

    printf("Uncompressed: %s\n", uncompressed.data());
    printf("Compressed: ");
    std::ostream &standard_output = std::cout;
    buffer::print_bytes(standard_output, (const unsigned char *) compressed.data(), compressed.size(), false);
    printf("Decompressed: %s\n", decompressed.data());

    return 0;
}