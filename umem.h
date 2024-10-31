#ifndef _UMEM_H
#define _UMEM_H

#include <stddef.h>
#include <stdio.h>

#define MAGIC 0xDEADBEEFLL          // Magic number used for detecting memory corruption

#define BEST_FIT 					(1)
#define WORST_FIT 					(2)
#define FIRST_FIT 					(3)
#define NEXT_FIT 					(4)
#define BUDDY						(5)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// structures : Both structures are required and are 64 bit. 
//              These structures are each 16 bytes in length.
//
typedef struct {
    long size;              // Size of the block
    long magic;             // Magic number for integrity check
} header_t;

typedef struct __node_t {
    long size;              // Size of the free block
    struct __node_t *next;  // Pointer to the next free block
} node_t;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// function prototypes
//
int 	umeminit(size_t sizeOfRegion, int allocationAlgo);
void 	*umalloc(size_t size);
void    *urealloc(void *ptr, size_t size);
int 	ufree(void *ptr);
void    umemstats(void);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
 * Macro: printumemstats
 * 
 * This macro is used to print memory allocation statistics (bytes allocated, 
 * bytes freed, and the number of allocations). The function is implemented 
 * as a macro to avoid multiple definition issues that might arise when included 
 * in multiple .c files, which can happen if we define this in the header file.
 * 
 * Warning:
 * This is a bit of a hack and not generally recommended in larger projects 
 * or production code. Typically, function definitions should go into a .c 
 * file, and only declarations should be in the header. However, to minimize 
 * the number of files and simplify the assignment setup, we're using a macro 
 * here. Note that macros do not offer the same type safety as functions, and 
 * debugging may be more challenging.
 */

 #define printumemstats(total_allocations, total_deallocations, allocated_memory, free_memory, fragmentation) \
    do {                                                                                                   \
        printf("Memory Allocation Statistics:\n");                                                        \
        printf("Total Allocations: %d\n", total_allocations);                                             \
        printf("Total Deallocations: %d\n", total_deallocations);                                         \
        printf("Currently Allocated Memory: %zu bytes\n", allocated_memory);                              \
        printf("Currently Free Memory: %zu bytes\n", free_memory);                                        \
        printf("Memory Fragmentation: %.2f%%\n", fragmentation);                                          \
    } while(0)

#endif