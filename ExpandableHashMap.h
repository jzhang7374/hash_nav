// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#ifndef HASHMAP
#define HASHMAP

#include <list> // ?
#include "provided.h"

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	struct Node
	{
		Node(KeyType k, ValueType v)
		{
			this->myKey = k;
			this->myVal = v;
		}
		int getKey() { return myKey; };
		int getVal() { return myVal; };
		KeyType myKey;
		ValueType myVal;
	};
	unsigned int hasher(const GeoCoord& g)
	{
		return std::hash<std::string>()(g.latitudeText + g.longitudeText);
	}
	std::list<Node> *m_table;
	double m_load;
	int m_size = 8;
	int m_filled = 0;
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
	m_table = new std::list<Node>[m_size];
	if (maximumLoadFactor > 0)
	{
		m_load = maximumLoadFactor;
	}
	else
		m_load = 0.5;
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	for (int i = 0; i < m_size; i++)
	{
		if (m_table[i] != nullptr)
			delete m_table[i];
	}
	delete[] m_table;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	for (int i = 0; i < m_size; i++)
	{
		if (m_table[i] != nullptr)
			delete m_table[i];
	}
	m_size = 8;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_size;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	int index = hasher(key);
	Node* toPush = new Node(key, value);
	m_table[index].push_back(toPush);
	
	/*if (m_filled / m_size > m_load)
	{
		for (int i = 0; i < m_size; i++)
		{
			m_size *= 2;
			if (m_table[i] != nullptr)
				delete m_table[i];
		}
	}*/ //how to implement
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	return nullptr;  // Delete this line and implement this function correctly
}

#endif
