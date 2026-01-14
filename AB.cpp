// Write your solution here
// C++23 for C++ using GCC 14.2
// /////////////////////////////////////////////////////////////////////////
// If C++: Your code is automatically compiled with a precompiled header. //
// 99% of includes / packages are already added for you.                  //
// You do NOT need to add your own includes here.                         //
// /////////////////////////////////////////////////////////////////////////
#include <cstddef>
#include <set>
#include <unordered_map>
#include<vector>
#include<generator>

enum class Stream { A, B };
struct Packet {
    Stream Source;
    size_t ReceiveTime;
    int SequenceNumber;
};

//i hope the move operator is good lmao
using Packets = std::vector<Packet>;

//IF PACKET RECEIVED AT THE SAME TIME FAVOR STREAM A
Packets ArbitrageStreams(std::generator<Packet>& streamA, std::generator<Packet>& streamB) {
    Packets packets;

    auto StreamA_iter = streamA.begin();
    auto StreamB_iter = streamB.begin();

    //I guess the end of a cpp23 generator is fixed...
    const auto StreamA_end = streamA.end();
    const auto StreamB_end = streamB.end();

    //hash map of sequence_number -> packet struct, contains a copy of the sequence number but its ok 
    //if the time of a B stream object for the same sequence number is bette than the A object, override it to the B stream object

    std::unordered_map<size_t, Packet> ab_arb;
    std::set<size_t> keys; //used instead of sorting the unordered set 

    while(StreamA_iter != StreamA_end || StreamB_iter != StreamB_end){ // end if both streams fully iterated

        //intake A first since A is favoured we dont need to perform another memory operation
        //also check if A is able to overwrite a B object in the same sequence number
        if(StreamA_iter!= StreamA_end){
            const auto a_packet = *StreamA_iter;
            auto it = ab_arb.find(a_packet.SequenceNumber);
            if(it == ab_arb.end()){ //if its empty
                ab_arb.emplace(a_packet.SequenceNumber, a_packet); // wtf is going on with these hashmap inserts bruh
                keys.insert(a_packet.SequenceNumber);
            } else{ //already a value from B here
                const auto& current_packet = it->second; //take the B packet 
                if(a_packet.ReceiveTime <= current_packet.ReceiveTime){ //if A is better or equal replace that object
                    it->second = a_packet; //replace the packet with A's
                }
            }
            ++StreamA_iter;
        }

        //now for B, check if A already has a hashed one there and overwrite if better receive time 
        if(StreamB_iter != StreamB_end){
            const auto b_packet = *StreamB_iter;
            auto it = ab_arb.find(b_packet.SequenceNumber); // check if a packet is already chosen for this sequence number
            if(it == ab_arb.end()){ //nothing there, add the B packet 
                ab_arb.emplace(b_packet.SequenceNumber, b_packet);
                keys.insert(b_packet.SequenceNumber);
            } else{ //already exists, see if we can override 
                const auto& current_packet = it->second;
                if(b_packet.ReceiveTime < current_packet.ReceiveTime){
                    it->second = b_packet;
                } 
            }
        ++StreamB_iter;
        }
    }

    //from the keys set, iterate over the set instead of sorting the unordered map
    for(const auto& k: keys){
        auto it = ab_arb.find(k); //iterator to the packet
        const auto packet = it->second;
        packets.push_back(packet);
    }
    return packets;
}