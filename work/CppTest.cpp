//#include <termios.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//#include <unistd.h>

#include <stdio.h>

#include <utility>

#include "AVLTree.h"
#include "PageFrameAllocator.h"

#define NUM_ITEMS 1000

int testItems[NUM_ITEMS]; // = { 9, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
int testValues[NUM_ITEMS];

int RandTo(int max)
{
    double d = (double)rand() / (double)RAND_MAX;
    return (int)(d * (max - 1));
}

void InitTestValues()
{
    //size_t max = 0;

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        testItems[i] = i;

        int v = RandTo(4) + 1;

        testValues[i] = 4096 * v;
        //max += testValues[i];
    }

    //printf("To allocate: %ul\r\n", max);
}

void ShuffleItems()
{
    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int v = RandTo(NUM_ITEMS);
        std::swap(testItems[i], testItems[v]);
    }
}

int main()
{
    InitTestValues();

    ShuffleItems();

#if 0
    IntTree test;

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int value = testItems[i];

        test.Insert(value, value);
    }
 
    if (test.GetHeight() == -1)
    {
        printf("TREE INBALANCED!\r\n");
        test.Print();
        return 1;
    }

    //test.Print();
    Shuffle();

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int value = testItems[i];

        if (!test.Remove(value))
        {
            printf("Index: %d\r\n", i);

            printf("Failure removing item: %d\r\n", value);

            if (i > 0)
                printf("Previous item: %d\r\n", testItems[i - 1]);

            test.CheckTree();
            int h = test.GetHeight();

            if (h == -1)
                printf("Tree is inbalanced!\r\n");
        }
    }

    if (test.Count() != 0)
    {
        printf("The tree is not empty!!!\r\n");
    }
 
    // Tree should be empty
    test.Print();
#endif

#if 1
    // Simulate 32 MB
    PageFrameAllocator test(32ull * 1024 * 1024);

    PageBlock pb = test.Aquire(0x000A0000, 65536);

    printf("Addr: 0x%08X - %d\r\n", pb.Start, pb.Size);

#if 0
    PageBlock blocks[NUM_ITEMS];

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int index = testItems[i];

        blocks[index] = test.Allocate(testValues[index]);

        if (!blocks[index])
        {
            printf("Out of memory!?\r\n");
            return 1;
        }
    }

    ShuffleItems();
    
    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int index = testItems[i];

        test.Release(std::move(blocks[index]));
    }

    if (!test.CheckAllFree())
    {
        printf("Not all items were released!\r\n");
        return 1;
    }
#endif

#endif

    return 0;
}
