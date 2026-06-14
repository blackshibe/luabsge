#pragma once

#include <deque>
#include <functional>

// A common approach by lots of engines: we add the objects we want to delete
// into a queue, then run that queue to delete all objects in the correct order.
// We store std::function callbacks in a deque and flush it First In Last Out, so
// that objects added last are destroyed first.
//
// Storing whole std::functions per object is inefficient at scale; for the amount
// of objects here it is fine. A faster implementation would store arrays of typed
// vulkan handles (VkImage, VkBuffer, ...) and delete those from a loop.
struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()> &&function);
    void flush();
};