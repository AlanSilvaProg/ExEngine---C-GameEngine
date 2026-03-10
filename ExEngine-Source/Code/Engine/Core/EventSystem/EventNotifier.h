#pragma once
#include "IEvent.h"
#include <vector>

class EventNotifier : public IEvent{
private:
    std::vector<std::pair<int, std::function<void()>>> events;
    int nextId = 0;
public:

    int operator +=(std::function<void()>  event) 
    {
        int id = nextId++;
        events.emplace_back(id, std::move(event));
        return id;
    };

    void operator -=(int id) 
    {
        events.erase(
            std::remove_if(events.begin(), events.end(),
                [id](auto& pair){ return pair.first == id; }),
            events.end()
        );
    };

    void Invoke() const { 
        for(auto event : events){
            event.second();
        }
    };
};