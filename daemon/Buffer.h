/**
 * Copyright (C) ARM Limited 2013-2016. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <semaphore.h>

#include "k/perf_event.h"

class Sender;

enum
{
    FRAME_SUMMARY = 1,
    FRAME_BLOCK_COUNTER = 5,
    FRAME_EXTERNAL = 10,
    FRAME_PERF_ATTRS = 11,
    FRAME_PERF = 12,
};

class Buffer
{
public:
    static const size_t MAXSIZE_PACK32 = 5;
    static const size_t MAXSIZE_PACK64 = 10;

    Buffer(int32_t core, int32_t buftype, const int size, sem_t * const readerSem);
    ~Buffer();

    void write(Sender *sender);

    int bytesAvailable() const;
    int contiguousSpaceAvailable() const;
    bool hasUncommittedMessages() const;
    void commit(const uint64_t time, const bool force = false);
    void check(const uint64_t time);

    // Summary messages
    void summary(const uint64_t currTime, const int64_t timestamp, const int64_t uptime, const int64_t monotonicDelta,
                 const char * const uname, const long pageSize, const bool nosync);
    void coreName(const uint64_t currTime, const int core, const int cpuid, const char * const name);

    // Block Counter messages
    bool eventHeader(uint64_t curr_time);
    bool eventTid(int tid);
    void event(int key, int32_t value);
    void event64(int key, int64_t value);

    // Perf Attrs messages
    void marshalPea(const uint64_t currTime, const struct perf_event_attr * const pea, int key);
    void marshalKeys(const uint64_t currTime, const int count, const __u64 * const ids, const int * const keys);
    void marshalKeysOld(const uint64_t currTime, const int keyCount, const int * const keys, const int bytes,
                        const char * const buf);
    void marshalFormat(const uint64_t currTime, const int length, const char * const format);
    void marshalMaps(const uint64_t currTime, const int pid, const int tid, const char * const maps);
    void marshalComm(const uint64_t currTime, const int pid, const int tid, const char * const image,
                     const char * const comm);
    void onlineCPU(const uint64_t currTime, const int cpu);
    void offlineCPU(const uint64_t currTime, const int cpu);
    void marshalKallsyms(const uint64_t currTime, const char * const kallsyms);
    void perfCounterHeader(const uint64_t time);
    void perfCounter(const int core, const int key, const int64_t value);
    void perfCounterFooter(const uint64_t currTime);
    void marshalHeaderPage(const uint64_t currTime, const char * const headerPage);
    void marshalHeaderEvent(const uint64_t currTime, const char * const headerEvent);

    void setDone();
    bool isDone() const;

    // Prefer a new member to using these functions if possible
    char *getWritePos()
    {
        return mBuf + mWritePos;
    }
    void advanceWrite(int bytes)
    {
        mWritePos = (mWritePos + bytes) & /*mask*/(mSize - 1);
    }
    static void packInt(char * const buf, const int size, int &writePos, int32_t x);
    void packInt(int32_t x);
    static void packInt64(char * const buf, const int size, int &writePos, int64_t x);
    void packInt64(int64_t x);
    void writeBytes(const void * const data, size_t count);
    void writeString(const char * const str);

    static void writeLEInt(unsigned char *buf, uint32_t v)
    {
        buf[0] = (v >> 0) & 0xFF;
        buf[1] = (v >> 8) & 0xFF;
        buf[2] = (v >> 16) & 0xFF;
        buf[3] = (v >> 24) & 0xFF;
    }

    static void writeLELong(unsigned char *buf, uint64_t v)
    {
        buf[0] = (v >> 0) & 0xFF;
        buf[1] = (v >> 8) & 0xFF;
        buf[2] = (v >> 16) & 0xFF;
        buf[3] = (v >> 24) & 0xFF;
        buf[4] = (v >> 32) & 0xFF;
        buf[5] = (v >> 40) & 0xFF;
        buf[6] = (v >> 48) & 0xFF;
        buf[7] = (v >> 56) & 0xFF;
    }

private:
    void frame();
    bool commitReady() const;
    bool checkSpace(int bytes);

    char * const mBuf;
    sem_t * const mReaderSem;
    uint64_t mCommitTime;
    sem_t mWriterSem;
    const int mSize;
    int mReadPos;
    int mWritePos;
    int mCommitPos;
    bool mAvailable;
    bool mIsDone;
    const int32_t mCore;
    const int32_t mBufType;

    // Intentionally unimplemented
    Buffer(const Buffer &);
    Buffer &operator=(const Buffer &);
};

#endif // BUFFER_H
