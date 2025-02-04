#pragma once
#include <functional>
#include <vector>
#include "IEvent.h"
#include "../../Logger/Logger.h"

template<typename ...TArgs>
class Event : public IEvent{
private:
    std::vector<std::function<void(TArgs...)>> events;
public:

    void operator +=(std::function<void(TArgs...)>  event) 
    {
        events.push_back(event);
    };

    void operator()(TArgs&& ...args){
        Invoke(std::forward<TArgs>(args)...);
        Logger::Log("Chamou O event");
    };

    void Invoke(TArgs&& ...args) { 
        for(auto event : events){
            event(std::forward<TArgs>(args)...);
        }
    };
};