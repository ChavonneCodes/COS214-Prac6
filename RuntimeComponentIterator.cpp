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

template <typename T>
void DFSTraversalStrategy<T>::push(const T* component)
{
	components.push(component);
}

template <typename T>
const T* DFSTraversalStrategy<T>::pop()
{
	const T* top = components.top();
	components.pop();
	return top;
}

template <typename T>
bool DFSTraversalStrategy<T>::isEmpty() const
{
	return components.empty();
}
