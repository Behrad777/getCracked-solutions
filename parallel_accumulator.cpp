#include <iterator>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <vector>


#define max_thread_workload 12000000

//the start iterator is determined by the calling function, it knows only to go up to
template <typename Iter, typename Value>
Value parallel_calculator_callback(Iter startIter, Iter endIter){
    return std::accumulate(startIter, endIter, Value{}); // correct Value type
}

template <typename Iter, typename Value>
Value DoParallelCalc(const Iter startIter, const Iter endIter, Value initialValue = {}) { //wtf is the point of initial value
    std::vector<std::thread> threads;

    Value accumulated_sum{initialValue};
    auto num_threads_required{0};
    const auto data_size_total{std::distance(startIter, endIter)};
    const auto& global_end_iter = endIter; //pass this into the threads so the last thread doesnt overflow

//------unallowed cases------
    if(data_size_total<0){
        throw std::logic_error("Cant have the end iterator before the start iterator gng");
    }
    if(data_size_total==0) return initialValue; //empty range or array

//------calculate the number of threads we need------

    //if we have 1000 points just have single threaded execution and return
    if(data_size_total <=max_thread_workload){
        return std::accumulate(startIter, endIter, accumulated_sum);
    }

    //otherwise, divide it up, the last thread may only need to do a small amount of work if its not an even multiple of 1000
    num_threads_required =(data_size_total + max_thread_workload - 1) / max_thread_workload; // ceil(n/workload)
    threads.reserve(num_threads_required);
    std::vector<Value> results(num_threads_required, Value{});

//------begin the callbacks------

//0->1000, 1000->2000, 2000->3000 etc
// no need to hardcode this in, just while
    for(size_t i{0}; i<num_threads_required; ++i){
        auto local_start = startIter + (max_thread_workload * i);
        auto local_end   = std::min(local_start + max_thread_workload, endIter); //dont go past end

        threads.emplace_back([&, i, local_start, local_end] {results[i] = parallel_calculator_callback<Iter, Value>(local_start, local_end);});
    }

    for (auto& t : threads) t.join();
    for (const auto& r : results) accumulated_sum += r;
    return accumulated_sum;
}
