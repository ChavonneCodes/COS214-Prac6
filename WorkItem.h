#ifndef WORK_ITEM_H
#define WORK_ITEM_H

#include "RuntimeComponent.h"
#include "ActivityDefinition.h"
#include "WorkItemState.h"


class WorkItem : public RuntimeComponent {
private:
    std::string instanceId;
    ActivityDefinition* definition;
    std::string state;
    WorkItemState* currentState;

public:
    WorkItem(const std::string& instanceId, ActivityDefinition* def);
    
    std::string getId() const override;
    std::string getState() const override;
    void setState(const std::string& newState) override;
    void print(int indentLevel = 0) const override;
    void changeState(WorkItemState* newState);
    bool assign(Participant* p);
    bool start();
    bool complete();
    bool reject();
    bool cancel() ;
    bool escalated();
    bool fail();
    bool makeAvailable();
       

};

#endif
