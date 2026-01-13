// Write your solution here
// C++23 for C++ using GCC 14.2
// /////////////////////////////////////////////////////////////////////////
// If C++: Your code is automatically compiled with a precompiled header. //
// 99% of includes / packages are already added for you.                  //
// You do NOT need to add your own includes here.                         //
// /////////////////////////////////////////////////////////////////////////

#include <cstddef>
using namespace std;
namespace getcracked

{
    template <typename T>
    class SPSCQ
    {
    public:
        SPSCQ(size_t capacity): capacity_(capacity), queue_(make_unique<T[]>(capacity_)) {
            if(bitset<32>(capacity_).count()!= 1 || capacity_ < 2){
                throw logic_error("The capacity needs to be a power of 2");
            }
        }

        ~SPSCQ() = default;

        //pushing is done by the producer, so memory acquire the consumers index
        bool push(const T& item)
        {
            const auto prod_pos = producer_position_.load(memory_order_relaxed);
            const auto cons_pos = consumer_position_.load(memory_order_acquire);
            const auto next_producer = (prod_pos+1) & (capacity_-1);

            if(next_producer == cons_pos) return false; //full, no pushing
            
            queue_[prod_pos] = item;

            producer_position_.store(next_producer, memory_order_release); 
            return true;
        }

        bool pop(T& item)
        {
            //popping is done by the consumer, so memory acquire the producer's index
            const auto prod_pos = producer_position_.load(memory_order_acquire);
            const auto cons_pos = consumer_position_.load(memory_order_relaxed); //no need for memory guaruntees here since just reading 
            const auto next_consumer = (cons_pos+1) & (capacity_-1);
            
            if(cons_pos == prod_pos) return false; //queue is empty, cant pop anything 
            
            item = move(queue_[cons_pos]); 
            consumer_position_.store(next_consumer, memory_order_release);
            return true;

        }

        [[nodiscard]] bool full() const
        {
            const auto prod_pos = producer_position_.load(memory_order_relaxed);
            const auto cons_pos = consumer_position_.load(memory_order_relaxed); //no need for memory guaruntees here since just reading 
            const auto next_producer = (prod_pos+1) & (capacity_-1);
            return next_producer == cons_pos;
            
        }

        [[nodiscard]] size_t size() const
        {
            const auto prod_pos = producer_position_.load(memory_order_relaxed);
            const auto cons_pos = consumer_position_.load(memory_order_relaxed);

            if(prod_pos > cons_pos){
                return prod_pos - cons_pos;
            }
            else if (prod_pos < cons_pos){
                return capacity_ - (cons_pos-prod_pos); //space betwene them taken away from queue
            }

            //if theyre the same at starting 
            return 0;
        }

        [[nodiscard]] bool empty() const
        {
            const auto prod_pos = producer_position_.load(memory_order_relaxed);
            const auto cons_pos = consumer_position_.load(memory_order_relaxed); //no need for memory guaruntees here since just reading 
            return cons_pos == prod_pos;
        }

        //throw for move
        void operator=(SPSCQ&& other){
            throw logic_error("Cant move");
        }
        SPSCQ(SPSCQ&& other){
            throw logic_error("Cant move");
        }

        //throw for copy 
        void operator=(SPSCQ& other){
            throw logic_error("cant copy");
        }
        SPSCQ(SPSCQ& other){
            throw logic_error("Cant copy");
        }
    private:
        size_t capacity_;
        unique_ptr<T[]> queue_; //templated of that type, also automatically garbage collected so no manual deallocation not required

        //put the consumer and producer iterators on different cache lines to prevent cache invalidation
        //producer should always be in front of consumer
        alignas(hardware_destructive_interference_size);
        atomic<size_t> consumer_position_{0};

        alignas(hardware_destructive_interference_size);
        atomic<size_t> producer_position_{0};

        //we can use bitmask to get the modulo, eg capacity is 15
        //15 -1 is 00111
        //21 is    10101
        // masked: 00001  

    };
}