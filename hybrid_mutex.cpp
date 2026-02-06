#include <atomic>
using namespace std;

#define SPIN_CYCLES 100

class Mutex
{
public:
    void lock()
    {
        //checks immidately if we get released
        while(occupied.exchange(true, memory_order_acquire)){
            for(int i = 0; i < SPIN_CYCLES; ++i){
                if(!occupied.exchange(true, memory_order_acquire)){ //previous value was false, and we just acquired it, setting it to true
                    return; //are we locked!
                }
                //else spin more
            }

            //spins didn't work, wait on the atomic now
            occupied.wait(true, memory_order_acquire); //the old value should be true if we're not able to get it

        }
        return;


    }

    //only a thread that holds the mutex can unlock it
    void unlock()
    {
        occupied.store(false, memory_order_relaxed); //we control it, relax memory order
        occupied.notify_one(); //notify just 1 thread, preventing a bunch of cpu overhead and cache line contention

    }
private:
atomic<bool> occupied{false};
};


//plan:
// use an atomic as the synchronization primitive
// spin a few cycles atomically loading the variable with memory order acqire
