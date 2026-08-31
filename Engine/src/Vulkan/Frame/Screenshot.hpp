#pragma once

#include "../Device/Device.hpp"

namespace mtd::Screenshot
{
    void takeScreenshot
    (
        const Device& mtdDevice, vk::Image image, UIntVec2 dimensions,
        vk::Semaphore renderFinished, vk::Semaphore screenshotCopy
    );
}
