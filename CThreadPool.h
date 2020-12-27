#pragma once

// ͷ�ļ�
#pragma once

#include <list>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

#include <iostream>


/*
* @CreateTime:  2020/12/26 
* @Author:      wangdong
* @ProjectFile: D:\Code\Projects\C++Project\Tools\Tools\Tools.vcxproj
* @Version:     V1.00
* @Notes :      �̳߳ص�ʵ��
*
*/ 
class ThreadPool;

// �����࣬ʹ�õ�ʱ��̳и��࣬��дrun�����Ϳ����ˡ�
class Task
{
private:
	int m_priority;
public:
	enum PRIORITY
	{
		MIN = 1,
		NORMAL = 15,
		MAX = 25
	};
	Task() {};
	Task(PRIORITY priority) : m_priority{ priority } {}
	~Task() {};
	void setPriority(PRIORITY priority)
	{
		m_priority = priority;
	}
	virtual void run() = 0;
};


class WorkThread
{
	std::thread m_thread;		// �����߳�
	Task* m_task;				// ����ָ��
	std::mutex m_mutexThread;	// ���������Ƿ�ִ�еĻ�����
	std::mutex m_mutexCondition;// ����������
	std::mutex m_mutexTask;		// ���ڷ�������Ļ�����
	std::condition_variable m_condition;	// ������������
	std::atomic<bool> m_bRunning;			// �Ƿ����еı�־λ
	bool m_bStop;
protected:
	virtual void run();				// �̺߳���
public:
	WorkThread();
	~WorkThread();
	WorkThread(const WorkThread& thread) = delete;
	WorkThread(const WorkThread&& thread) = delete;
	WorkThread& operator=(const WorkThread& thread) = delete;
	WorkThread& operator=(const WorkThread&& thread) = delete;

	bool assign(Task* task);		// ��������
	std::thread::id getThreadID();	// ��ȡ�߳�ID
	void stop();					// ֹͣ�߳����У���ʵ���ǽ����߳�����			
	void notify();					// ֪ͨ�����߳�
	void notify_all();				// ֪ͨ���е������߳�
	bool isExecuting();				// �Ƿ���ִ������
};

// �����߳��б�
// ����Ҳ���̰߳�ȫ�ģ�������ʹ�õĹ�����û��ҵ��������
class LeisureThreadList
{
	std::list<WorkThread*> m_threadList;	// �߳��б�
	std::mutex m_mutexThread;				// �߳��б���ʻ�����
	void assign(const size_t counts);		// �����߳�
public:
	LeisureThreadList(const size_t counts);
	~LeisureThreadList();
	void push(WorkThread* thread);			// ����߳�
	WorkThread* top();						// ���ص�һ���߳�ָ��
	void pop();								// ɾ����һ���߳�
	size_t size();							// �����̸߳���
	void stop();							// ֹͣ����
};

// �̳߳�
// �������̰߳�ȫ��
/*
�������е�ʱ��ֻҪ����addTask()�������Ϳ����ˣ�������Զ����С�
���Ҫ�̳߳��˳�ʱ�������һ��exit()������
�̳߳ؿ����ݶ�ִ�У�ֻ�����stop()�Ϳ����ˣ�Ҫ���¿�ʼ�����������start()�Ϳ����ˡ�
*/
class ThreadPool
{
	std::thread m_thread;	// �̳߳ص���������߳�
	LeisureThreadList m_leisureThreadList;				// �߳��б�
	std::queue<Task*, std::list<Task*>> m_taskList;	// �������
	std::atomic<bool> m_bRunning;				// �Ƿ�����
	std::atomic<bool> m_bEnd;					// �Ƿ��������
	std::atomic<size_t> m_threadCounts;			// �߳�����
	std::condition_variable m_condition_task;	// ��������
	std::condition_variable m_condition_thread;	// �߳��б�����
	std::condition_variable m_condition_running;// ������������
	std::mutex m_runningMutex;					// ���п��Ʊ���
	std::mutex m_mutexThread;					// ���л�����
	std::mutex m_taskMutex;						// ���������б�����
	bool m_bExit;								// �Ƿ��˳���־λ

	void run();					// �̳߳����̺߳���
public:
	ThreadPool(const size_t counts);
	~ThreadPool();
	size_t threadCounts();
	bool isRunning();			// �߳��Ƿ�������������
	void addTask(Task* task);	// �������
	// �̳߳ؿ�ʼ���������̳߳ش������õ��øú������ú�����Ҫ��stop()
	// ���ʹ�á�
	void start();				// �̳߳ؿ�ʼִ��
	// �̳߳���ͣ������ȣ����ǲ�Ӱ��������ӡ���Ҫ��ʼ������ȣ������
	// start()����
	void stop();				// �̳߳���ͣ����
	// �ú��������̳߳��е��������񶼷��ɳ�ȥ������̳߳ص��߳����У�
	// ͬ�����߳��б��е��̻߳����Լ�������ִ�����Ȼ�����˳���
	void exit();				// �̳߳��˳�
};


