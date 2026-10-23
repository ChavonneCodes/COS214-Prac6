#include "Iterator.h"
#include "RuntimeComponent.h"

RuntimeComponentIterator::RuntimeComponentIterator(const RuntimeComponent& component)
{
	const auto& children = component.getChildren();
	for (auto it = children.rbegin(); it != children.rend(); it++)
		components.push(*it);
}

bool RuntimeComponentIterator::hasNext() const
{
	return !components.empty();
}

const Iterable& RuntimeComponentIterator::getNext()
{
	const auto& child = components.top();
	components.pop();

	const auto& children = child->getChildren();
	for (auto it = children.rbegin(); it != children.rend(); it++)
		components.push(*it);
	return *child;
}
