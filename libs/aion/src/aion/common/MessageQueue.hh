#pragma once

#include <cassert>
#include <queue>
#include <mutex>

namespace aion
{
/**
 * @brief A generic message queue
 *
 * Messages can be added by multiple writers
 * Messages are read by a single reader (they can also be read by multiple readers)
 *
 * All public functions are thread safe
 *
 * Usage: e.g. MessageQueue<int> q;
 *
 * Sender: q.send(10);
 *
 * Receiver-Loop:
 *      int msg;
 *      while (q.receive(msg))
 *      {
 *          // Do something with msg
 *      }
 *
 * Can also be used as priority queue:
 * MessageQueue<int, std::priority_queue<int> > q;
 */
template <class T, class QueueT = std::queue<T>>
class MessageQueue
{
private:
    /// The queue
    QueueT mQueue;

    /// message mutex
    std::mutex mMutex;

    // abstracting queue::front and priority_queue::top
    static T front(std::queue<T> &q) { return std::move(q.front()); }
    static T front(std::priority_queue<T> &q) { return std::move(q.top()); }
public:
    /// ctor
    MessageQueue() {}
    MessageQueue(MessageQueue const &q) : mQueue(q.mQueue) {}
    /// Sends a msg
    void send(const T &_msg)
    {
        std::lock_guard<std::mutex> _lock(mMutex);

        mQueue.push(_msg);
    }

    /// Sends a msg
    void send(T &&_msg)
    {
        std::lock_guard<std::mutex> _lock(mMutex);

        mQueue.push(std::move(_msg));
    }

    /// Gets the size of the msg queue
    std::size_t size() const { return mQueue.size(); }
    /// Checks if messages are available (does not lock)
    bool hasMessage() const { return mQueue.size() > 0; }
    /// Receives messages (leaves it unchanged if nothing is available)
    bool receive(T &_msg)
    {
        if (mQueue.size() == 0)
            return false;

        std::lock_guard<std::mutex> _lock(mMutex);

        if (mQueue.size() == 0)
            return false;

        _msg = front(mQueue);
        mQueue.pop();

        return true;
    }

    /**
     * @brief clears this queue
     */
    void clear()
    {
        std::lock_guard<std::mutex> _lock(mMutex);
        while (!mQueue.empty())
            mQueue.pop();
    }

    /**
     * @brief clears this queue and returns a copy of all elements
     */
    std::vector<T> copyAndClear()
    {
        std::lock_guard<std::mutex> _lock(mMutex);
        std::vector<T> v;
        while (!mQueue.empty())
        {
            v.push_back(front(mQueue));
            mQueue.pop();
        }
        return v;
    }
};
}
