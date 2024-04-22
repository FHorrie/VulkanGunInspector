#include "descriptors.h"

#include <cassert>
#include <stdexcept>

//Descriptor Set Layout Builder
FH::FHDescriptorSetLayout::Builder& FH::FHDescriptorSetLayout::Builder::AddBinding(
    uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) 
{
    assert(bindings.count(binding) == 0 && "Binding already in use");

    VkDescriptorSetLayoutBinding layoutBinding
    {
        .binding = binding,
        .descriptorType = descriptorType,
        .descriptorCount = count,
        .stageFlags = stageFlags
    };
    bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<FH::FHDescriptorSetLayout> FH::FHDescriptorSetLayout::Builder::Build() const
{
    return std::make_unique<FHDescriptorSetLayout>(m_FHDevice, bindings);
}

//Descriptor Set Layout
FH::FHDescriptorSetLayout::FHDescriptorSetLayout(
    FHDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : m_FHDevice{ device }
    , m_Bindings{ bindings } 
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto& kv : bindings)
    {
        setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(
        m_FHDevice.GetDevice(),
        &descriptorSetLayoutInfo,
        nullptr,
        &m_DescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

FH::FHDescriptorSetLayout::~FHDescriptorSetLayout() 
{
    vkDestroyDescriptorSetLayout(m_FHDevice.GetDevice(), m_DescriptorSetLayout, nullptr);
}

//Descriptor Pool Builder
FH::FHDescriptorPool::Builder& FH::FHDescriptorPool::Builder::AddPoolSize(
    VkDescriptorType descriptorType, uint32_t count) 
{
    m_PoolSizes.push_back({ descriptorType, count });
    return *this;
}

FH::FHDescriptorPool::Builder& FH::FHDescriptorPool::Builder::SetPoolFlags(
    VkDescriptorPoolCreateFlags flags) 
{
    m_PoolFlags = flags;
    return *this;
}
FH::FHDescriptorPool::Builder& FH::FHDescriptorPool::Builder::SetMaxSets(uint32_t count) 
{
    m_MaxSets = count;
    return *this;
}

std::unique_ptr<FH::FHDescriptorPool> FH::FHDescriptorPool::Builder::Build() const 
{
    return std::make_unique<FHDescriptorPool>(m_FHDevice, m_MaxSets, m_PoolFlags, m_PoolSizes);
}

//Descriptor Pool
FH::FHDescriptorPool::FHDescriptorPool(FHDevice& device, uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
    : m_FHDevice{ device }
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(m_FHDevice.GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) 
        throw std::runtime_error("failed to create descriptor pool!");
}

FH::FHDescriptorPool::~FHDescriptorPool() 
{
    vkDestroyDescriptorPool(m_FHDevice.GetDevice(), m_DescriptorPool, nullptr);
}

bool FH::FHDescriptorPool::AllocateDescriptorSet(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const 
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    if (vkAllocateDescriptorSets(m_FHDevice.GetDevice(), &allocInfo, &descriptor) != VK_SUCCESS)
        return false;
    return true;
}

void FH::FHDescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const 
{
    vkFreeDescriptorSets(
        m_FHDevice.GetDevice(),
        m_DescriptorPool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
}

void FH::FHDescriptorPool::ResetPool() 
{
    vkResetDescriptorPool(m_FHDevice.GetDevice(), m_DescriptorPool, 0);
}


FH::FHDescriptorWriter::FHDescriptorWriter(FHDescriptorSetLayout& setLayout, FHDescriptorPool& pool)
    : m_SetLayout{ setLayout }
    , m_Pool{ pool } 
{}

FH::FHDescriptorWriter& FH::FHDescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
    assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto& bindingDescription = m_SetLayout.m_Bindings[binding];

    assert(bindingDescription.descriptorCount == 1 &&
        "Binding expects multiple descriptor info var");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    m_Writes.push_back(write);
    return *this;
}

FH::FHDescriptorWriter& FH::FHDescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) 
{
    assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto& bindingDescription = m_SetLayout.m_Bindings[binding];

    assert(bindingDescription.descriptorCount == 1 &&
        "Binding expects multiple descriptor info var");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    m_Writes.push_back(write);
    return *this;
}

bool FH::FHDescriptorWriter::Build(VkDescriptorSet& set) 
{
    bool success = m_Pool.AllocateDescriptorSet(m_SetLayout.GetDescriptorSetLayout(), set);
    if (!success)
        return false;

    Overwrite(set);
    return true;
}

void FH::FHDescriptorWriter::Overwrite(VkDescriptorSet& set) 
{
    for (auto& write : m_Writes)
        write.dstSet = set;

    vkUpdateDescriptorSets(m_Pool.m_FHDevice.GetDevice(), m_Writes.size(), m_Writes.data(), 0, nullptr);
}