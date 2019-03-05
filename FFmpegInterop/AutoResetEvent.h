#include <mutex>
#include <condition_variable>
#include <thread>
#include <stdio.h>

class AutoResetEvent
{
public:
	explicit AutoResetEvent(bool initial = false)
		: flag_(initial)
	{
	}

	void Set()
	{
		std::lock_guard<std::mutex> _(protect_);
		flag_ = true;
		signal_.notify_one();
	}

	void Reset()
	{
		std::lock_guard<std::mutex> _(protect_);
		flag_ = false;
	}

	bool WaitOne()
	{
		std::unique_lock<std::mutex> lk(protect_);
		while (!flag_) // prevent spurious wakeups from doing harm
			signal_.wait(lk);
		flag_ = false; // waiting resets the flag
		return true;
	}

private:
	AutoResetEvent(const AutoResetEvent&);
	AutoResetEvent& operator=(const AutoResetEvent&); // non-copyable
	bool flag_;
	std::mutex protect_;
	std::condition_variable signal_;
};

