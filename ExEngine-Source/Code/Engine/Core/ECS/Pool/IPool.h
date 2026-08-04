#pragma once

class IPool{
public:
    virtual ~IPool() = default;
    virtual void CopyComponent(const int fromEntityId, const int toEntityId) = 0;
};