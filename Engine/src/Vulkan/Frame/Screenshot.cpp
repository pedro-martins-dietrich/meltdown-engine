#include <pch.hpp>
#include "Screenshot.hpp"

#include <format>

#include "../Device/GpuBuffer.hpp"
#include "../../Utils/FileHandler.hpp"
#include "../../Utils/Logger.hpp"

namespace mtd::Screenshot
{
    struct Job
    {
        vk::Device device;
        GpuBuffer buffer;
        CommandHandler commandHandler;
        vk::Fence copyFence;
        UIntVec2 dimensions;
    };

    static void transitionImage(vk::CommandBuffer commandBuffer, vk::Image image, bool toCopy);
    static void copyImageToBuffer
    (
        vk::CommandBuffer commandBuffer, GpuBuffer& buffer, vk::Image image, UIntVec2 dimensions
    );
    static void saveScreenshot(std::unique_ptr<Job> job);
}

void mtd::Screenshot::takeScreenshot
(
    const Device& mtdDevice, vk::Image image, UIntVec2 dimensions,
    vk::Semaphore renderFinished, vk::Semaphore screenshotCopy
)
{
    std::unique_ptr<Job> job = std::make_unique<Job>
    (
        mtdDevice.getDevice(),
        GpuBuffer
        {
            mtdDevice,
            dimensions.x * dimensions.y * 4UL,
            vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
        },
        CommandHandler{mtdDevice},
        mtdDevice.getDevice().createFence({}),
        dimensions
    );

    vk::CommandBuffer commandBuffer = job->commandHandler.beginSingleTimeCommand();

    transitionImage(commandBuffer, image, true);
    copyImageToBuffer(commandBuffer, job->buffer, image, dimensions);
    transitionImage(commandBuffer, image, false);

    job->commandHandler.endSingleTimeCommand(commandBuffer, renderFinished, screenshotCopy, job->copyFence);

    std::thread([job = std::move(job)]() mutable
    {
        saveScreenshot(std::move(job));
    }).detach();
}

void mtd::Screenshot::transitionImage(vk::CommandBuffer commandBuffer, vk::Image image, bool toCopy)
{
    vk::ImageSubresourceRange subresource{};
	subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresource.baseMipLevel = 0U;
	subresource.levelCount = 1U;
	subresource.baseArrayLayer = 0U;
	subresource.layerCount = 1U;

    vk::ImageMemoryBarrier barrier{};
    barrier.srcAccessMask = toCopy ? vk::AccessFlagBits::eNone : vk::AccessFlagBits::eTransferRead;
    barrier.dstAccessMask = toCopy ? vk::AccessFlagBits::eTransferRead : vk::AccessFlagBits::eNone;
    barrier.oldLayout = toCopy ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eTransferSrcOptimal;
    barrier.newLayout = toCopy ? vk::ImageLayout::eTransferSrcOptimal : vk::ImageLayout::ePresentSrcKHR;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange = subresource;
    barrier.pNext = nullptr;

    commandBuffer.pipelineBarrier
    (
        toCopy ? vk::PipelineStageFlagBits::eTopOfPipe : vk::PipelineStageFlagBits::eTransfer,
        toCopy ? vk::PipelineStageFlagBits::eTransfer : vk::PipelineStageFlagBits::eTopOfPipe,
        vk::DependencyFlags(), nullptr, nullptr, barrier
    );
}

void mtd::Screenshot::copyImageToBuffer
(
    vk::CommandBuffer commandBuffer, GpuBuffer& buffer, vk::Image image, UIntVec2 dimensions
)
{
    vk::ImageSubresourceLayers subresource{};
    subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresource.mipLevel = 0U;
    subresource.baseArrayLayer = 0U;
    subresource.layerCount = 1U;

    vk::BufferImageCopy region{};
    region.bufferOffset = 0UL;
    region.bufferRowLength = 0U;
    region.bufferImageHeight = 0U;
    region.imageSubresource = subresource;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{dimensions.x, dimensions.y, 1U};

    commandBuffer.copyImageToBuffer(image, vk::ImageLayout::eTransferSrcOptimal, buffer.getBuffer(), region);
}

void mtd::Screenshot::saveScreenshot(std::unique_ptr<Job> job)
{
    vk::DeviceSize imageSize = job->dimensions.x * job->dimensions.y * 4UL;
    std::vector<uint8_t> pixels(imageSize);

    vk::Result result = job->device.waitForFences(job->copyFence, vk::True, UINT64_MAX);
    if(result != vk::Result::eSuccess)
        LOG_ERROR("Failed to wait screenshot copy fence. Vulkan result: %d.");

    job->buffer.copyMemoryFromBuffer(imageSize, pixels.data());

    for(size_t i = 0; i < (4 * job->dimensions.x * job->dimensions.y); i += 4)
    {
        uint8_t blueChannel = pixels[i];
        pixels[i] = pixels[i + 2];
        pixels[i + 2] = blueChannel;
        pixels[i + 3] = 255U;
    }

    using Milliseconds = std::chrono::milliseconds;
    const std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
    const Milliseconds milliseconds = std::chrono::duration_cast<Milliseconds>(currentTime.time_since_epoch());
    std::string path = std::format("./screenshots/{:%Y-%m-%d_%H.%M}.{:%S}.png", currentTime, milliseconds);

    if(FileHandler::saveImage(path, job->dimensions, 4, pixels))
        LOG_INFO("Screenshot saved at \"%s\".", path.c_str());

    job->device.destroyFence(job->copyFence);
}
