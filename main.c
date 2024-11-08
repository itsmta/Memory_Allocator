#include "umem.h"
#include <stdio.h>
#include <string.h>

/*int main() {
    //Initialize memory region with a sufficient size for testing fragmentation and coalescing
    umeminit(3500, 4);  //Using First Fit algorithm for demonstration

    //Step 1: Allocate a series of blocks of different sizes
    void *ptr1 = umalloc(256);  
    void *ptr2 = umalloc(128);  
    void *ptr3 = umalloc(256);  
    void *ptr4 = umalloc(512);  
    void *ptr5 = umalloc(128);  

    //Step 2: Free some blocks to create fragmentation
    ufree(ptr2);  //Free the 128-byte block (between ptr1 and ptr3)
    ufree(ptr4);  //Free the 512-byte block (between ptr3 and ptr5)
    
    //Test for double free exit & output to stderr
    //ufree(ptr4);

    //Step 3: Allocate and free additional blocks to increase fragmentation
    void *ptr6 = umalloc(64);    //fills gap from pointer 2 depending on algorithm
    void *ptr7 = umalloc(128);
    void *ptr8 = umalloc(64);
    ufree(ptr7);
    ufree(ptr6);

    //Step 4: Further allocations that may fit into fragmented free spaces, increasing fragmentation
    void *ptr9 = umalloc(256);
    void *ptr10 = umalloc(128);
    ufree(ptr3);
    ufree(ptr5);

    //Step 5: Coalescing test - Free adjacent blocks and observe if they are combined
    ufree(ptr1);
    ufree(ptr9);

    //Step 6: urealloc Tests

    //Test 1: urealloc with NULL pointer (should behave like umalloc)
    void *realloc_ptr1 = urealloc(NULL, 100);  //Allocate 100 bytes
    printf("urealloc(NULL, 100) -> %p\n", realloc_ptr1);

    //Test 2: urealloc with size 0 (should free realloc_ptr1)
    void *realloc_ptr2 = urealloc(realloc_ptr1, 0);  //Free the 100-byte block
    printf("urealloc(realloc_ptr1, 0) -> %p\n", realloc_ptr2);

    //Test 3: Expand an allocated block with sufficient contiguous space
    void *ptr11 = umalloc(128);  //Allocate 128 bytes
    printf("Original allocation at ptr11: %p\n", ptr11);
    ptr11 = urealloc(ptr11, 200);  //Attempt to expand to 200 bytes
    printf("Expanded ptr11 to 200 bytes: %p\n", ptr11);

    //Test 4: Shrink an allocated block
    void *ptr12 = umalloc(300);  //Allocate 300 bytes
    printf("Original allocation at ptr12: %p\n", ptr12);
    ptr12 = urealloc(ptr12, 150);  //Shrink to 150 bytes
    printf("Shrunk ptr12 to 150 bytes: %p\n", ptr12);

    //Test 5: Expand with insufficient contiguous space (causes reallocation)
    void *ptr13 = umalloc(64);   //Allocate 64 bytes
    printf("Original allocation at ptr13: %p\n", ptr13);
    ptr13 = urealloc(ptr13, 500);  //Expand to 500 bytes, should move to a new location
    printf("Reallocated ptr13 to 500 bytes: %p\n", ptr13);

    //End of test
    umemstats();
    return 0;
}*/


//next fit test
/*int main() {
    //Initialize memory region with a sufficient size and set the allocation algorithm to Next Fit
    umeminit(4096, 4);  //Using 4 as the identifier for Next Fit (assuming 4 represents Next Fit)

    //Step 1: Allocate a series of blocks of different sizes
    void *ptr1 = umalloc(256);
    void *ptr2 = umalloc(128); 
    void *ptr3 = umalloc(256);  
    void *ptr4 = umalloc(512);  
    void *ptr5 = umalloc(128);  
    printf("Initial allocations done.\n");

    //Step 2: Free some blocks to create fragmentation
    ufree(ptr2);  //Free 128-byte block (between ptr1 and ptr3)
    ufree(ptr4);  //Free 512-byte block (between ptr3 and ptr5)
    printf("Freed ptr2 and ptr4 to create fragmentation.\n");

    //Step 3: Allocate additional blocks to test Next Fit's ability to skip used areas
    void *ptr6 = umalloc(64);    //Should fit into the space left by ptr2 
    void *ptr7 = umalloc(128);
    void *ptr8 = umalloc(64);
    printf("Allocated ptr6, ptr7, and ptr8.\n");

    //Step 4: Free a few more blocks
    ufree(ptr3); 
    ufree(ptr5); 
    printf("Freed ptr3 and ptr5 to create more free space.\n");

    //Step 5: Allocate again to see if Next Fit finds the correct free block
    void *ptr9 = umalloc(256);   //This should fit where ptr3 was if Next Fit is working properly
    void *ptr10 = umalloc(128);  //Allocate again to see Next Fit behavior
    printf("Allocated ptr9 and ptr10.\n");

    //Step 6: Edge case - allocate and immediately free to see if next_fit updates properly
    void *temp = umalloc(200);  //Allocate 200 bytes
    ufree(temp);                //Immediately free it
    //ufree(temp);                //Double free
    printf("Allocated and freed temp.\n");

    //Step 7: Allocate with Next Fit in fragmented memory
    void *ptr11 = umalloc(300); //Should fit into the largest available fragment
    printf("Allocated ptr11.\n");

    //Print final statistics and free list to observe Next Fit handling
    umemstats();


    return 0;
}*/

//Coalesce test
int main() {
    umeminit(4096, 1);

    //Allocate three blocks of memory
    void *ptr1 = umalloc(200);
    void *ptr2 = umalloc(100);
    void *ptr3 = umalloc(150);

    //Free the middle block (ptr2)
    ufree(ptr2);
    //no coalescing should occur.


    //Free the first block (ptr1)
    ufree(ptr1);
    //The two free blocks (ptr1 and ptr2) should be merged into a single block.

    //Free the last block (ptr3)
    ufree(ptr3);
    //All free blocks should now be merged into a single large free block.

    //The entire memory region should now be a single free block.

    //Print memory statistics
    umemstats();

    return 0;
}