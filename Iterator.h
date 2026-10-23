#ifndef ITERATOR_H
#define ITERATOR_H

#include "RuntimeComponent.h"

using namespace std;

class Iterator;

class Iterable
{
	public:
		virtual Iterator* createIterator() const = 0;
};

class Iterator
{
	public:
		virtual bool hasNext() const = 0;
		virtual const Iterable* getNext() = 0;
};

class RuntimeComponent;
class RuntimeComponentIterator : public Iterator
{
	private:
		const RuntimeComponent& component;
	public:
		RuntimeComponentIterator(const RuntimeComponent& component) : component(component) {}
		bool hasNext() const override;
		const Iterable* getNext() override;
};

#endif
