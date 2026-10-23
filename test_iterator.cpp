#include "Iterator.h"
#include "StageInstance.h"

int main()
{
	StageInstance stageInstanceRoot("id", new StageDefinition("id", "root"));
	StageInstance* stageInstance0 = new StageInstance("id", new StageDefinition("id", "name0"));
	StageInstance* stageInstance01 = new StageInstance("id", new StageDefinition("id", "name01"));
	StageInstance* stageInstance02 = new StageInstance("id", new StageDefinition("id", "name02"));
	StageInstance* stageInstance1 = new StageInstance("id", new StageDefinition("id", "name1"));
	StageInstance* stageInstance2 = new StageInstance("id", new StageDefinition("id", "name2"));
	stageInstanceRoot.addChild(stageInstance0);
	stageInstance0->addChild(stageInstance01);
	stageInstance0->addChild(stageInstance02);
	stageInstanceRoot.addChild(stageInstance1);
	stageInstanceRoot.addChild(stageInstance2);
	cout << "Depth first traversal" << endl;
	auto iterDFS = stageInstanceRoot.createIterator(new DFSTraversalStrategy<RuntimeComponent>());
	while(iterDFS->hasNext())
		dynamic_cast<const RuntimeComponent*>(&iterDFS->getNext())->print();
	cout << "Breadth first traversal" << endl;
	auto iterBFS = stageInstanceRoot.createIterator(new BFSTraversalStrategy<RuntimeComponent>());
	while(iterBFS->hasNext())
		dynamic_cast<const RuntimeComponent*>(&iterBFS->getNext())->print();
	return 0;
}
