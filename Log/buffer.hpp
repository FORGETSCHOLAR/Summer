#pragma once
#include <iostream>
#include <vector>
#include <cassert>

namespace nhylog
{

#define DEFAULT_BUFFER_SIZE 1*1024*1024
#define INCREMENT_BUFFER_SIZE 1*1024*1024
#define THRESHOLD_BUFFER_SIZE 5*1024*1024

class Buffer
{
public:
    Buffer() : buffer_(DEFAULT_BUFFER_SIZE),reader_idx_(0),writer_idx_(0){}

    void push(const char* data,size_t len)
    {
        ensureEnough(len);
        std::copy(data,data+len,&buffer_[writer_idx_]);
        moveWriter(len);
    }

    const char* begin(){
        return &buffer_[reader_idx_];
    }

    void swap(Buffer& buffer){
        buffer_ .swap(buffer.buffer_);
        std::swap(writer_idx_,buffer.writer_idx_);
        std::swap(reader_idx_,buffer.reader_idx_);
    }

    void reset(){
        writer_idx_ = 0;
        reader_idx_ = 0;
    }

    bool empty(){
        return writer_idx_ == reader_idx_;
    }

    size_t writeAbleSize(){
        return buffer_.size() - writer_idx_;
    }
    
    size_t readAbleSize(){
        return writer_idx_ - reader_idx_;
    }
private:
    void ensureEnough(size_t len)
    {
        if(len < writeAbleSize()) return;
        size_t new_size = 0;
        if(buffer_.size() < THRESHOLD_BUFFER_SIZE){
            new_size = buffer_.size()*2+len;
        }else {
            new_size = buffer_.size() + INCREMENT_BUFFER_SIZE + len;
        }
        buffer_.resize(new_size);
    }



    void moveWriter(size_t len){
        assert(writer_idx_ + len < buffer_.size());
        writer_idx_ += len;
    }

    void moveReader(size_t len){
        assert(len <= readAbleSize());
        reader_idx_ += len;
    }
private:
    std::vector<char> buffer_;
    size_t reader_idx_;
    size_t writer_idx_;
};// class Buffer end

}// namespace nhylog end