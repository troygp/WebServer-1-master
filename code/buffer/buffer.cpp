/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

size_t Buffer::PrependableBytes() const {
    return readPos_;
}

const char* Buffer::Peek() const {                  //指针， 指向可读的位置
    return BeginPtr_() + readPos_;
}

void Buffer::Retrieve(size_t len) {                 //读取len个长度后，可读的位置pos
    assert(len <= ReadableBytes());
    readPos_ += len;
}

void Buffer::RetrieveUntil(const char* end) {       //读到end指向的位置，
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {            //缓冲区所有字节转换为str，并清空缓冲区
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

void Buffer::Append(const std::string& str) {               //把str指向的字符串添加到buffer_后
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {         //把str指向的字符串添加到buffer_后
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char* str, size_t len) {          //把str指向的字符串添加到buffer_后
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer& buff) {                   //把buff添加到buffer_后
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}


//结合栈上空间，避免内存使用过大，提高内存使用率
//如果有10K个连接，每个连接就分配64K缓冲区的话，将占用640M内存
//而大多数时候，这些缓冲区的使用率很低
//从socket读到缓冲区的方法是使用readv先读至Buffer_，
//Buffer_空间如果不够会读入到栈上65536个字节大小的空间，然后以append的
//方式追加入Buffer_。既考虑了避免系统调用带来开销，又不影响数据的接收。
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {            //从文件描述符中读取数据
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {
        writePos_ += len;
    }
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}

char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {                           //扩展
    if(WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } 
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}