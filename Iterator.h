#ifndef ITERATOR_H
#define ITERATOR_H

#include <stack>

using namespace std;

template <typename T>
class Iterator;

template <typename T>
class TraversalStrategy;

template <typename T>
class Iterable
{
	public:
		virtual Iterator<T>* createIterator(TraversalStrategy<T>* strat) const = 0;
		virtual ~Iterable() = default;
};

template <typename T>
class Iterator
{
	public:
		virtual bool hasNext() const = 0;
		virtual const Iterable<T>& getNext() = 0;
		virtual ~Iterator() = default;
};

template <typename T>
class TraversalStrategy
{
	public:
		virtual void push(const T*) = 0;
		virtual const T* pop() = 0;
		virtual bool isEmpty() const = 0;
		virtual ~TraversalStrategy() = default;
};

template <typename T>
class DFSTraversalStrategy
{
	private:
		stack<const T*> components;
	public:
		virtual void push(const T*) override;
		virtual const T* pop() override;
		virtual bool isEmpty() const override;
};

class RuntimeComponent;

class RuntimeComponentIterator : public Iterator<RuntimeComponent>
{
	private:
		TraversalStrategy<RuntimeComponent>* strat;
	public:
		RuntimeComponentIterator(const RuntimeComponent&, TraversalStrategy<RuntimeComponent>* strat);
		bool hasNext() const override;
		const Iterable<RuntimeComponent>& getNext() override;
};

#endif
