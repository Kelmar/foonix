/*************************************************************************/
/*************************************************************************/

//#include "assert.h"
#include <string.h>

#include <kernel/allocator.h>
#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/arch/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/vm/vm.h>

//#include <kernel/page.h>

#include "cpu.h"

/*************************************************************************/

extern KernelArgs g_KernelArguments;

PageBlock NullBlock(0, 0);

using namespace VM;

/*************************************************************************/

void VM::Init()
{
    Arch::InitBootMemory(&g_KernelArguments);

    g_KernelArguments.ShowAvailableMemory();

    //Kernel::ErrorCode errCode = Arch::InitPaging(ka);

    /*
    if (errCode != Kernel::ErrorCode::NoError)
    {
        Debug::Panic("ERROR: Unable to initialize paging");
    }
    */
}

/*************************************************************************/
/**
 * @brief Our boot up allocator for the page frame allocator.
 */
class BootPageAllocator : public PageAllocator
{
public:
    /* constructor */ BootPageAllocator()
    { 
        // Don't do anything here, initialization happens before global initializers.
    }

    // We just "allocate" frames out of the g_KernelArguments structure.
    virtual PageBlock NewBlock(size_t count)
    {
        size_t byteSize = count * PAGE_SIZE;

        // Start at the bottom of memory and work our way down
        size_t target = g_KernelArguments.MemoryMapEntries - 1; 
        size_t index = target;

        while (index < target)
        {
            if (g_KernelArguments.MemoryMap[index].Length >= index)
            {
                // Okay we have our frames
                /*
                physical_addr_t addr =
                    (physical_addr_t)(g_KernelArguments.MemoryMap[index].End() - byteSize);
                */
                
                // Add them to the boot page table
                logical_addr_t blockAddr = 0; //Arch::AddBootFrames(addr, count);

                PageBlock p(blockAddr, count);

                g_KernelArguments.MemoryMap[index].Length -= byteSize;

                if (g_KernelArguments.MemoryMap[index].Length == 0)
                    --g_KernelArguments.MemoryMapEntries;

                return p;
            }

            --index;
        }

        return NullBlock;
    }

    virtual void FreeBlock(PageBlock &block)
    {
        (void)block;
    }
} g_BootPageAlloc;

/*************************************************************************/
