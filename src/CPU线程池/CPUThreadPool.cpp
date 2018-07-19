//
// Created by chijinxin on 17-7-18.
//
#include <iostream>
#include <functional>
#include <atomic>
#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <algorithm>
#include <condition_variable>

using namespace std;

class TaskExecutor{
    using Task = std::function<void()>;   //定义task仿函数

private:
    //线程池
    std::vector<std::thread> pool_;
    //任务队列
    std::queue<Task> tasks_;
    //是否关闭提交
    std::atomic_bool stop_{false};
    //线程池大小
    size_t  size_;
    //同步
    std::mutex mu_;
    //条件变量
    std::condition_variable cv_task_;
    //空闲线程数量
    std::atomic_int idlThreadNum_;

public:
    //构造函数
    TaskExecutor(size_t s = 0):size_(s),stop_(false)
    {
        if(size_ == 0)
        {
            size_ = std::thread::hardware_concurrency();   //默认线程池大小等于CPU核数
        }
        idlThreadNum_ = size_;

        //初始化工作线程数量
        for(int i=0; i<size_; i++)
        {
            pool_.emplace_back(
                    //工作线程函数（死循环，pull任务去执行，stop_为真线程停止）
                    [this]()
                    {
                        while(!this->stop_)
                        {
                            if(this->stop_)
                            {
                                //cout<<"线程 id="<<std::this_thread::get_id()<<" 退出！"<<endl;
                                break;
                            }
                            Task task = this->get_one_task();   //从任务队列中获取一个待执行的任务
                            --this->idlThreadNum_;
                            task();
                            ++this->idlThreadNum_;
                        }
                        cout<<"线程 id="<<std::this_thread::get_id()<<" 退出！"<<endl;
                    });
        }
    }
    //析构函数
    ~TaskExecutor()
    {
        stop_.store(true);
        cv_task_.notify_all();   //唤醒所有的线程去执行
        for(std::thread & t : pool_)
        {
            //t.detach();  //分离线程，让线程自生自灭
            if(t.joinable())
            {
                t.join();   //等待任务结束，前提是线程一定会结束
            }
        }
    }

    //停止任务的提交
    void shutdown()
    {
        this->stop_.store(true);
    }
    //重启任务提交
    void restart()
    {
        this->stop_.store(false);
    }
    //提交一个任务
    template <class F, class ... Args>
    auto commit(F&& f, Args&& ...args)->std::future<decltype(f(args...))>
    {
        if(stop_.load())
        {
            throw std::runtime_error("task executor closed commit!!!");
        }

        using ResType = decltype(f(args...));  //typename std::result_of<F(Args...)>::type  函数ｆ的返回值类型

        //利用std::packaged_task包装任务，与std::future配合从而可以异步的获取任务的执行结果
        auto task = std::make_shared< std::packaged_task<ResType()> >
                (
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                );
        std::future<ResType> future = task->get_future();   //获取任务异步执行凭证
        {
            //添加任务到任务队列，需要加锁
            std::lock_guard<std::mutex> lock_append(mu_);
            tasks_.emplace(
                    [task]()
                    {
                        (*task)();
                    });
        }
        //条件变量唤醒一个线程去执行任务
        cv_task_.notify_one();
        //返回异步执行结果
        return future;
    }

    //获取空闲线程数量
    int getIdlCount()
    {
        return idlThreadNum_;
    }

private:

    //获取一个待执行的task
    Task get_one_task()
    {
        Task task;
        std::unique_lock<std::mutex> lock(this->mu_);
        //条件变量wait()直到有任务队列中有数据
        this->cv_task_.wait(lock,
                            [this]()
                            {
                                return this->stop_.load() || !this->tasks_.empty();
                            });
        //取一个任务
        task = std::move(this->tasks_.front());
        this->tasks_.pop();
        return std::move(task);
    }

};



void task1(int x)
{
    cerr<<"线程id="<<this_thread::get_id()<<"输出："<<x<<endl;
}

int main()
{
    TaskExecutor taskExecutor(0);
    cout<<"空闲线程数量："<<taskExecutor.getIdlCount()<<endl;

    for(int i=0;i<1000000;i++)
    {
        std::future<void> a = taskExecutor.commit(&task1,i);
    }
    while(1)
    {
        this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}