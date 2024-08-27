#pragma once
#include <atomic>
#include <condition_variable>
#include <thread>
#include <functional>

#include "buffer.hpp"

namespace nhylog
{

using Functor = std::function<void(Buffer&)>;

enum class AsyncType
{
    ASYNC_SAFE,ASYNC_UNSAFE
};

class AsyncLooper
{
public:
    using ptr = std::shared_ptr<AsyncLooper>;
    AsyncLooper(
        const Functor& cb,
        AsyncType type = AsyncType::ASYNC_SAFE
    ):
        stop_(false),
        callBack_(cb),
        looper_type_(type),
        thread_(std::thread(&AsyncLooper::threadEntry,this))
    {
    }

    ~AsyncLooper(){
        stop();
    }

    void push(const char* data, size_t len){
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(looper_type_ == AsyncType::ASYNC_SAFE){
                pro_cond_.wait(lock,[&](){return pro_buffer_.writeAbleSize() >= len;});
            }
            pro_buffer_.push(data,len);
        }
        con_cond_.notify_all();
    }

private:
    void stop(){
        stop_ = true;
        con_cond_.notify_all();
        thread_.join();
    }

    void threadEntry(){
        while(true){
            {
                std::unique_lock<std::mutex> lock(mutex_);
                con_cond_.wait(lock,[&](){return stop_ || !pro_buffer_.empty();});
                if(stop_ && pro_buffer_.empty()) break;
                con_buffer_.swap(pro_buffer_);
            }
            pro_cond_.notify_all();
            callBack_(con_buffer_);
            con_buffer_.reset();
        }
    }
private:
    Functor callBack_;
    std::atomic<bool> stop_;
    Buffer pro_buffer_;
    Buffer con_buffer_;
    std::mutex mutex_;
    std::condition_variable pro_cond_;
    std::condition_variable con_cond_;
    std::thread thread_;
    AsyncType looper_type_;
};//class AsyncLooper end


}//namespace nhylog end