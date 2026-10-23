#include "Iterator.h"

RuntimeComponentIterator::RuntimeComponentIterator(const RuntimeComponent& component)
{
	const auto& children = component.getChildren();
	for (auto it = children.rbegin(); it != children.rend(); it++)
	{
		components.push(&(*it));
	}
}

bool RuntimeComponent::hasNext()
{
	return !components.empty();
}

blah
