#include "Iterator.h"
#include "StageInstance.h"

int main()
{
	StageInstance stageInstance("id", new StageDefinition("id", "name"));
	StageInstance stageInstance1("id", new StageDefinition("id", "name"));
	stageInstance.addChild(&stageInstance1);
}
