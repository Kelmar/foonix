/*************************************************************************/
/*************************************************************************/

//#include "assert.h"
#include <string.h>

#include <kernel/allocator.h>
#include <kernel/bitmap.h>
#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/arch/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/vm/vm.h>

#include "cpu.h"

/*************************************************************************/

namespace
{
    struct BuddyFrame
    {
        BuddyFrame *Parent;
        bool Allocated;
        size_t Size;

        BuddyFrame *Left;
        BuddyFrame *Right;

        BuddyFrame()
            : Parent(nullptr)
            , Allocated(false)
            , Size(0)
            , Left(this)
            , Right(this)
        {
        }
    };

    BuddyFrame RootFrame;

    // Pool of availble buddy frames not being used.
    BuddyFrame FreePool;

    int NextPow2(int base)
    {
        // Brute force method
        int i = 1;

        while (i < base)
            i << 1;

        return i;
    }

    /**
     * @brief Attempts to allocate a frame
     * @remark This will not allocate new pages for the frame.
     */
    BuddyFrame *AllocFrameRaw(void)
    {
        BuddyFrame *frame = FreePool.Left;

        if (frame == &FreePool)
            return nullptr; // Out of frames

        frame->Left->Right = &FreePool;
        FreePool.Left = frame->Left;
        frame->Left = nullptr;
        frame->Right = nullptr;

        return frame;
    }

    void ReleaseFrame(BuddyFrame *frame)
    {
        if (!frame)
            return;

        // Drop the frame back into the pool, we'll use it again later.
        frame->Left = FreePool.Left;
        frame->Left->Right = frame;
        frame->Right = &FreePool;
        FreePool.Left = frame;
    }

    /**
     * @brief Attempts to split a frame
     * @return NoError if the frame was split.
     * @return OutOfMemory if there are no frames available from the current pages.
     *
     * This function will not attempt to allocate any new pages for the frames.
     */
    Kernel::ErrorCode SplitFrameRaw(BuddyFrame *parent)
    {
        ASSERT(parent, "Invalid buddy frame parent");

        if (parent->Allocated)
            return Kernel::ErrorCode::AlreadyInUse;

        BuddyFrame *left = AllocFrameRaw();
        BuddyFrame *right = AllocFrameRaw();

        if (!left || !right)
        {
            ReleaseFrame(left);
            ReleaseFrame(right);

            return Kernel::ErrorCode::OutOfMemory;
        }

        left->Size = right->Size = parent->Size >> 1;

        parent->Allocated = true;
        parent->Left = left;
        parent->Right = right;

        return Kernel::ErrorCode::NoError;
    }
}

/*************************************************************************/

void InitPageAllocator(KernelArgs *ka)
{
    /*
     * This will almost certianly end up with areas that do not actually exist.
     *
     * That is okay, we're going to mark those areas as unavailable.
     */
    RootFrame.Size = NextPow2(ka->MemorySizeKByte);
}

/*************************************************************************/

