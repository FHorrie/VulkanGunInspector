#include "buffer.h"

#include <cassert>
#include <cstring>


FH::FHBuffer::FHBuffer(
    FHDevice& device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment)
    : m_FHDevice{ device }
    , m_InstanceSize{ instanceSize }
    , m_InstanceCount{ instanceCount }
    , m_UsageFlags{ usageFlags }
    , m_MemoryPropertyFlags{ memoryPropertyFlags } 
{
    m_AlignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
    m_BufferSize = m_AlignmentSize * instanceCount;
    device.CreateBuffer(m_BufferSize, usageFlags, memoryPropertyFlags, m_Buffer, m_Memory);
}

FH::FHBuffer::~FHBuffer() 
{
    UnMap();
    vkDestroyBuffer(m_FHDevice.GetDevice(), m_Buffer, nullptr);
    vkFreeMemory(m_FHDevice.GetDevice(), m_Memory, nullptr);
}

// Returns the minimum instance size required to be compatible with devices minOffsetAlignment (STATIC)
VkDeviceSize FH::FHBuffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) 
{
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

// Maps a memory range
VkResult FH::FHBuffer::Map(VkDeviceSize size, VkDeviceSize offset) 
{
    assert(m_Buffer && m_Memory && "Called map on buffer before create");
    return vkMapMemory(m_FHDevice.GetDevice(), m_Memory, offset, size, 0, &m_Mapped);
}


// Unmaps a memory range
void FH::FHBuffer::UnMap() 
{
    if (m_Mapped) {
        vkUnmapMemory(m_FHDevice.GetDevice(), m_Memory);
        m_Mapped = nullptr;
    }
}


// Copies the specified data to the mapped buffer. Default value writes whole buffer range
void FH::FHBuffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) 
{
    assert(m_Mapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE) 
        memcpy(m_Mapped, data, m_BufferSize);
    else 
    {
        char* memOffset = static_cast<char*>(m_Mapped);
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

// Flush a memory range of the buffer to make it visible to the device
VkResult FH::FHBuffer::FlushBufferMemRange(VkDeviceSize size, VkDeviceSize offset) 
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(m_FHDevice.GetDevice(), 1, &mappedRange);
}


// Invalidate a memory range of the buffer to make it visible to the host
VkResult FH::FHBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) 
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(m_FHDevice.GetDevice(), 1, &mappedRange);
}


// Create a buffer info descriptor
VkDescriptorBufferInfo FH::FHBuffer::GetDescriptorInfo(VkDeviceSize size, VkDeviceSize offset) 
{
    return VkDescriptorBufferInfo
    {
        m_Buffer,
        offset,
        size
    };
}


// Copies "m_InstanceSize" bytes of data to the mapped buffer at an offset of index * m_AlignmentSize
void FH::FHBuffer::WriteToIndex(void* data, int index) 
{
    WriteToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
}


// Flush the memory range at index * m_AlignmentSize of the buffer to make it visible to the device
VkResult FH::FHBuffer::FlushAtIndex(int index) 
{ 
    return FlushBufferMemRange(m_AlignmentSize, index * m_AlignmentSize); 
}

// Create a buffer info descriptor
VkDescriptorBufferInfo FH::FHBuffer::GetDescriptorInfoForIndex(int index) 
{
    return GetDescriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
}

// Invalidate a memory range of the buffer to make it visible to the host
VkResult FH::FHBuffer::InvalidateIndex(int index) 
{
    return Invalidate(m_AlignmentSize, index * m_AlignmentSize);
}
