
#include "CThreadPool.h"



// �����߳�
WorkThread::WorkThread() : m_task{ nullptr }, m_bStop{ false }
{
	m_bRunning.store(true);		// ��ζ�Ÿö��󴴽����߳̾�����������
	// ��ʼ��ִ���߳�
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
	// ֪ͨ�̣߳�Ȼ�����߳�ִ��
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
		// �ȴ��������û���������߳�Ҳû���˳��Ļ��̻߳�����
		{
			std::unique_lock<std::mutex> lock(m_mutexTask);
			// �ȴ��ź�
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
		task->run();// ִ������
		delete task;// �ͷ�task�ڴ�
		task = nullptr;
	}
}

std::thread::id WorkThread::getThreadID()
{
	return std::this_thread::get_id();
}

void WorkThread::stop()
{
	m_bRunning.store(false);//����ֹͣ��־λ
	m_mutexThread.lock();
	if (m_thread.joinable())
	{
		// ����һ��Ҫ�����ñ�־λ��Ȼ��֪ͨ�̣߳�����
		// ��Ϊ��ʱ����ʱ�߳��������ģ�������Ҫ֪ͨ��Ȼ��join
		// �����߳̿��ܲ����˳�����һֱ����
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
	// ����ʹ��m_task���ж��Ƿ���ִ������������ʹ������ִ��
	// ����m_task�Ѿ�������λnullptr�ˣ������ٴ�����������
	// �����ٴ��������Ļ����Ͳ���������ˡ�����Ҳ���ᶪʧ����
	bool ret;
	m_mutexTask.lock();
	ret = (nullptr == m_task);
	m_mutexTask.unlock();
	return !ret;
}

LeisureThreadList::LeisureThreadList(const size_t counts)
{
	assign(counts);// ��������߳�
}

LeisureThreadList::~LeisureThreadList()
{
	// ɾ���߳��б��е������߳�
	// �������������������ü���
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
		// �����߳�
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

// �̳߳�
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
		// ��ִͣ�У�������

		{
			std::unique_lock<std::mutex> lockRunning(m_runningMutex);
			m_condition_running.wait(lockRunning,
				[this]() {return this->m_bRunning.load(); });
		}
		WorkThread* thread = nullptr;
		Task* task = nullptr;

		{
			std::unique_lock<std::mutex> lock(m_taskMutex);
			// ���û��������û�н���������
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
		// ѡ����е��߳�ִ������
		do {
			thread = m_leisureThreadList.top();
			m_leisureThreadList.pop();
			m_leisureThreadList.push(thread);
		} while (thread->isExecuting());
		// ֪ͨ�߳�ִ��
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
	// ֪ͨ��صȴ��߳�
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
	// ������������ñ�־λ���ұ���notify����Ϊ�߳�
	// ��ʱ��������ñ�־λ�󻹴�������״̬
	m_condition_task.notify_all();
	// �����߳�
	m_mutexThread.lock();
	if (m_thread.joinable())
	{
		m_thread.join();
	}
	m_mutexThread.unlock();// �����߳�
	// �����߳�ͬ��ֹͣ����Ȼ����ִ���߳��˳�ǰ���߳��е�����ִ�����
	m_leisureThreadList.stop();
	m_bExit = true;
}
