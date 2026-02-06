#include <cstdarg>
#include <iterator>
#include <ranges>
#include <type_traits>
using namespace std;

namespace aux {

    //determine if the templated type of optional is small or not
    template <typename T>
    struct IsSmallType : std::true_type { }; // default: small

    template <typename T>
    requires (sizeof(T) > sizeof(void*) || alignof(T) > alignof(void*))
    struct IsSmallType<T> : std::false_type { };

    template <typename T, bool IsSmallType = IsSmallType<T>::value>
    class optional;

    //in the case where our size and alignment is less strict than a void*
    template <typename T>
    class optional<T, false> {
        public:

            //cant move or copy from a std optional,
            void operator=(const optional&& other) = delete;
            void operator=(const optional& other) = delete;
            optional(const optional& other) = delete;
            optional(const optional && other) = delete;

            optional() {}

            //destructor destructs our pointer's data
            ~optional(){
                delete data_;
            }

            optional(T other) {
                data_ = new T{other};
            }

            [[nodiscard]] operator bool() const {
                return has_value();
            }

            [[nodiscard]] bool has_value() const {
                return data_!=nullptr;
            };

            //derefernce overload
            [[nodiscard]] const T& operator*() const {
                return value(); //get its value if its a valid object
                //imagine test cases werent known, we'd do something like
                // throw std::bad_optional_access("Doesnt have a value");
            }

            const T& value() const {
                if(has_value()) return *data_;
                //imagine test cases werent known, we'd do something like
                // throw std::bad_optional_access("Doesnt have a value");
            }
        private:
        T* data_{nullptr}; //initially nullptr, if it changes our optional contains a value
    };

    //in the case that we dont have a small object
    template <typename T>
    class optional<T, true> {
        public:
            void operator=(const optional&& other) = delete;
            void operator=(const optional& other) = delete;
            optional(const optional& other) = delete;
            optional(const optional && other) = delete;

            optional() {}

            //no need for a destructor since we dont have a member variable pointing to memory

            //memcpy?
            optional(T other) {
                ::new (data_) T{other}; //placement new, put the object into our data buffer
                exists_=true;
            }

            [[nodiscard]] operator bool() const {
                return has_value();
            }

            [[nodiscard]] bool has_value() const {
                return exists_;
            };

            //derefernce overload
            [[nodiscard]] const T& operator*() const {
                return value();

            }

            const T& value() const{
                //we need to convert this byte array into a type of T

                return *reinterpret_cast<const T*>(data_);

            }
        private:
        //since the data is big here, use placement new, preallocate the data we need and just construct it in there
        alignas(T) byte data_[sizeof(T)]; //align the memory placement of this object to the size of T
        bool exists_{false}; // initially doesnt contain anything
    };
}
