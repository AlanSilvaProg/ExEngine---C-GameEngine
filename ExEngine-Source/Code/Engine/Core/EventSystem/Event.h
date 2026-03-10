#pragma once
#include <functional>
#include <vector>
#include "IEvent.h"

template<typename ...TArgs>
class Event : public IEvent{
private:
    std::vector<std::pair<int, std::function<void(TArgs...)>>> events;
    int nextId = 0;
public:

    int operator +=(std::function<void(TArgs...)>  event) 
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

    const void operator()(TArgs&& ...args){
        Invoke(std::forward<TArgs>(args)...);
    };

    void Invoke(TArgs&& ...args) const { 
        for(auto event : events){
            event.second(std::forward<TArgs>(args)...);
        }
    };
};