#pragma once

// 头文件
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
* @Notes :      线程池的实现
*
*/ 
class ThreadPool;

// 任务类，使用的时候继承该类，重写run函数就可以了。
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
	std::thread m_thread;		// 工作线程
	Task* m_task;				// 任务指针
	std::mutex m_mutexThread;	// 关于任务是否执行的互斥量
	std::mutex m_mutexCondition;// 条件互斥量
	std::mutex m_mutexTask;		// 关于分配任务的互斥量
	std::condition_variable m_condition;	// 任务条件变量
	std::atomic<bool> m_bRunning;			// 是否运行的标志位
	bool m_bStop;
protected:
	virtual void run();				// 线程函数
public:
	WorkThread();
	~WorkThread();
	WorkThread(const WorkThread& thread) = delete;
	WorkThread(const WorkThread&& thread) = delete;
	WorkThread& operator=(const WorkThread& thread) = delete;
	WorkThread& operator=(const WorkThread&& thread) = delete;

	bool assign(Task* task);		// 分配任务
	std::thread::id getThreadID();	// 获取线程ID
	void stop();					// 停止线程运行，其实就是结束线程运行			
	void notify();					// 通知阻塞线程
	void notify_all();				// 通知所有的阻塞线程
	bool isExecuting();				// 是否在执行任务
};

// 空闲线程列表
// 该类也是线程安全的，所以在使用的过程中没毕业给他加锁
class LeisureThreadList
{
	std::list<WorkThread*> m_threadList;	// 线程列表
	std::mutex m_mutexThread;				// 线程列表访问互斥量
	void assign(const size_t counts);		// 创建线程
public:
	LeisureThreadList(const size_t counts);
	~LeisureThreadList();
	void push(WorkThread* thread);			// 添加线程
	WorkThread* top();						// 返回第一个线程指针
	void pop();								// 删除第一个线程
	size_t size();							// 返回线程个数
	void stop();							// 停止运行
};

// 线程池
// 该类是线程安全的
/*
该类运行的时候只要调用addTask()添加任务就可以了，任务会自动运行。
如果要线程池退出时必须调用一下exit()函数。
线程池可以暂定执行，只需调用stop()就可以了，要重新开始运行则需调用start()就可以了。
*/
class ThreadPool
{
	std::thread m_thread;	// 线程池的任务分配线程
	LeisureThreadList m_leisureThreadList;				// 线程列表
	std::queue<Task*, std::list<Task*>> m_taskList;	// 任务队列
	std::atomic<bool> m_bRunning;				// 是否运行
	std::atomic<bool> m_bEnd;					// 是否结束运行
	std::atomic<size_t> m_threadCounts;			// 线程总数
	std::condition_variable m_condition_task;	// 任务条件
	std::condition_variable m_condition_thread;	// 线程列表条件
	std::condition_variable m_condition_running;// 运行条件变量
	std::mutex m_runningMutex;					// 运行控制变量
	std::mutex m_mutexThread;					// 空闲互斥量
	std::mutex m_taskMutex;						// 访问任务列表互斥量
	bool m_bExit;								// 是否退出标志位

	void run();					// 线程池主线程函数
public:
	ThreadPool(const size_t counts);
	~ThreadPool();
	size_t threadCounts();
	bool isRunning();			// 线程是否正在运行任务
	void addTask(Task* task);	// 添加任务
	// 线程池开始调度任务，线程池创建后不用调用该函数。该函数需要和stop()
	// 配合使用。
	void start();				// 线程池开始执行
	// 线程池暂停任务调度，但是不影响任务添加。想要开始任务调度，则调用
	// start()函数
	void stop();				// 线程池暂停运行
	// 该函数会在线程池中的所有任务都分派出去后结束线程池的线程运行，
	// 同样的线程列表中的线程会在自己的任务执行完后，然后再退出。
	void exit();				// 线程池退出
};


