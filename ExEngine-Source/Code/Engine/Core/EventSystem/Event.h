#pragma once
#include <functional>
#include <vector>
#include "IEvent.h"

template<typename ...TArgs>
class Event : public IEvent{
private:
    std::vector<std::function<void(TArgs...)>> events;
public:

    void operator +=(std::function<void(TArgs...)>  event) 
    {
        events.push_back(event);
    };

    const void operator()(TArgs&& ...args){
        Invoke(std::forward<TArgs>(args)...);
    };

    void Invoke(TArgs&& ...args) const { 
        for(auto event : events){
            event(std::forward<TArgs>(args)...);
        }
    };
};