#pragma once
#include <Functions/MemoryPool/MemoryPool.h>
#include <future>
#include <functional>

namespace SERVER {
	namespace FUNCTIONS {
		namespace CIRCULARLINKEDLIST {
			template<typename T>
			struct NodeData : public SERVER::FUNCTIONS::MEMORYMANAGER::MemoryManager<NodeData<T>, 100> {
			public:
				T m_nodeData;

				NodeData* m_pNextNode;
				NodeData* m_pPrevNode;

			public:
				NodeData() : m_nodeData(), m_pNextNode(nullptr), m_pPrevNode(nullptr) {};
				NodeData(T&& newData) : m_nodeData(std::move(newData)), m_pNextNode(nullptr), m_pPrevNode(nullptr) {};
				NodeData(const T& newData) : m_nodeData(newData), m_pNextNode(nullptr), m_pPrevNode(nullptr) {};

				T& operator*() { return m_nodeData; }

			};

			template<typename T>
			class CircularLinkedList {
				using node_type = NodeData<T>;

				struct iterator : std::iterator<std::input_iterator_tag, T> {
				public:
					node_type* m_pPtr;

				public:
					iterator(node_type* pPtr) : m_pPtr(pPtr) {};

					iterator& operator++() { m_pPtr = m_pPtr->m_pNextNode; return (*this); };
					iterator operator++(int) { return (*this).operator++(); };

					T& operator*() { return m_pPtr->m_nodeData; }

					bool operator==(iterator other) { return m_pPtr == other.m_pPtr; };
					bool operator!=(iterator other) { return m_pPtr != other.m_pPtr; };

				};

			public:
				CircularLinkedList() : m_pHead(new node_type()), m_pTail(m_pHead) {};
				~CircularLinkedList() {
					delete m_pHead;
				}

			public:
				void push_back(T&& newData) {
					auto pNewNodeData = new node_type(std::move(newData));

					if (!m_pTail->m_pNextNode) {
						pNewNodeData->m_pNextNode = m_pHead;
						pNewNodeData->m_pPrevNode = m_pHead;
						m_pHead->m_pNextNode = pNewNodeData;
						m_pHead->m_pPrevNode = pNewNodeData;
					}
					else {
						pNewNodeData->m_pNextNode = m_pTail->m_pNextNode;
						pNewNodeData->m_pPrevNode = m_pTail;
						m_pTail->m_pNextNode->m_pPrevNode = pNewNodeData;
						m_pTail->m_pNextNode = pNewNodeData;
					}
					m_pTail = pNewNodeData;
				}

				bool pop_front(T& outData) {
					if (!m_pHead->m_pNextNode || m_pHead == m_pHead->m_pNextNode) return false;

					auto pPopNode = m_pHead->m_pNextNode;

					m_pHead->m_pNextNode = pPopNode->m_pNextNode;
					pPopNode->m_pNextNode->m_pPrevNode = m_pHead;

					outData = pPopNode->m_nodeData;
					delete pPopNode;
				}

				bool pop_back(T& outData) {
					if (m_pHead == m_pTail) return false;

					auto pPopNode = m_pTail;

					m_pTail->m_pPrevNode->m_pNextNode = m_pHead;
					m_pHead->m_pPrevNode = m_pTail->m_pPrevNode;
					m_pTail = m_pTail->m_pPrevNode;

					outData = pPopNode->m_nodeData;
					delete pPopNode;
				}

				bool erase(iterator& it) {
					auto pCachedNodePtr = it.m_pPtr;
					if (!pCachedNodePtr) return false;

					if (pCachedNodePtr == m_pTail)
						m_pTail = pCachedNodePtr->m_pPrevNode;

					auto pNextNode = pCachedNodePtr->m_pNextNode;
					pCachedNodePtr->m_pNextNode->m_pPrevNode = pCachedNodePtr->m_pPrevNode;
					pCachedNodePtr->m_pPrevNode->m_pNextNode = pNextNode;
					pCachedNodePtr->m_pNextNode = pCachedNodePtr->m_pPrevNode;
					pCachedNodePtr->m_pPrevNode = pNextNode;


					delete pCachedNodePtr;

					return true;
				}

				bool is_empty() {
					return m_pHead->m_pNextNode == nullptr || m_pHead->m_pNextNode == m_pHead;
				}

				iterator begin() {
					return iterator(m_pHead->m_pNextNode);
				}

				iterator end() {
					return iterator(m_pTail->m_pNextNode);
				}

			private:
				node_type* m_pHead;
				node_type* m_pTail;


			};
		}
	}
}