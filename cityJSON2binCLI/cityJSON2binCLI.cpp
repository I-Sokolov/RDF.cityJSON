
#include <stdio.h>

#include "cityJson2bin.h"

int main(int argc, const char* argv[])
{
    if (argc < 3) {
        printf("USAGE:\n");
        printf("\t%s <input cityJSON file path> <output RDF bin file path>\n", argv[0]);
        return -1;
    }

    cityJson2bin_Convert(argv[1], argv[2]);

    return 0;
}

