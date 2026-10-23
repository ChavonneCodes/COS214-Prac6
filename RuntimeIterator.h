#ifndef ITERATOR_H
#define ITERATOR_H

#include "RuntimeComponent.h"

class Iterator
{
	public:
		virtual bool hasNext() = 0;
		virtual RuntimeComponent* next() = 0;
		virtual void reset() = 0;
}

class RuntimeIterator
{
	public:
		bool hasNext() override;
		RuntimeComponent* next() override;
		void reset() override;
}

#endif
