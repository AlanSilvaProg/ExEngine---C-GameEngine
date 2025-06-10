#pragma once

class EntityBrowserNode{
private:
    int nodeId;
public:

    EntityBrowserNode(int& entityId) : nodeId(entityId) {}

    int& GetNodeId();
};