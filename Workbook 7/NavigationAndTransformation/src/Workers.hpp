#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include<condition_variable>
#include <queue>


class Semaphore {
public:

	void incriment();

	void decriment();

	unsigned getCount();

	Semaphore(unsigned semCount);
private:
	std::mutex internalMutex;		//Locked for the duration of any method to force single-threadedness
	std::condition_variable blocker;//Used to block threads on a decriment
	std::atomic_uint semCount;		//The internal track of the semaphores value
};

//Internal mutex is locked on all operations meaning only one function can be running in a thread at once

void Semaphore::incriment() {
	std::unique_lock<std::mutex> lock(internalMutex);
	semCount += 1;
	//Notify any waiting thread
	blocker.notify_one();
}

void Semaphore::decriment() {
	std::unique_lock<std::mutex> lock(internalMutex);

	//Wait for a count
	while (semCount == 0)
		//blocks, but in a while loop just in case
		blocker.wait(lock);

	semCount -= 1;
}


unsigned Semaphore::getCount() {
	return semCount;
}

Semaphore::Semaphore(unsigned semCount) :
	semCount(semCount)
{}


namespace pool{


    namespace hidden{
        struct Work{
            void* workInfo;
            void(*workFunction)(void*);

            void operator()(){
                workFunction(workInfo);
            }
        };

        std::mutex workQueueLock;
        std::queue<Work> workQueue;
        Semaphore workSemaphore(0);

        const unsigned numberOfWorkers = 1;
        std::array<std::thread, numberOfWorkers> workers;
        Semaphore deadWorkerLock(0);

        bool shutdown = false;

        void worker(){
            while(shutdown == false){
                workSemaphore.decriment();
                workQueueLock.lock();
                Work myWork = workQueue.front();
                workQueue.pop();
                workQueueLock.unlock();

                myWork();
            }
            deadWorkerLock.incriment();
        }
    }


    void addWork(void(*func)(void*), void* info){
        hidden::Work w;
        w.workInfo = info;
        w.workFunction = func;

        hidden::workQueueLock.lock();
        hidden::workQueue.emplace(w);
        hidden::workSemaphore.incriment();
        hidden::workQueueLock.unlock();

    }


    void init(){
        hidden::shutdown = false;
        for(size_t i = 0; i < hidden::numberOfWorkers; i++)
            hidden::workers[i] = std::thread(hidden::worker);
    }

    void shutdown(){
        hidden::shutdown = true;
        for(size_t i = 0; i < hidden::numberOfWorkers; i++)
            addWork([](void*){}, nullptr);
        for(size_t i = 0; i < hidden::numberOfWorkers; i++){
            hidden::deadWorkerLock.decriment();
            hidden::workers[i].join();
        }
        
        while(hidden::workSemaphore.getCount() > 0)
            hidden::workSemaphore.decriment();
        while(hidden::workQueue.size() > 0)
            hidden::workQueue.pop();
    }
}
