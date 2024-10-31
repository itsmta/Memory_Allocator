#include "umem.h"
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

static void *memory_region = NULL;    // Base pointer for memory region
static size_t total_memory = 0;       // Total size of the memory region
static int alloc_algorithm = FIRST_FIT;  // Default allocation algorithm (can be changed)

static size_t allocated_memory = 0;   // Track allocated memory
static size_t free_memory = 0;        // Track free memory
static int total_allocations = 0;     // Track total allocations
static int total_deallocations = 0;   // Track total deallocations
static node_t *free_list = NULL;      // Head of the free list

int umeminit(size_t sizeOfRegion, int allocationAlgo){

    int pageSize = getpagesize();

    fprintf(stdout,"Page size: %d bytes\n", pageSize);

    //Check if memory region is initialized already
   if (memory_region != NULL) {
        fprintf(stderr, "Memory region is already initialized.\n");
        return -1;
    }

    //Open /dev/zero for mmap
    int fd = open("/dev/zero", O_RDWR);
    if (fd == -1) {
        perror("Failed to open /dev/zero");
        return -1;
    }

    //Map memory from /dev/zero to simulate a contiguous memory region
    memory_region = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (memory_region == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    // Close /dev/zero since it’s no longer needed
    close(fd);

    total_memory = sizeOfRegion;
    alloc_algorithm = allocationAlgo;
    free_memory = sizeOfRegion;

    // Initialize the free list with the full block
    node_t* initial_free_block = (node_t *)memory_region;  // Start of free list at beginning of memory region
    initial_free_block->size = sizeOfRegion - sizeof(node_t);  // Free block size after accounting for node_t header
    initial_free_block->next = NULL;  // No other free blocks initially

    // Set free list head to the full block
    free_list = initial_free_block;
    fprintf(stdout, "free list size: %ld\n", free_list->size);
    return 0;  // Success
}

void* umalloc(size_t size){

        // Check if umeminit was called by main before memory allocation
    if (memory_region == NULL) {
        fprintf(stderr, "Memory region is not initialized.\n");
        return NULL;
    }

    // Check if free space is available for the size of the requested allocation
    if (size == 0 || size > free_memory) {
        fprintf(stderr, "Requested size is invalid or exceeds available memory.\n");
        return NULL;
    }

    // Calculate total required size, including header
    size_t allocation_size = size + sizeof(header_t);
    node_t *prev = NULL;
    node_t *current = free_list;

    // Search for a suitable block (First Fit)
    while (current != NULL) {
        printf("Checking free block at %p with size %zu\n", current, current->size);
        if (current->size >= allocation_size) {
            break;  // Found a block that fits
        }
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        fprintf(stderr, "No sufficient free block found.\n");
        return NULL;
    }
    
    fprintf(stdout, "Free list size before allocation: %ld\n", current->size);

    // Determine if we can split the block
    if (current->size >= allocation_size + sizeof(node_t)) {
        // Create a new free block for the remaining memory after allocation
        node_t *new_free_block = (node_t *)((char *)current + allocation_size);
        new_free_block->size = current->size - allocation_size;
        new_free_block->next = current->next;

        if (prev == NULL) {
            free_list = new_free_block;  // Update the free list head if first block is used
        } else {
            prev->next = new_free_block;  // Link previous free block to new free block
        }

        printf("Split block. New free block at %p with size %zu\n", new_free_block, new_free_block->size);
    } else {
        // Use the entire block if it's too small to split
        if (prev == NULL) {
            free_list = current->next;
        } else {
            prev->next = current->next;
        }

        printf("Used entire block without splitting.\n");
    }

    // Now set up the header for the allocated block
    header_t *header = (header_t *)current;
    header->size = size;           // Set the allocated size (not the full block size)
    header->magic = MAGIC;          // Integrity check value
    void *allocated_memory_ptr = (void *)(header + 1); // Pointer to memory after the header

    fprintf(stdout, "Allocated block size (header): %zu\n", header->size);

    // Update memory tracking
    free_memory -= allocation_size;
    allocated_memory += size;
    total_allocations++;

    printf("Allocated %zu bytes at %p\n", size, allocated_memory_ptr);
    printf("Free memory after allocation: %zu bytes\n", free_memory);

    return allocated_memory_ptr;
}

int ufree(void* ptr){
    return 0;
}

void* urealloc(void* ptr, size_t size){
    return 0;
}

void umemstats(void){
    double fragmentation = 0.0;  // Calculate fragmentation if relevant

    printumemstats(total_allocations, total_deallocations, allocated_memory, free_memory, fragmentation);
}

// Clean up memory when done (not part of API but useful for testing)
void cleanup() {
    if (memory_region) {
        free(memory_region);
        memory_region = NULL;
    }
}