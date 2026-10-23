#include "Iterator.h"
#include "RuntimeComponent.h"

RuntimeComponentIterator::RuntimeComponentIterator(const RuntimeComponent& component, TraversalStrategy<RuntimeComponent>* strat)
	: strat(strat)
{
	const auto& children = component.getChildren();
	for (auto it = children.rbegin(); it != children.rend(); it++)
		strat->push(*it);
}

bool RuntimeComponentIterator::hasNext() const
{
	return !strat->isEmpty();
}

const Iterable<RuntimeComponent>& RuntimeComponentIterator::getNext()
{
	const auto& child = strat->pop();
	const auto& children = child->getChildren();
	for (auto it = children.rbegin(); it != children.rend(); it++)
		strat->push(*it);
	return *child;
}
