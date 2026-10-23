/**
 * @file Iterator.h
 * @brief Header file defining template interfaces and concrete classes for 
 *        the Iterator and Strategy design patterns.
 */

#ifndef ITERATOR_H
#define ITERATOR_H

#include <stack>
#include <queue>

using namespace std;

// Forward declarations
template <typename T>
class Iterator;

template <typename T>
class TraversalStrategy;

/**
 * @brief Interface for objects that can produce an Iterator.
 * 
 * @tparam T The underlying component type being traversed.
 */
template <typename T>
class Iterable
{
	public:
		/**
		 * @brief Creates a new Iterator using the specified traversal strategy.
		 * 
		 * @param strat Pointer to the traversal strategy algorithm (e.g., DFS or BFS).
		 * @return Pointer to a dynamically allocated Iterator. Caller is responsible for memory.
		 */
		virtual Iterator<T>* createIterator(TraversalStrategy<T>* strat) const = 0;

		/**
		 * @brief Virtual destructor for safe polymorphic cleanup.
		 */
		virtual ~Iterable() = default;
};

/**
 * @brief Interface for iterating over a collection of Iterable elements.
 * 
 * @tparam T The underlying component type.
 */
template <typename T>
class Iterator
{
	public:
		/**
		 * @brief Checks if there are more elements left to iterate over.
		 * 
		 * @return true If another element exists.
		 * @return false If traversal is complete.
		 */
		virtual bool hasNext() const = 0;

		/**
		 * @brief Advances the iterator and returns the next element.
		 * 
		 * @return const Iterable<T>& Reference to the next element.
		 */
		virtual const Iterable<T>& getNext() = 0;

		/**
		 * @brief Virtual destructor for safe polymorphic cleanup.
		 */
		virtual ~Iterator() = default;
};

/**
 * @brief Abstract strategy interface defining how nodes are buffered and retrieved during iteration.
 * 
 * @tparam T The underlying component type stored in the strategy buffer.
 */
template <typename T>
class TraversalStrategy
{
	public:
		/**
		 * @brief Adds a node pointer to the traversal buffer.
		 * 
		 * @param comp Pointer to the component to insert.
		 */
		virtual void push(const T* comp) = 0;

		/**
		 * @brief Removes and returns the next node pointer according to the strategy rule.
		 * 
		 * @return const T* Pointer to the next component.
		 */
		virtual const T* pop() = 0;

		/**
		 * @brief Checks whether the strategy buffer is empty.
		 * 
		 * @return true If there are no components left in the buffer.
		 * @return false If there are components remaining.
		 */
		virtual bool isEmpty() const = 0;

		/**
		 * @brief Virtual destructor for safe polymorphic cleanup.
		 */
		virtual ~TraversalStrategy() = default;
};

/**
 * @brief Depth-First Search (DFS) implementation of TraversalStrategy using LIFO (std::stack).
 * 
 * @tparam T The underlying component type.
 */
template <typename T>
class DFSTraversalStrategy : public TraversalStrategy<T>
{
	private:
		stack<const T*> components; ///< LIFO stack storing node pointers during traversal.

	public:
		/**
		 * @brief Pushes a component onto the top of the stack.
		 * 
		 * @param comp Pointer to the component.
		 */
		virtual void push(const T* comp) override
		{
			components.push(comp);
		}

		/**
		 * @brief Pops and returns the component from the top of the stack.
		 * 
		 * @return const T* Pointer to the popped component.
		 */
		virtual const T* pop() override
		{
			const T* top = components.top();
			components.pop();
			return top;
		}

		/**
		 * @brief Checks if the stack is empty.
		 * 
		 * @return true If stack is empty.
		 * @return false If stack contains items.
		 */
		virtual bool isEmpty() const override
		{
			return components.empty();
		}
};

/**
 * @brief Breadth-First Search (BFS) implementation of TraversalStrategy using FIFO (std::queue).
 * 
 * @tparam T The underlying component type.
 */
template <typename T>
class BFSTraversalStrategy : public TraversalStrategy<T>
{
	private:
		queue<const T*> components; ///< FIFO queue storing node pointers during traversal.

	public:
		/**
		 * @brief Pushes a component onto the back of the queue.
		 * 
		 * @param comp Pointer to the component.
		 */
		virtual void push(const T* comp) override
		{
			components.push(comp);
		}

		/**
		 * @brief Removes and returns the component from the front of the queue.
		 * 
		 * @return const T* Pointer to the front component.
		 */
		virtual const T* pop() override
		{
			const T* front = components.front();
			components.pop();
			return front;
		}

		/**
		 * @brief Checks if the queue is empty.
		 * 
		 * @return true If queue is empty.
		 * @return false If queue contains items.
		 */
		virtual bool isEmpty() const override
		{
			return components.empty();
		}
};

class RuntimeComponent;

/**
 * @brief Concrete iterator for traversing RuntimeComponent tree structures.
 */
class RuntimeComponentIterator : public Iterator<RuntimeComponent>
{
	private:
		TraversalStrategy<RuntimeComponent>* strat; ///< Pointer to the strategy driving traversal order.

	public:
		/**
		 * @brief Constructs a RuntimeComponentIterator.
		 * 
		 * @param root Reference to the root RuntimeComponent to start iteration from.
		 * @param strat Pointer to the traversal strategy algorithm instance to use.
		 */
		RuntimeComponentIterator(const RuntimeComponent& root, TraversalStrategy<RuntimeComponent>* strat);

		/**
		 * @brief Checks if more elements exist in the strategy buffer.
		 * 
		 * @return true If elements remain.
		 * @return false If traversal finished.
		 */
		bool hasNext() const override;

		/**
		 * @brief Returns the next RuntimeComponent in traversal order and buffers its children.
		 * 
		 * @return const Iterable<RuntimeComponent>& Reference to the next element.
		 */
		const Iterable<RuntimeComponent>& getNext() override;
};

#endif // ITERATOR_H
