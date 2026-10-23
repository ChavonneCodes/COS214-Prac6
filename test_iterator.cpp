#include "Iterator.h"
#include "StageInstance.h"

int main()
{
	StageInstance stageInstanceRoot("id", new StageDefinition("id", "root"));
	StageInstance* stageInstance0 = new StageInstance("id", new StageDefinition("id", "name0"));
	StageInstance* stageInstance01 = new StageInstance("id", new StageDefinition("id", "name01"));
	StageInstance* stageInstance02 = new StageInstance("id", new StageDefinition("id", "name02"));
	StageInstance* stageInstance1 = new StageInstance("id", new StageDefinition("id", "name0"));
	stageInstanceRoot.addChild(stageInstance0);
	stageInstance0->addChild(stageInstance01);
	stageInstance0->addChild(stageInstance02);
	stageInstanceRoot.addChild(stageInstance1);
	auto iter = stageInstanceRoot.createIterator(new DFSTraversalStrategy<RuntimeComponent>());
	while(iter->hasNext())
		dynamic_cast<const RuntimeComponent*>(&iter->getNext())->print();
	return 0;
}
