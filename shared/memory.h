#ifndef MEMORY_H
#define MEMORY_H

#include "shared/efi_global_state.h"

static void * malloc(unsigned long long size)
{
    EFI_STATUS error;
    void * handle;
    error = efi_boot_services()->AllocatePool(EFI_MEMORY_TYPE_EfiLoaderData, size, &handle);
    return error ? 0 : handle;
}

static void free(void * ptr)
{
    efi_boot_services()->FreePool(ptr);
}

static void * malloc_aligned(unsigned long long size, unsigned long long alignment, unsigned char * out_offset)
{
    EFI_STATUS error;
    void * handle;
    
    if ((alignment & (alignment - 1)) != 0) return 0; // Alignment must be power of two
    if (alignment > 255) return 0; // Alignment must not exceed 255 bytes

    error = efi_boot_services()->AllocatePool(
        EFI_MEMORY_TYPE_EfiLoaderData, 
        size + alignment - 1, // Allocate enough pages to ensure alignment
        &handle
    );
    if (error) return 0;

    // Align the address to the specified alignment
    const EFI_UINT64 handleAddr = (EFI_UINT64)handle;
    const EFI_UINT64 misalignment = handleAddr % alignment;
    const EFI_UINT64 aligned_addr = 
        misalignment == 0 ? 
            handleAddr : 
            handleAddr + alignment - misalignment;

    void * aligned_ptr = (void *)aligned_addr;
    *out_offset = (unsigned char)((EFI_UINT64)aligned_ptr - handleAddr); // Store the offset from the original handle
    return aligned_ptr;
}

static void free_aligned(void * ptr, unsigned char offset)
{
    // Freeing aligned memory is done by freeing the original handle
    EFI_UINT64 aligned_addr = (EFI_UINT64)ptr - offset;
    efi_boot_services()->FreePool((void *)aligned_addr);
}

static int numPages(unsigned long long size)
{
    // Assuming a page size of 4096 bytes
    return (size + 4095) / 4096; // Round up to the nearest page
}

static void * malloc_page_aligned(unsigned long long size)
{
    EFI_STATUS error;
    EFI_PHYSICAL_ADDRESS handle;
    error = efi_boot_services()->AllocatePages(
        EFI_ALLOCATE_TYPE_AllocateAnyPages, 
        EFI_MEMORY_TYPE_EfiLoaderData, 
        numPages(size),
        &handle
    );
    return error ? 0 : (void*)handle;
}

static void free_pages(void * ptr, unsigned long long size)
{
    // Freeing aligned memory is done by freeing the pages
    EFI_PHYSICAL_ADDRESS address = (EFI_PHYSICAL_ADDRESS)ptr;
    efi_boot_services()->FreePages(address, numPages(size));
}

static void * memcpy(void * dst, const void * src, unsigned long long len)
{
    efi_boot_services()->CopyMem(dst, (void*)src, len);
    return dst;
}

static void * memset(void * dst, int value, unsigned long long len)
{
    efi_boot_services()->SetMem(dst, len, value);
    return dst;
}

//Unoptimized memcpy that needs no boot_services pointer
static void * memcpy_naive(void * dst, void * src, unsigned long long len)
{
    char * dst_char = (char*)dst;
    char * src_char = (char*)src;
    for (int i = 0; i < len; i++){
        dst_char[i] = src_char[i];
    }
    return dst;
}

//Unoptimized memset that needs no boot_services pointer
static void * memset_naive(void * dst, int value, unsigned long long len)
{
    unsigned char * dst_char = (unsigned char*)dst;
    unsigned char cvalue = (unsigned char)value;
    for (int i = 0; i < len; i++){
        dst_char[i] = value;
    }
    return dst;
}

#define IS_ALIGNED(ptr) (((unsigned long long)ptr & 15) == 0)

// Unrolled memset 
static void * memset_unrolled(void * dst, EFI_UINT32 value, unsigned long long count)
{
    EFI_UINT32 * dst32 = (EFI_UINT32 *)dst;
    while (!IS_ALIGNED(dst32) && count > 0) //handle unaligned start
    {
        *dst32 = value;
        dst32 += 1;
        count--;
    }
    while (count >= 16) //handle aligned blocks of 16 uints
    {
        dst32[0] = value;
        dst32[1] = value;
        dst32[2] = value;
        dst32[3] = value;
        dst32[4] = value;
        dst32[5] = value;
        dst32[6] = value;
        dst32[7] = value;
        dst32[8] = value;
        dst32[9] = value;
        dst32[10] = value;
        dst32[11] = value;
        dst32[12] = value;
        dst32[13] = value;
        dst32[14] = value;
        dst32[15] = value;

        dst32 += 16;
        count -= 16;
    }
    while (count > 0) //handle remaining bytes
    {
        *dst32 = value;
        dst32 = &dst32[1];        
        count--;
    }
    return dst;
}

// vectorized memset 
static void * memset_vector(void * dst, EFI_UINT32 value, unsigned long long count)
{
    EFI_UINT32 * dst32 = (EFI_UINT32 *)dst;
    while (!IS_ALIGNED(dst32) && count-- > 0) //handle unaligned start
    {
        *dst32++ = value;
    }
    unsigned long long blocksize = 16; // 16 integers = 512 bits
    unsigned long long numBlocks = count / blocksize; //number of 16-integer (512 bit) blocks
    if (numBlocks) {
        __asm__ __volatile__ (
            "movd      %[val], %%xmm0\n\t"              // Load the value into xmm0
            "pshufd    $0, %%xmm0, %%xmm0\n\t"          // Broadcast the value to all elements of xmm0
            "1:\n\t"                                    // Loop label
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "decq      %[n]\n\t"                        // We have now processed one block of 16 ints, decrement the block count
            "jnz       1b\n\t"                          // If there are more blocks, jump back to the loop label
            : [dst] "+r"(dst32), [n] "+r"(numBlocks)    // Output: These operands are modified by the assembly code
            : [val] "r"(value)                          // Input: These operands are read by the assembly code
            : "cc", "memory", "xmm0"                    // Clobbered registers: These registers are modified by the assembly code
                                                        // "cc" indicates that the condition codes may be modified 
        );
        count %= blocksize; //update count to remaining bytes after processing blocks
    }
    while (count-- > 0) //handle remaining bytes
    {
        *dst32++ = value;
    }
    return dst;
}

// vectorized memset 
static void * memset_vector_array(void * dst, EFI_UINT32 value[4], unsigned long long count)
{
    EFI_UINT32 * dst32 = (EFI_UINT32 *)dst;
    while (!IS_ALIGNED(dst32) && count-- > 0) //handle unaligned start
    {
        *dst32++ = value[0];
    }
    unsigned long long blocksize = 16; // 16 integers = 512 bits
    unsigned long long numBlocks = count / blocksize; //number of 16-integer (512 bit) blocks
    if (numBlocks) {
        __asm__ __volatile__ (
            "movdqu    %[val], %%xmm0\n\t"              // Load the values into xmm0
            "1:\n\t"                                    // Loop label
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "movdqa    %%xmm0, (%[dst])\n\t"            // -- Store the value in xmm0 to the dst32
            "addq      $16, %[dst]\n\t"                 // -- Move the destination pointer forward by 16 bytes
            "decq      %[n]\n\t"                        // We have now processed one block of 16 ints, decrement the block count
            "jnz       1b\n\t"                          // If there are more blocks, jump back to the loop label
            : [dst] "+r"(dst32), [n] "+r"(numBlocks)    // Output: These operands are modified by the assembly code
            : [val] "m"(value)                          // Input: These operands are read by the assembly code
            : "cc", "memory", "xmm0"                    // Clobbered registers: These registers are modified by the assembly code
                                                        // "cc" indicates that the condition codes may be modified 
        );
        count %= blocksize; //update count to remaining bytes after processing blocks
    }
    while (count-- > 0) //handle remaining bytes
    {
        *dst32++ = value[0];
    }
    return dst;
}
#endif