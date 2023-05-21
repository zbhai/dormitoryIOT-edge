#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <pthread.h>
#include <queue>


// 定义任务结构体
using cb = void (*)(void *);
struct Task {
  Task() {
    function = nullptr;
    arg = nullptr;
  }
  Task(cb f, void *arg) {
    function = f;
    this->arg = arg;
  }
  cb function;
  void *arg;
};

// 任务队列
class TaskQueue {
public:
  TaskQueue();
  ~TaskQueue();

  // 添加任务
  void addTask(Task &task);
  void addTask(cb func, void *arg);

  // 取出一个任务
  Task takeTask();

  // 获取当前队列中任务个数
  inline int taskNumber() { return m_queue.size(); }

private:
  pthread_mutex_t m_mutex;  // 互斥锁
  std::queue<Task> m_queue; // 任务队列
};

class ThreadPool {
public:
  ThreadPool(int min, int max);
  ~ThreadPool();

  // 添加任务
  void addTask(Task task);
  // 获取忙线程的个数
  int getBusyNumber();
  // 获取活着的线程个数
  int getAliveNumber();

private:
  // 工作的线程的任务函数
  static void *worker(void *arg);
  // 管理者线程的任务函数
  static void *manager(void *arg);
  void threadExit();

private:
  pthread_mutex_t m_lock;
  pthread_cond_t m_notEmpty;
  pthread_t *m_threadIDs;
  pthread_t m_managerID;
  TaskQueue *m_taskQ;
  int m_minNum;
  int m_maxNum;
  int m_busyNum;
  int m_aliveNum;
  int m_exitNum;
  bool m_shutdown = false;
};

#endif // __THREADPOOL_HPP__