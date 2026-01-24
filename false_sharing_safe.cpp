#include <new>
#include <memory>
#include <concepts>
#include <vector>

template<std::integral T1, std::integral T2> //specify the type is integral
struct Data{

    alignas(std::hardware_destructive_interference_size) std::atomic<T1> x;
    alignas(std::hardware_destructive_interference_size) std::atomic<T2> y;
};
//yes this is the completed answer lmao
