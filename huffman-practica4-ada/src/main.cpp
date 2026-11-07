#include "codec.h"
#include <cstdio>
#include <string>

static void usage(const char* p) {
    std::fprintf(stderr,
        "Uso:\n  %s c <entrada> <salida.huf>\n  %s d <entrada.huf> <salida>\n", p, p);
}

int main(int argc, char** argv) {
    if (argc != 4) { usage(argv[0]); return 1; }
    std::string mode = argv[1];
    if (mode == "c") return huf::compress(argv[2], argv[3])   ? 0 : 1;
    if (mode == "d") return huf::decompress(argv[2], argv[3]) ? 0 : 1;
    usage(argv[0]);
    return 1;
}