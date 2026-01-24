#include <optional>
#include <cmath>
#include <unistd.h>
#include <vector>
#include <iostream>

using DTE = double;
using Price = long;
using Prices = std::vector<Price>;

enum class Tenor {
    Month,
    Annual,
};

struct Instrument {
    Tenor PaymentFrequency;
    DTE DaysToExpiration;
    Prices SettlementPrices;
};

using Instruments = std::vector<Instrument>;


//helper functions
static bool closeTo(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

static bool validMonthTradable2Settle(const Instrument& i) {
    return i.PaymentFrequency == Tenor::Month
        && i.SettlementPrices.size() >= 2;
}

static void keepMaxDTE(std::optional<Instrument>& best, const Instrument& cand) {
    if (!best || cand.DaysToExpiration > best->DaysToExpiration) best = cand;
}

static void keepMinDTE(std::optional<Instrument>& best, const Instrument& cand) {
    if (!best || cand.DaysToExpiration < best->DaysToExpiration) best = cand;
}

Instruments SelectInstruments(const Instruments& instruments) {

    Instruments selected; //result vector
    selected.reserve(2); //allocate space for 2 instruments

    //zero init this, since a valid value can never be 0,
    std::optional<Instrument> bestShort;
    std::optional<Instrument> fallbackShort; //keep track of this as we go to prevent looping again

    for(const auto& instrument : instruments){


        //---------SPECIAL CASES----------

        //ignore year tenored instruments, ignore expired instruments
        if(instrument.PaymentFrequency == Tenor::Annual || instrument.DaysToExpiration<=0){
            continue;
        }
        //DTE==30 and 2 or more settlements, special case
        if(closeTo(instrument.DaysToExpiration, 30) && instrument.SettlementPrices.size() >=2){
            return {instrument}; //select and return just this one
        }


        //---------Short term selection LOGIC-------------

        //select a non expired monthly instrument
        if(validMonthTradable2Settle(instrument)){
            if(instrument.DaysToExpiration >=10.0 && instrument.DaysToExpiration <30.0){
                keepMaxDTE(bestShort, instrument); //keep track of the max short
            } else if(instrument.DaysToExpiration > 30){
                keepMinDTE(fallbackShort, instrument);
            }
        }
    }

    //we can use std optional to select the best short term instrument
    std::optional<Instrument> near = bestShort ? bestShort : fallbackShort;
    if (!near) return { }; //we couldnt find a non-expired DTE instrument, fail right here immediately

    //we found at least a best short instrument or a fallback value, now we look for a minimum long value
     selected.push_back(*near);

     //---------Long term selection LOGIC-------------
     std::optional<Instrument> bestLong; // trying to get as close to 30 as possible
     const double nearDTE = near->DaysToExpiration; //needs to be bigger than this

     //2nd loop, Long Term Selection Criteria
     for(const auto& instrument : instruments){

         if(!validMonthTradable2Settle(instrument))continue; //skip expired or instruments with less than 2 settlement prices

         if(instrument.DaysToExpiration > 30.0 && instrument.DaysToExpiration > nearDTE){
             keepMinDTE(bestLong, instrument);
         }
     }

    //if we got a value in the optional, push the value onto selected
    if (!bestLong) return {};
    selected.push_back(*bestLong);
    return selected;
}

int main(){
    //a test case where theyre all too small
    std::vector<Instrument> instruments = {
        { Tenor::Month, 1.5,  {100, 101} },
        { Tenor::Month, 5.0,  {102, 103} },
        { Tenor::Month, 9.9,  {104, 105} },
        { Tenor::Month, 0.5,  {106, 107} }, // effectively expired / too small
        { Tenor::Annual, 20.0,{108, 109} }, // ignored (Year)
    };

    auto result = SelectInstruments(instruments);
    std::cout << "Selected Instruments: ";
    for(const auto& instrument : result){
        std::cout << instrument.DaysToExpiration << " ";
    }
    std::cout << std::endl;

}
