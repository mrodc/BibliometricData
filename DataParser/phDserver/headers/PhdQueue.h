//
// Created by mrodc on 2/1/26.
//

#ifndef PHDSERVER_PHDQUEUE_H
#define PHDSERVER_PHDQUEUE_H
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <vector>


    using namespace std;

    template <class T>
    class PhdQueue
    {
    public:
        // TSQueue();
        // virtual ~TSQueue();
        void push(T item)
        {
            m_queue.push(item);
        }
        void push(vector<T> item)
        {
            for (const auto &i : item)
                m_queue.push(i);
        }
        T pop()
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            // retrieve item
            T item = m_queue.front();
            m_queue.pop();

            return item;
        }
        bool empty()
        {
            return m_queue.empty();
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            while (m_queue.size() > 0)
                m_queue.pop();
        }
        int count()
        {
            return m_queue.size();
        }

    protected:
    private:
        std::queue<T> m_queue;

        // mutex for thread synchronization
        std::mutex m_mutex;

        // Condition variable for signaling
        std::condition_variable m_cond;
    };




#endif //PHDSERVER_PHDQUEUE_H