#include <h3api.h>
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    H3Index indexed = 0x8a2a1072b59ffffL;
    // Distance away from the origin to find:
    int k = 2;

    int maxNeighboring = maxKringSize(k);
    H3Index* neighboring = (H3Index*) calloc(maxNeighboring, sizeof(H3Index));
    kRing(indexed, k, neighboring);

    printf("Neighbors:\n");
    for (int i = 0; i < maxNeighboring; i++) {
        // Some indexes may be 0 to indicate fewer than the maximum
        // number of indexes.
        if (neighboring[i] != 0) {
            printf("%" PRIx64 "\n", neighboring[i]);
        }
    }

    free(neighboring);
}
