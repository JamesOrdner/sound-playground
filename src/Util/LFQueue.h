#pragma once

#include <atomic>
#include <memory>

// LFQueue provides a single-writer, single-reader lock-free queue.
// Pushing to the queue will allocate and potentially free memory,
// but reading from the queue will not dynamically allocate any
// memory, providing bounded execution time.
template<typename T>
class LFQueue
{
public:
	
	LFQueue()
	{
		begin = current = end = new Node(T());
	}

	~LFQueue()
	{
		while (begin != nullptr) {
			Node* tmp = begin;
			begin = tmp->next;
			delete tmp;
		}
	}

	void push(const T& val)
	{
		Node* node = end.load()->next = new Node(val);
		end.store(node);

		while (begin != current) { // cleanup
			Node* tmp = begin;
			begin = begin->next;
			delete tmp;
		}
	}

	bool pop(T& val)
	{
		if (current != end) {
			Node* next = current.load()->next;
			val = next->value;
			current.store(next);
			return true;
		}

		return false;
	}

private:

	struct Node
	{
		Node(T val) : value(val), next(nullptr) {}
		T value;
		Node* next;
	};

	Node* begin;
	std::atomic<Node*> current, end;
};
