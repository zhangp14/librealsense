// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

// Simplest implementation of a blocking concurrent queue for thread messaging
template<class T>
class single_consumer_queue
{
	std::queue<T> q;
	std::mutex mutex;
	std::condition_variable cv;

public:
	single_consumer_queue<T>() : q(), mutex(), cv() {}

	void enqueue(T item)
	{
		std::unique_lock<std::mutex> lock(mutex);
		q.push(std::move(item));
		lock.unlock();
		cv.notify_one(); 
	}

	T dequeue()
	{
		std::unique_lock<std::mutex> lock(mutex);
		const auto ready = [this]() { return !q.empty(); };
		if (!ready() && !cv.wait_for(lock, std::chrono::seconds(5), ready)) throw std::runtime_error("Timeout waiting for queued items!");
		auto item = std::move(q.front());
		q.pop();
		return std::move(item);
	}

	int size()
	{
		std::unique_lock<std::mutex> lock(mutex); 
		return q.size();
	}
};