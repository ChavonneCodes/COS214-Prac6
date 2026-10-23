#ifndef ITERATOR_H
#define ITERATOR_H

#include <stack>
#include <queue>

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
class DFSTraversalStrategy : public TraversalStrategy<T>
{
	private:
		stack<const T*> components;
	public:
		virtual void push(const T* comp) override
		{
			components.push(comp);
		}
		virtual const T* pop() override
		{
			const T* top = components.top();
			components.pop();
			return top;
		}
		virtual bool isEmpty() const override
		{
			return components.empty();
		}
};

template <typename T>
class BFSTraversalStrategy : public TraversalStrategy<T>
{
        private:
                queue<const T*> components;
        public:
                virtual void push(const T* comp) override
                {
                        components.push(comp);
                }
                virtual const T* pop() override
                {
                        const T* front = components.front();
                        components.pop();
                        return front;
                }
                virtual bool isEmpty() const override
                {
                        return components.empty();
                }
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
