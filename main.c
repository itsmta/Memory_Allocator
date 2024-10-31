#include "umem.h"
#include <stdio.h>

int main() {
    // Initialize memory region with 1024 bytes and first-fit allocation algorithm
    if (umeminit(8192, FIRST_FIT) != 0) {
        fprintf(stderr, "Failed to initialize memory.\n");
        return 1;
    }

    // Allocate 128 bytes
    void *ptr1 = umalloc(128);
    if (ptr1 != NULL) {
        printf("Allocated 128 bytes.\n");
    } else {
        printf("Failed to allocate 128 bytes.\n");
    }

    // Allocate another 256 bytes
    void *ptr2 = umalloc(256);
    if (ptr2 != NULL) {
        printf("Allocated 256 bytes.\n");
    } else {
        printf("Failed to allocate 256 bytes.\n");
    }

    // Reallocate the first pointer to 200 bytes
    ptr1 = urealloc(ptr1, 200);
    if (ptr1 != NULL) {
        printf("Reallocated to 200 bytes.\n");
    } else {
        printf("Failed to reallocate to 200 bytes.\n");
    }

    // Free the first pointer
    if (ufree(ptr1) == 0) {
        printf("Freed 200 bytes.\n");
    } else {
        printf("Failed to free 200 bytes.\n");
    }

    // Free the second pointer
    if (ufree(ptr2) == 0) {
        printf("Freed 256 bytes.\n");
    } else {
        printf("Failed to free 256 bytes.\n");
    }

    // Print memory statistics
    umemstats();

    return 0;
}
