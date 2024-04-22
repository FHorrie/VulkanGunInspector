#pragma once
#include "device.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace FH {

    class FHDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(FHDevice& device)
                : m_FHDevice{ device }
            {}

            Builder& AddBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);

            std::unique_ptr<FHDescriptorSetLayout> Build() const;

        private:
            FHDevice& m_FHDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        FHDescriptorSetLayout(
            FHDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~FHDescriptorSetLayout();
        FHDescriptorSetLayout(const FHDescriptorSetLayout&) = delete;
        FHDescriptorSetLayout& operator=(const FHDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

    private:
        FHDevice& m_FHDevice;
        VkDescriptorSetLayout m_DescriptorSetLayout{};
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings{};

        friend class FHDescriptorWriter;
    };

    class FHDescriptorPool {
    public:
        class Builder {
        public:
            Builder(FHDevice& device) : m_FHDevice{ device } {}

            Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t count);
            std::unique_ptr<FHDescriptorPool> Build() const;

        private:
            FHDevice& m_FHDevice;
            std::vector<VkDescriptorPoolSize> m_PoolSizes{};
            uint32_t m_MaxSets = 1000;
            VkDescriptorPoolCreateFlags m_PoolFlags = 0;
        };

        FHDescriptorPool(
            FHDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~FHDescriptorPool();
        FHDescriptorPool(const FHDescriptorPool&) = delete;
        FHDescriptorPool& operator=(const FHDescriptorPool&) = delete;

        bool AllocateDescriptorSet(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void ResetPool();

    private:
        FHDevice& m_FHDevice;
        VkDescriptorPool m_DescriptorPool{};

        friend class FHDescriptorWriter;
    };

    class FHDescriptorWriter {
    public:
        FHDescriptorWriter(FHDescriptorSetLayout& setLayout, FHDescriptorPool& pool);

        FHDescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        FHDescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool Build(VkDescriptorSet& set);
        void Overwrite(VkDescriptorSet& set);

    private:
        FHDescriptorSetLayout& m_SetLayout;
        FHDescriptorPool& m_Pool;
        std::vector<VkWriteDescriptorSet> m_Writes{};
    };
}