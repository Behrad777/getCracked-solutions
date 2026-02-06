#include <iterator>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <iostream>
using namespace std;

enum class Kind {
      Begin,
      End,
};

struct Event {
      Kind Type;
      double Elapsed;
      string Method;
};

using Events = vector<Event>;

struct Sample {
      double Elapsed;
      vector<string> Stack;
};

using Samples = vector<Sample>;



//helper for getting the common prefix on two consecutive samples
int commonPrefix(vector<string> prev, vector<string> current){
    size_t k = 0;
    while (k < prev.size() && k < current.size() && prev[k] == current[k]) ++k;
    return k;
}

Events GenerateEvents(const Samples& samples) {
    Events events; //answer goes in here
    if (samples.empty()) return {};

    //initial size of the call stack
    const double prev_elapsed  = samples[0].Elapsed;

    for(const auto initial_function : samples[0].Stack){
        Event begin_event{ Kind::Begin, prev_elapsed, initial_function };
        events.push_back(begin_event);
    }
    //works up to this point, we can put the first one in here no problem

    vector<string> prev_stack = samples[0].Stack;

    for (size_t i = 1; i < samples.size(); ++i) {
        const auto& current_stack = samples[i].Stack;
        const double current_elapsed = samples[i].Elapsed;

        int common_prefix_index = commonPrefix(prev_stack, current_stack);

        // cout << "common prefix index: " << common_prefix_index << endl;

        //remove all the frames of the previous stack from end to k
        for (int end_index = prev_stack.size()-1; end_index >= common_prefix_index; --end_index) {
            // cout <<" currently ending event: " << prev_stack[end_index] << endl;
            Event end_event = {Kind::End, current_elapsed, prev_stack[end_index]};
            events.push_back(end_event);
        }

        //add all the new calls of the current call stack
        for (int begin_index = common_prefix_index; begin_index< current_stack.size(); ++begin_index) {
            Event begin_event{ Kind::Begin, current_elapsed, current_stack[begin_index] };
            events.push_back(begin_event);
        }
        //after everything is done update the prev
        prev_stack = current_stack;
    }

    return events;
}


int main() {
    Samples samples = {
        { 1.0, { "main" } },
        { 2.0, { "main", "foo" } },
        { 3.0, { "main", "foo", "bar" } },
        { 4.0, { "main", "foo", "ball" } },
        { 5.0, { "main" } }
    };


    Events events = GenerateEvents(samples);
    //bar is not being ended in this test case

    for (const auto& e : events) {
        cout << (e.Type == Kind::Begin ? "Begin" : "End")
             << "(" << e.Elapsed << ", " << e.Method << ")\n";
    }

    return 0;
}



//basically getting the difference in the call stack at every point
/*

Input Samples

(1.0) [main]

(2.0) [main, foo]

(3.0) [main, foo, bar]

(4.0) [main]

Output Events

Begin(1.0, main)

Begin(2.0, foo)

Begin(3.0, bar)

End(4.0, bar)

End(4.0, foo)

(At 4.0, both bar and foo are gone, so you emit two End events, inner first.)

 */
