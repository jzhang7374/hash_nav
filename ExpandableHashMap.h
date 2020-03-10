// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#include <iostream>
#include <list>
#include <vector>
#include <utility>

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
	int m_load;
	int m_associations;
	std::vector<std::list<std::pair<KeyType, ValueType>>> m_table;
	unsigned int hashVal(const KeyType& key) const
	{
		unsigned int hasher(const KeyType & key);
		unsigned int h = hasher(key);
		return h;
	}
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor) :m_table(8)
{
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
	m_table.clear();
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	m_table.clear();
	m_table.resize(8);
	m_associations = 0;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_associations;  // returns number of associations
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	bool loadCap = false;
	int index = hashVal(key);
	for (auto& [k, v] : m_table[index])
	{
		if (k == key)//if key exists reset value
		{
			v = value;
			return;
		}
	}

	//std::cerr << key << "/" << value << std::endl;

	m_table[index].emplace_back(key, value); //insert new pair
	m_associations++;
	if (m_associations / m_table.size() > m_load) //if load is exceeded create a temp that is 2x current table size and fill it
	{
		std::vector<std::list<std::pair<KeyType, ValueType>>> temp(m_table.size() * 2);
		m_table.swap(temp);
		for (std::list<std::pair<KeyType, ValueType>>& bucket : temp)
		{
			for (auto& [k, v] : bucket)
			{
				int index = hashVal(key);
				m_table[index].emplace_back(key, value);
			}
		}
	}
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	int index = hashVal(key);
	for (auto& [k, v] : m_table[index]) //return address to value of given key if found
	{
		if (k == key)
		{
			return&v;
		}
	}
	return nullptr;
}

//streetmap: expandablehashmap<geocoord>,vector<streetseg>> hashmap