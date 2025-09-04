//
// Created by Behzad Haki on 2022-02-11.
//
#pragma once

#include <juce_core/juce_core.h>
#include <utility>

using namespace std;

// ============================================================================================================
// ==========          LockFreeQueue (First In - First Out)          ==========================================
// ============================================================================================================

// Lock-free queue for GUI/Audio thread communication
template<typename T, int queue_size>
class DynamicLockFreeQueue {
private:
    std::unique_ptr<juce::AbstractFifo> lockFreeFifo;
    std::vector<T> data;

    int num_reads = 0;
    int num_writes = 0;
    T latest_written_data{}; // snapshot of last pushed value

public:
    DynamicLockFreeQueue()
    {
        lockFreeFifo = std::make_unique<juce::AbstractFifo>(queue_size);
        data.resize(queue_size); // allocate queue_size Ts
    }

    int getNumReady() const { return lockFreeFifo->getNumReady(); }

    void push(const T& writeData)
    {
        int start1, start2, blockSize1, blockSize2;
        lockFreeFifo->prepareToWrite(1, start1, blockSize1, start2, blockSize2);

        if (blockSize1 > 0)
            data[start1] = writeData;

        latest_written_data = writeData;
        ++num_writes;

        lockFreeFifo->finishedWrite(1);
    }

    T pop()
    {
        int start1, start2, blockSize1, blockSize2;
        lockFreeFifo->prepareToRead(1, start1, blockSize1, start2, blockSize2);

        T res{};
        if (blockSize1 > 0)
            res = data[start1];

        ++num_reads;
        lockFreeFifo->finishedRead(1);

        return res;
    }


    T getLatestOnly()
    {
        int numReady = getNumReady();
        if (numReady == 0)
            return latest_written_data;

        int start1, start2, blockSize1, blockSize2;
        lockFreeFifo->prepareToRead(numReady, start1, blockSize1, start2, blockSize2);

        T readData{};
        if (blockSize2 > 0)
            readData = data[start2 + blockSize2 - 1];
        else if (blockSize1 > 0)
            readData = data[start1 + blockSize1 - 1];

        ++num_reads;
        lockFreeFifo->finishedRead(numReady);

        return readData;
    }

    int getNumberOfWrites() const { return num_writes; }

    // 👇 Safe GUI peek (does not move FIFO heads)
    T getLatestDataWithoutMovingFIFOHeads() const
    {
        return latest_written_data;
    }
};

// use this queue only for types that don't change size after initialization
// and also have a default constructor and a copy constructor
template<typename T, int queue_size>
class StaticLockFreeQueue {
private:
    std::unique_ptr<juce::AbstractFifo> lockFreeFifo;
    juce::Array<T> data;

    // keep track of number of reads/writes and the latest_value without moving FIFO
    int num_reads = 0;
    int num_writes = 0;
    T latest_written_data{};

public:
    StaticLockFreeQueue() {

        lockFreeFifo = std::unique_ptr<juce::AbstractFifo>(
            new juce::AbstractFifo(queue_size));

        data.ensureStorageAllocated(queue_size);

        while (data.size() < queue_size) {
            // check if T is a tuple
            data.add(T());
        }
    }

    int getNumReady() {
        return lockFreeFifo->getNumReady();
    }

    void push(T writeData) {

        int start1, start2, blockSize1, blockSize2;

        lockFreeFifo->prepareToWrite(
            1, start1, blockSize1,
            start2, blockSize2);
        auto start_data_ptr = data.getRawDataPointer() + start1;
        *start_data_ptr = writeData;
        latest_written_data = writeData;
        num_writes += 1;
        lockFreeFifo->finishedWrite(1);

    }

    T pop() {
        int start1, start2, blockSize1, blockSize2;

        lockFreeFifo->prepareToRead(
            1, start1, blockSize1,
            start2, blockSize2);

        auto start_data_ptr = data.getRawDataPointer() + start1;

        auto res = *(start_data_ptr);
        lockFreeFifo->finishedRead(1);
        num_reads += 1;

        return res;
    }


    T getLatestOnly() {
        int start1, start2, blockSize1, blockSize2;
        T readData;

        lockFreeFifo ->prepareToRead(
            getNumReady(), start1, blockSize1,
            start2, blockSize2);

        if (blockSize2 > 0) {
            auto start_data_ptr = data.getRawDataPointer() + start2;
            readData = *(start_data_ptr+blockSize2-1);
            lockFreeFifo -> finishedRead(blockSize1+blockSize2);
            num_reads += 1;
            return readData;

        }
        if (blockSize1 > 0) {
            auto start_data_ptr = data.getRawDataPointer() + start1;
            readData = *(start_data_ptr+blockSize1-1);
            lockFreeFifo -> finishedRead(blockSize1+blockSize2);
            num_reads += 1;
            return readData;
        }

    }

    int getNumberOfWrites() {
        return num_writes;
    }

    // This method is useful for keeping track of whether any data has previously
    //      written to Queue regardless of being read or not
    // !! This method should only be used for initialization of GUI objects !!
    // !!! To use the QUEUE for lock free communication use the ReadFrom() or pop() methods!!!
    T getLatestDataWithoutMovingFIFOHeads() {
        return latest_written_data;
    }

};
