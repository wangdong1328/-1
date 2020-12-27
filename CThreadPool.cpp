
#include "CThreadPool.h"



// 工作线程
WorkThread::WorkThread() : m_task{ nullptr }, m_bStop{ false }
{
	m_bRunning.store(true);		// 意味着该对象创建后线程就运行起来了
	// 初始化执行线程
	m_thread = std::thread(&WorkThread::run, this);
}

WorkThread::~WorkThread()
{
	if (!m_bStop)
		stop();
	if (m_thread.joinable())
		m_thread.join();
}

bool WorkThread::assign(Task* task)
{
	m_mutexTask.lock();
	if (nullptr != m_task)
	{
		m_mutexTask.unlock();
		return false;
	}
	m_task = task;
	m_mutexTask.unlock();
	// 通知线程，然后唤醒线程执行
	m_condition.notify_one();
	return true;
}

void WorkThread::run()
{
	while (true)
	{
		if (!m_bRunning.load())
		{
			m_mutexTask.lock();
			if (nullptr == m_task)
			{
				m_mutexTask.unlock();
				break;
			}
			m_mutexTask.unlock();
		}
		Task* task = nullptr;
		// 等待任务，如果没有任务并且线程也没有退出的话线程会阻塞
		{
			std::unique_lock<std::mutex> lock(m_mutexTask);
			// 等待信号
			m_condition.wait(lock,
				[this]() {
					return !((nullptr == m_task) && this->m_bRunning.load());
				});
			task = m_task;
			m_task = nullptr;
			if (nullptr == task)
			{
				continue;
			}
		}
		task->run();// 执行任务
		delete task;// 释放task内存
		task = nullptr;
	}
}

std::thread::id WorkThread::getThreadID()
{
	return std::this_thread::get_id();
}

void WorkThread::stop()
{
	m_bRunning.store(false);//设置停止标志位
	m_mutexThread.lock();
	if (m_thread.joinable())
	{
		// 这里一定要先设置标志位后，然后通知线程，唤醒
		// 因为有时候这时线程是阻塞的，所以需要通知，然后join
		// 否则线程可能不会退出，会一直阻塞
		m_condition.notify_all();
		m_thread.join();
	}
	m_mutexThread.unlock();
	m_bStop = true;
}

void WorkThread::notify()
{
	m_mutexCondition.lock();
	m_condition.notify_one();
	m_mutexCondition.unlock();
}

void WorkThread::notify_all()
{
	m_mutexCondition.lock();
	m_condition.notify_all();
	m_mutexCondition.unlock();
}

bool WorkThread::isExecuting()
{
	// 这里使用m_task来判断是否在执行任务，这样即使任务在执行
	// 但是m_task已经被设置位nullptr了，可以再次添加任务。如果
	// 后面再次添加任务的话，就不能再添加了。这样也不会丢失任务。
	bool ret;
	m_mutexTask.lock();
	ret = (nullptr == m_task);
	m_mutexTask.unlock();
	return !ret;
}

LeisureThreadList::LeisureThreadList(const size_t counts)
{
	assign(counts);// 创建多个线程
}

LeisureThreadList::~LeisureThreadList()
{
	// 删除线程列表中的所有线程
	// 这里是析构函数，不用加锁
	while (!m_threadList.empty())
	{
		WorkThread* temp = m_threadList.front();
		m_threadList.pop_front();
		delete temp;
	}
}

void LeisureThreadList::assign(const size_t counts)
{
	for (size_t i = 0u; i < counts; i++)
	{
		// 创建线程
		m_threadList.push_back(new WorkThread());
	}
}

void LeisureThreadList::push(WorkThread* thread)
{
	if (nullptr == thread)
		return;
	m_mutexThread.lock();
	m_threadList.push_back(thread);
	m_mutexThread.unlock();
}


WorkThread* LeisureThreadList::top()
{
	WorkThread* thread;
	m_mutexThread.lock();
	if (m_threadList.empty())
		thread = nullptr;
	thread = m_threadList.front();
	m_mutexThread.unlock();
	return thread;
}

void LeisureThreadList::pop()
{
	m_mutexThread.lock();
	if (!m_threadList.empty())
		m_threadList.pop_front();
	m_mutexThread.unlock();
}

size_t LeisureThreadList::size()
{
	size_t counts = 0u;
	m_mutexThread.lock();
	counts = m_threadList.size();
	m_mutexThread.unlock();
	return counts;
}

void LeisureThreadList::stop()
{
	m_mutexThread.lock();
	for (auto thread : m_threadList)
	{
		thread->stop();
	}
	m_mutexThread.unlock();
}

// 线程池
ThreadPool::ThreadPool(const size_t counts) : m_leisureThreadList{ counts }, m_bExit{ false }
{
	m_threadCounts = counts;
	m_bRunning.store(true);
	m_bEnd.store(true);
	m_thread = std::thread(&ThreadPool::run, this);
}

ThreadPool::~ThreadPool()
{
	if (!m_bExit)
		exit();
}

size_t ThreadPool::threadCounts()
{
	return m_threadCounts.load();
}

bool ThreadPool::isRunning()
{
	return m_bRunning.load();
}

void ThreadPool::run()
{
	while (true)
	{
		if (!m_bEnd.load())
		{
			m_taskMutex.lock();
			if (m_taskList.empty())
			{
				m_taskMutex.unlock();
				break;
			}
			m_taskMutex.unlock();
		}
		// 暂停执行，则阻塞

		{
			std::unique_lock<std::mutex> lockRunning(m_runningMutex);
			m_condition_running.wait(lockRunning,
				[this]() {return this->m_bRunning.load(); });
		}
		WorkThread* thread = nullptr;
		Task* task = nullptr;

		{
			std::unique_lock<std::mutex> lock(m_taskMutex);
			// 如果没有任务且没有结束则阻塞
			m_condition_task.wait(lock,
				[this]() {
					return !(this->m_taskList.empty() && this->m_bEnd.load());
				});
			if (!m_taskList.empty())
			{
				task = m_taskList.front();
				m_taskList.pop();
			}
		}
		// 选择空闲的线程执行任务
		do {
			thread = m_leisureThreadList.top();
			m_leisureThreadList.pop();
			m_leisureThreadList.push(thread);
		} while (thread->isExecuting());
		// 通知线程执行
		thread->assign(task);
		thread->notify();
	}
}

void ThreadPool::addTask(Task* task)
{
	if (nullptr == task)
		return;
	m_taskMutex.lock();
	m_taskList.push(task);
	m_taskMutex.unlock();
	// 通知相关等待线程
	m_condition_task.notify_one();
}

void ThreadPool::start()
{
	m_bRunning.store(true);
	m_condition_running.notify_one();
}

void ThreadPool::stop()
{
	m_bRunning.store(false);
}

void ThreadPool::exit()
{
	m_bEnd.store(false);
	// 这里必须在设置标志位后，且必须notify，因为线程
	// 有时候会在设置标志位后还处于阻塞状态
	m_condition_task.notify_all();
	// 锁定线程
	m_mutexThread.lock();
	if (m_thread.joinable())
	{
		m_thread.join();
	}
	m_mutexThread.unlock();// 解锁线程
	// 任务线程同步停止，当然会在执行线程退出前把线程中的任务执行完成
	m_leisureThreadList.stop();
	m_bExit = true;
}
