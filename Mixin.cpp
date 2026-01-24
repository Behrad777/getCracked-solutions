// Write your solution here
// C++23 using GCC 14.2
// Python 3.12.3
//
// If Python: The packages you need to solve any problem are already
// installed on the host's venv.
//
// If C++: Your code is automatically compiled with a precompiled header.
// 99% of includes / packages are already added for you.
// You do NOT need to add your own includes here.
// /////////////////////////////////////////////////////////////////////////


//open and close is the candle body, high/low is the sticks on each side

// MODIFY TO MAKE THIS A MIXIN.
#include <vector>
#include<string>


//mixin util structs
struct Label {
    std::string Text;
};

struct Color {
    int Alpha;
};


template<class... Mixins> //unspecified number of mixins
struct OHLC : Mixins... { //add the mixins here
    double Open, High, Low, Close;
};

using LibOHLC = OHLC<Label, Color>; //libOHLC is a specific case where we include 2 mixins into OHLC

struct Chart {
private:
    std::vector<LibOHLC> openHighLowCloses_; //vector of the sticks
public:
    Chart(std::initializer_list<LibOHLC> ohlcs) : openHighLowCloses_{ ohlcs } { }
    std::string GetFirstLabel() const { return openHighLowCloses_.size() ?
        openHighLowCloses_.at(0).Text : ""; }
    int GetFirstAlpha() const { return openHighLowCloses_.size() ?
        openHighLowCloses_.at(0).Alpha : -1; }
};
