# COS214-Prac6

Runtime decorators
- Added GetPriority to RuntimeComponent
- PriorityDecorator should naturally integrate with the iterator assuming the virtual GetChildren is called, its functionality is that it changes GetChildren method to arrange in the order of priorities they have
- Security decorator enforces security in the sense that if it is made needs the component to also have an attached attached participant, and requiredRoleId arguments in the constructor and it'll validate whether the attached arguments in the constructor allow for a setState call

## Added scenario (scenario1.cpp) build using "make Scenario1"
