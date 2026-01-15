// Write your solution here
// C++23 for C++ using GCC 14.2
// /////////////////////////////////////////////////////////////////////////
// If C++: Your code is automatically compiled with a precompiled header. //
// 99% of includes / packages are already added for you.                  //
// You do NOT need to add your own includes here.                         //
// /////////////////////////////////////////////////////////////////////////
#include <cstddef>
#include<mutex>
#include <atomic>
#include <semaphore>


namespace getcracked 
{
    //shared instance of this class is accessed by all the readers and writers 
    class ReaderWriterLock
    {
    public:

        unsigned ReaderLock()
        {
            //at the time of call how many readers in there
            reader_count_mut.lock();
            size_t current_readers = reader_count;
            reader_count_mut.unlock();

            //
            turnstile.acquire();
            turnstile.release();  //if the writer was able to get past the turnstile

            reader_count_mut.lock();
                reader_count++;
                if(reader_count ==1){ //first reader, unlock the mutex for everybody
                    room_empty.acquire(); 
                }
            reader_count_mut.unlock();

            return (unsigned)current_readers;
            //enter the critical area 

            
        }

        unsigned ReaderUnlock() 
        { 

            reader_count_mut.lock();
            size_t current_readers = reader_count;
            reader_count--;
            if(reader_count ==0){
                room_empty.release();
            }
            reader_count_mut.unlock();
            return (unsigned)current_readers;
        }

        void WriterLock() 
        { 
            turnstile.acquire(); //lock the turnstile to prevent readers from entering
            waiting_writers++; //a writer is waiting
            room_empty.acquire();
            waiting_writers--; //one writer at a time has gained access to the data
            if(waiting_writers.load(std::memory_order_relaxed) == 0){ //only if there are no writers waiting, relaxed cuz writers own it
                turnstile.release(); 
            }
            //the rest is done in the unlocking
        }

        void WriterUnlock()
        { 
            room_empty.release();
        }

    private:

    std::atomic<int> waiting_writers{0}; //if a reader wants to enter dont if a writer wants 
    
    size_t reader_count{0}; //for changing the value of the readers in the critical area
    std::mutex reader_count_mut;
    
    std::binary_semaphore turnstile{1}; // sem is immediately posted after a reader acquires it, but the writers can lock it to prevent more readers from entering 
    std::binary_semaphore room_empty{1}; // if room empty is 1, we can enter, if 0 its full

    // std::mutex critical_section; not necessary 

    };
}