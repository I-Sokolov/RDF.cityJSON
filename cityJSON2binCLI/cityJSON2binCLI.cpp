
#include <stdio.h>

#include "cityJson2binAPI.h"

int main(int argc, const char* argv[])
{
    if (argc < 3) {
        printf("USAGE:\n");
        printf("\t%s <input cityJSON file path> <output RDF bin file path>\n", argv[0]);
        return -1;
    }

    printf("Converting %s to %s....\n", argv[1], argv[2]);

    auto res = cityJson2bin_Convert(argv[1], argv[2]);

    /*
    switch (res)
    {
        case enum_cityJson2bin_result::OK:
            printf("Converted successfully\n");
            break;
        case enum_cityJson2bin_result::FailRead:
            printf("ERROR: failed to read input file\n");
            break;
        default:
            printf("ERROR: code %d\n", (int) res);
            break;
    }
    */
    if (res) {
        printf("ERROR: %s\n", res);
    }

    return res ? 1 : 0;
}

