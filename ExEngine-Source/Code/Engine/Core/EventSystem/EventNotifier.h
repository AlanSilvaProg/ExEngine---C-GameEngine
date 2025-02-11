#pragma once
#include "IEvent.h"
#include <vector>

class EventNotifier : public IEvent{
private:
    std::vector<std::function<void()>> events;
public:

    void operator +=(std::function<void()>  event) 
    {
        events.push_back(event);
    };

    const void operator()(){
        Invoke();
    };

    void Invoke() const { 
        for(auto event : events){
            event();
        }
    };
};