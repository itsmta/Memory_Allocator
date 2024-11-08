#include "umem.h"
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

static void* memory_region = NULL;    //Base pointer for memory region
static size_t total_memory = 0;       //Total size of the memory region
static int alloc_algorithm = FIRST_FIT;  //Default allocation algorithm

static size_t allocated_memory = 0;   //Track allocated memory
static size_t free_memory = 0;        //Track free memory
static int total_allocations = 0;     //Track total allocations
static int total_deallocations = 0;   //Track total deallocations
static node_t* free_list = NULL;      //Head of the free list
static node_t* last_allocated = NULL; //Keeps track of last allocated's next node in the free list

//Function declarations
void coalesce(node_t* new_free_node);
node_t* first_fit(size_t allocation_size, node_t** selected_prev);
node_t* best_fit(size_t allocation_size, node_t** selected_prev);
node_t* worst_fit(size_t allocation_size, node_t** selected_prev);
node_t* next_fit(size_t allocation_size, node_t** selected_prev);
double calculate_fragmentation(void);

//Debugger function
void print_free_list();

//For my image maker
//void log_memory_operation(const char* operation, void* address, size_t size);

int umeminit(size_t sizeOfRegion, int allocationAlgo) {
    int pageSize = getpagesize();

    //Round up sizeOfRegion to the nearest multiple of pageSize
    sizeOfRegion = ((sizeOfRegion + pageSize - 1) / pageSize) * pageSize;

    //Check if memory region is already initialized
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
        exit(1);
    }

    //Close /dev/zero since it’s no longer needed
    close(fd);

    //Update memory statistics
    total_memory = sizeOfRegion;
    alloc_algorithm = allocationAlgo;
    free_memory = sizeOfRegion;

    //Initialize the free list with the full block (including header space)
    node_t* initial_free_block = (node_t* )memory_region;  //Start of free list at beginning of memory region
    initial_free_block->size = sizeOfRegion;  //Full region is free initially (header overhead will be accounted for during allocation)
    initial_free_block->next = NULL;  //No other free blocks initially

    //Set free list head to the full block
    free_list = initial_free_block;
    
    return 0;  //Success
}



void* umalloc(size_t size) {
    if (memory_region == NULL) {
        fprintf(stderr, "Memory region is not initialized.\n");
        return NULL;
    }

    if (size == 0 || size > free_memory) {
        fprintf(stderr, "Requested size is invalid or exceeds available memory.\n");
        return NULL;
    }

    //Round up the requested size to the nearest multiple of 8
    size = (size + 7) & ~7;

    //Calculate the total required size with header alignment
    size_t allocation_size = size + sizeof(header_t);
    node_t* selected_prev = NULL;
    node_t* selected = NULL;

    //Choose the allocation algorithm based on alloc_algorithm
    switch (alloc_algorithm) {
        case FIRST_FIT:
            selected = first_fit(allocation_size, &selected_prev);
            break;
        case BEST_FIT:
            selected = best_fit(allocation_size, &selected_prev);
            break;
        case WORST_FIT:
            selected = worst_fit(allocation_size, &selected_prev);
            break;
        case NEXT_FIT:
            selected = next_fit(allocation_size, &selected_prev);
            break;
        default:
            fprintf(stderr, "Unknown allocation algorithm.\n");
            return NULL;
    }

    if (selected == NULL) {
        fprintf(stderr, "No sufficient free block found.\n");
        return NULL;
    }

    //Determine if we can split the block
    if (selected->size >= allocation_size + sizeof(node_t)) {
        //Create a new free block for the remaining memory after allocation
        node_t* new_free_block = (node_t* )((char* )selected + allocation_size);
        new_free_block->size = selected->size - allocation_size;
        new_free_block->next = selected->next;

        //Update the previous block's next pointer
        if (selected_prev == NULL) {
            free_list = new_free_block;
        } else {
            selected_prev->next = new_free_block;
        }

        //Update last_allocated to the new free block
        if (alloc_algorithm == NEXT_FIT) {
            last_allocated = new_free_block;
        }
    } else {
        //Use the entire block if it's too small to split
        if (selected_prev == NULL) {
            free_list = selected->next;
        } else {
            selected_prev->next = selected->next;
        }

        //Update last_allocated to the next free block
        if (alloc_algorithm == NEXT_FIT) {
            last_allocated = selected->next;
        }
    }

    //Prepare the allocated block with a header
    header_t* header = (header_t* )selected;
    header->size = size;  //Store the requested size (not the full block size)
    header->magic = MAGIC;  //Set magic number for integrity check
    void* allocated_memory_ptr = (void* )(header + 1);  //Return memory after the header

    //Update memory statistics
    free_memory -= allocation_size;
    allocated_memory += size;
    total_allocations++;

    //Return the memory address to the user (after the header)
    return allocated_memory_ptr;
}


void ufree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    //Get the header of the block to free
    header_t* header = (header_t* )((char* )ptr - sizeof(header_t));

    //Check for memory corruption by verifying the MAGIC number
    if (header->magic != MAGIC) {
        fprintf(stderr, "Error: Memory corruption detected at block %p\n", ptr);
        exit(1);  //Exit on memory corruption as per specification
    }

    //Mark block as free by resetting the magic number
    header->magic = 0;

    //Update memory statistics
    size_t allocation_size = header->size + sizeof(header_t);
    free_memory += allocation_size;
    allocated_memory -= header->size;
    total_deallocations++;

    //Create a new free node for the block being freed
    node_t* new_free_node = (node_t* )header;
    new_free_node->size = allocation_size; 
    new_free_node->next = NULL;

    //Insert the freed block back into the free list in sorted order by address
    node_t* prev = NULL;
    node_t* current = free_list;

    while (current != NULL && current < new_free_node) {
        prev = current;
        current = current->next;
    }

    //Link the new free block into the list
    new_free_node->next = current;
    if (prev == NULL) {
        free_list = new_free_node;
    } else {
        prev->next = new_free_node;
    }

    //If the new free node is before last_allocated, update last_allocated
    if (alloc_algorithm == NEXT_FIT && (last_allocated == NULL || new_free_node < last_allocated)) {
        last_allocated = new_free_node;
    }

    //Call the coalesce function to merge adjacent free blocks
    coalesce(new_free_node);

}

//Function to coalesce adjacent free blocks
void coalesce(node_t* new_free_node) {


    //Coalesce with next free block if adjacent
    if (new_free_node->next != NULL &&
        (char* )new_free_node + new_free_node->size == (char* )new_free_node->next) {

        new_free_node->size += new_free_node->next->size;
        new_free_node->next = new_free_node->next->next;
    }

    //Coalesce with previous free block if adjacent
    node_t* prev = NULL;
    node_t* current = free_list;

    //Find the previous block in the free list
    while (current != NULL && current < new_free_node) {
        prev = current;
        current = current->next;
    }

    if (prev != NULL &&
        (char* )prev + prev->size == (char* )new_free_node) {

        prev->size += new_free_node->size;
        prev->next = new_free_node->next;
    }
}

void* urealloc(void* ptr, size_t size) {
    //If ptr is NULL, behave like umalloc
    if (ptr == NULL) {
        return umalloc(size);
    }
    //If size is 0, behave like ufree
    if (size == 0) {
        ufree(ptr);
        return NULL;
    }

    //Get the header of the current block
    header_t* header = (header_t* )ptr - 1;
    size_t current_size = header->size;

    //Round up the new requested size to the nearest multiple of 8 for alignment
    size = (size + 7) & ~7;

    //If the requested size is smaller than or equal to the current block, we can resize in place
    if (size <= current_size) {
        header->size = size;  //Simply update the header's size to the new requested size
        return ptr;
    }

    //Calculate the total allocation size with header for the requested size
    size_t allocation_size = size + sizeof(header_t);

    //Check if we can expand the block in place by checking the next free block
    node_t* next_block = (node_t* )((char* )header + sizeof(header_t) + header->size);

    //Verify if next_block is a free block by checking if it exists in the free list
    node_t* prev = NULL;
    node_t* current = free_list;
    int is_free = 0;

    while (current != NULL) {
        if (current == next_block) {
            is_free = 1;
            break;
        }
        prev = current;
        current = current->next;
    }

    if (is_free && next_block->size >= allocation_size - current_size) {
        //Expand the block
        if (next_block->size >= allocation_size - current_size + sizeof(node_t)) {
            //Split the next block if there's extra space
            node_t* new_free_block = (node_t* )((char* )next_block + allocation_size - current_size);
            new_free_block->size = next_block->size - (allocation_size - current_size);
            new_free_block->next = next_block->next;

            //Update the free list
            if (prev == NULL) {
                free_list = new_free_block;
            } else {
                prev->next = new_free_block;
            }
        } else {
            //Remove the entire next block from the free list if it cannot be split
            if (prev == NULL) {
                free_list = next_block->next;
            } else {
                prev->next = next_block->next;
            }
        }

        //Update header to the new size and free/allocated memory tracking
        header->size = size;
        free_memory -= (allocation_size - current_size);
        allocated_memory += (size - current_size);
        return ptr;
    }

    //Case 5: Allocate a new block, copy data, free old block
    void* new_ptr = umalloc(size);
    if (new_ptr == NULL) {
        return NULL; 
    }

    //Copy data from old block to new block
    size_t bytes_to_copy;
    if (current_size < size) {
        bytes_to_copy = current_size;
    } else {
        bytes_to_copy = size;
    }
    memcpy(new_ptr, ptr, bytes_to_copy);

    //Free the old block
    ufree(ptr);

    return new_ptr;
}

void umemstats(void){
    double fragmentation = calculate_fragmentation();

    printumemstats(total_allocations, total_deallocations, allocated_memory, free_memory, fragmentation);
}

//First Fit algorithm: Find the first block that fits the requested size
node_t* first_fit(size_t allocation_size, node_t** selected_prev) {
    node_t* prev = NULL;
    node_t* current = free_list;

    while (current != NULL) {
        if (current->size >= allocation_size) {
            *selected_prev = prev;
            return current;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

//Best Fit algorithm: Find the smallest block that fits the requested size
node_t* best_fit(size_t allocation_size, node_t** selected_prev) {
    node_t* prev = NULL;
    node_t* current = free_list;
    node_t* best = NULL;
    *selected_prev = NULL;

    while (current != NULL) {
        if (current->size >= allocation_size &&
            (best == NULL || current->size < best->size)) {
            best = current;
          *  selected_prev = prev;
        }
        prev = current;
        current = current->next;
    }
    return best;
}

//Worst Fit algorithm: Find the largest block that fits the requested size
node_t* worst_fit(size_t allocation_size, node_t** selected_prev) {
    node_t* prev = NULL;
    node_t* current = free_list;
    node_t* worst = NULL;
    *selected_prev = NULL;

    while (current != NULL) {
        if (current->size >= allocation_size &&
            (worst == NULL || current->size > worst->size)) {
            worst = current;
            *selected_prev = prev;
        }
        prev = current;
        current = current->next;
    }
    return worst;
}

//Next Fit algorithm: Find the next block from the last allocated block
node_t* next_fit(size_t allocation_size, node_t** selected_prev) {
    if (free_list == NULL) {
        return NULL;  //No free blocks available
    }

    node_t* prev = NULL;
    node_t* current;

    //Start the search from the node immediately after last_allocated, or from the head if last_allocated is NULL
    if (last_allocated == NULL || last_allocated->next == NULL) {
        current = free_list;
        prev = NULL;
    } else {
        current = last_allocated->next;
        prev = last_allocated;
    }

    //Keep a pointer to the start of our search to know when we've wrapped around
    node_t* start = current;

    //Search from current to the end of the free list, wrapping around if necessary
    do {
        if (current->size >= allocation_size) {
            //We found a suitable block, so set last_allocated and return
            *selected_prev = prev;
            last_allocated = current;
            return current;
        }

        //Move to the next node
        prev = current;
        current = current->next ? current->next : free_list;  //Wrap around if at the end
    } while (current != start);

    //No suitable block found
    return NULL;
}

//Debugger for checking the free list
void print_free_list() {
    node_t* current = free_list;
    printf("Free list: ");
    while (current != NULL) {
        printf("%p: %zu", (void*)current, current->size);
        current = current->next;
        if (current != NULL) {
            printf(", ");
        }
    }
    printf("\n");
}

double calculate_fragmentation(void) {
    double fragmentation = 0.0;
    size_t largest_free_block_size = 0;
    size_t total_small_free_blocks = 0;
    node_t* current = free_list;

    //First pass: Find the largest free block size
    while (current != NULL) {
        if (current->size > largest_free_block_size) {
            largest_free_block_size = current->size;
        }
        current = current->next;
    }

    //If there is no free memory, fragmentation is zero
    if (largest_free_block_size == 0 || free_memory == 0) {
        fragmentation = 0.0;
    } else {
        //Define threshold as half the size of the largest free block
        size_t threshold = largest_free_block_size / 2;

        //Second pass: Sum up memory in small free blocks
        current = free_list;
        while (current != NULL) {
            if (current->size < threshold) {
                total_small_free_blocks += current->size;
            }
            current = current->next;
        }

        //Calculate fragmentation percentage
        fragmentation = ((double)total_small_free_blocks / (double)free_memory) * 100.0;
    }

    return fragmentation;
}

//Function to log memory operations for image generation
/*void log_memory_operation(const char* operation, void* address, size_t size) {
    FILE* file = fopen("memory_log.txt", "a");
    if (file == NULL) {
        perror("Failed to open memory log file");
        return;
    }
    fprintf(file, "%s %p %zu\n", operation, address, size);
    fclose(file);
}*/