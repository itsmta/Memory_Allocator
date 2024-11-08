#include "umem.h"
#include <stdio.h>
#include <string.h>

int main(){
    //main_test();
    //next_fit_test();
    //coalesce_test();
    next_fit_test2();
    //worst_fit_test();
}

//umalloc,free, realloc testing
int main_test() {
    //Initialize memory region with a sufficient size for testing fragmentation and coalescing
    umeminit(3500, 1);

    //Allocate a series of blocks of different sizes
    void *ptr1 = umalloc(256);  
    void *ptr2 = umalloc(128);  
    void *ptr3 = umalloc(256);  
    void *ptr4 = umalloc(512);  
    void *ptr5 = umalloc(128);  

    //Free some blocks to create fragmentation
    ufree(ptr2);
    ufree(ptr4);
    
    //Test for double free exit & output to stderr
    //ufree(ptr4);

    //Allocate and free additional blocks to increase fragmentation
    void *ptr6 = umalloc(64);    //fills gap from pointer 2 depending on algorithm
    void *ptr7 = umalloc(128);
    void *ptr8 = umalloc(64);
    ufree(ptr7);
    ufree(ptr6);

    //Further allocations that may fit into fragmented free spaces, increasing fragmentation
    void *ptr9 = umalloc(256);
    void *ptr10 = umalloc(128);
    ufree(ptr3);
    ufree(ptr5);

    //Coalescing test - Free adjacent blocks and observe if they are combined
    ufree(ptr1);
    ufree(ptr9);

    //urealloc Tests
    //urealloc with NULL pointer (should behave like umalloc)
    void *realloc_ptr1 = urealloc(NULL, 100); 
    printf("urealloc(NULL, 100) -> %p\n", realloc_ptr1);

    //urealloc with size 0 (should free realloc_ptr1)
    void *realloc_ptr2 = urealloc(realloc_ptr1, 0);
    printf("urealloc(realloc_ptr1, 0) -> %p\n", realloc_ptr2);

    //Expand an allocated block with sufficient contiguous space
    void *ptr11 = umalloc(128);
    printf("Original allocation at ptr11: %p\n", ptr11);
    ptr11 = urealloc(ptr11, 200);  //Expand to 200 bytes
    printf("Expanded ptr11 to 200 bytes: %p\n", ptr11);

    //Shrink an allocated block
    void *ptr12 = umalloc(300);
    printf("Original allocation at ptr12: %p\n", ptr12);
    ptr12 = urealloc(ptr12, 150);  //Shrink to 150 bytes
    printf("Shrunk ptr12 to 150 bytes: %p\n", ptr12);

    //Expand with insufficient contiguous space (causes reallocation)
    void *ptr13 = umalloc(64);
    printf("Original allocation at ptr13: %p\n", ptr13);
    ptr13 = urealloc(ptr13, 500);  //Expand to 500 bytes, should move to a new location
    printf("Reallocated ptr13 to 500 bytes: %p\n", ptr13);

    //End of test
    umemstats();
    return 0;
}


//next fit test
int next_fit_test() {
    umeminit(4096, NEXT_FIT); //Assuming `NEXT_FIT` is defined in your code as the identifier for Next Fit
    printf("Initialized memory with Next Fit algorithm.\n");

    //Allocate a series of blocks of different sizes
    void *ptr1 = umalloc(256);
    void *ptr2 = umalloc(128);
    void *ptr3 = umalloc(256);
    void *ptr4 = umalloc(512);
    void *ptr5 = umalloc(128);
    printf("Initial allocations done:\n");
    print_free_list();

    //Free some blocks to create fragmentation
    ufree(ptr2);
    ufree(ptr4);
    printf("Freed ptr2 and ptr4 to create fragmentation:\n");
    print_free_list(); //Show free list after creating fragmentation

    //Allocate additional blocks to test Next Fit's ability to skip used areas
    void *ptr6 = umalloc(64); //Should fit into the space left by ptr2 
    void *ptr7 = umalloc(128); //Will use the next available space after ptr6
    void *ptr8 = umalloc(64); //Next available space after ptr7
    printf("Allocated ptr6, ptr7, and ptr8:\n");
    print_free_list();

    //Free additional blocks to create more free space
    ufree(ptr3);
    ufree(ptr5);
    printf("Freed ptr3 and ptr5 to create more free space:\n");
    print_free_list();

    //Allocate again to see if Next Fit finds the correct free block
    void *ptr9 = umalloc(256);   //Should fit where ptr3 was if Next Fit is working properly
    void *ptr10 = umalloc(128);  //Should go to the next suitable block after ptr9
    printf("Allocated ptr9 and ptr10:\n");
    print_free_list();

    //Edge case - allocate and immediately free to see if next_fit updates properly
    void *temp = umalloc(200);
    ufree(temp);
    printf("Allocated and freed temp (200 bytes):\n");
    print_free_list();

    //Allocate in fragmented memory to test next_fit’s handling of available blocks
    void *ptr11 = umalloc(300);
    printf("Allocated ptr11 (300 bytes):\n");
    print_free_list();

    //Print final statistics and free list to observe Next Fit handling
    umemstats();

    return 0;
}

//Coalesce test/fragmentation
int coalesce_test() {
    umeminit(4096, 1);
    print_free_list();
    umeminit(4096, 1);
    print_free_list();


    //Allocate three blocks of memory
    void *ptr1 = umalloc(200);
    print_free_list();
    void *ptr2 = umalloc(100);
    print_free_list();
    void *ptr3 = umalloc(150);
    print_free_list();

    //Free the middle block (ptr2)
    ufree(ptr2);
    //no coalescing should occur.


    //Free the first block (ptr1)
    ufree(ptr1);
    //The two free blocks (ptr1 and ptr2) should be merged into a single block.

    //Free the last block (ptr3)
    //comment free out for fragmentation
    ufree(ptr3);
    //All free blocks should now be merged into a single large free block.

    //The entire memory region should now be a single free block.

    //Print memory statistics
    umemstats();
    print_free_list();

    return 0;
}

int next_fit_test2() {
    //Initialize memory with 1024 bytes and use the NEXT_FIT algorithm
    if (umeminit(4092, NEXT_FIT) != 0) {
        fprintf(stderr, "Failed to initialize memory allocator\n");
        return 1;
    }

    printf("Testing NEXT_FIT allocation algorithm...\n");

    //Allocate a few blocks to see how NEXT_FIT behaves
    void *ptr1 = umalloc(100);
    void *ptr2 = umalloc(200);
    void *ptr3 = umalloc(150);

    printf("\nAfter initial allocations:\n");
    print_free_list(); //Display the free list after allocations

    //Free the second block to create a gap in the free list
    ufree(ptr2);
    printf("\nAfter freeing the second block (200 bytes):\n");
    print_free_list();

    //Next fit should start from ptr3 (last allocated block) and wrap around if needed
    void *ptr4 = umalloc(50); //Should find the freed block (200 bytes) left by ptr2 and allocate from it.


    printf("\nAfter allocating 50 bytes (should fit into the freed block of 200 bytes):\n");
    print_free_list();

    //Allocate another block that doesn't fit in the 200-byte block but requires the remaining free memory
    void *ptr5 = umalloc(300); //Since ptr3 was the last allocated block, the next fit algorithm should search from

    printf("\nAfter allocating 300 bytes (should wrap and find the next suitable block):\n");
    print_free_list();

    //Free all allocated blocks to observe if the algorithm properly resets
    ufree(ptr1);
    ufree(ptr3);
    ufree(ptr4);
    ufree(ptr5);
    printf("\nAfter freeing all allocated blocks:\n");
    print_free_list();

    umemstats();

    return 0;
}

int worst_fit_test() {
    umeminit(4096, WORST_FIT);
    printf("Initialized memory with Worst Fit algorithm.\n");

    //Allocate a series of blocks of different sizes
    void *ptr1 = umalloc(256);
    void *ptr2 = umalloc(128);
    void *ptr3 = umalloc(512);
    void *ptr4 = umalloc(64);
    void *ptr5 = umalloc(256);
    printf("Initial allocations done:\n");
    print_free_list();

    //Free some blocks to create fragmentation
    ufree(ptr1);
    ufree(ptr3);
    ufree(ptr5);
    printf("Freed ptr1, ptr3, and ptr5 to create fragmentation:\n");
    print_free_list();

    //Allocate additional blocks to test Worst Fit's ability to find the largest block
    void *ptr6 = umalloc(200);
    void *ptr7 = umalloc(100);
    printf("Allocated ptr6 and ptr7 using Worst Fit:\n");
    print_free_list();

    //Free additional blocks to further increase available space
    ufree(ptr2);
    ufree(ptr4);
    printf("Freed ptr2 and ptr4 to create more free space:\n");
    print_free_list();

    //Allocate again to see if Worst Fit finds the correct largest free block
    void *ptr8 = umalloc(300);
    void *ptr9 = umalloc(150);
    printf("Allocated ptr8 and ptr9 using Worst Fit:\n");
    print_free_list();

    //Allocate and free small blocks to test if Worst Fit continues choosing the largest blocks
    void *temp1 = umalloc(50);
    void *temp2 = umalloc(75);
    printf("Allocated temp1 and temp2 (small blocks):\n");
    print_free_list();

    ufree(temp1);
    ufree(temp2);
    printf("Freed temp1 and temp2:\n");
    print_free_list();

    //Allocation to observe behavior in a fragmented free list
    void *ptr10 = umalloc(400);  //Should go into the largest available block
    printf("Allocated ptr10 (400 bytes) using Worst Fit:\n");
    print_free_list();

    //Print final statistics and free list to observe Worst Fit handling
    umemstats();

    return 0;
}