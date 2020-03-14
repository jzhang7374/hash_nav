#include "provided.h"
#include <list>
#include <set>
#include <vector>
using namespace std;

class PointToPointRouterImpl
{
public:
	PointToPointRouterImpl(const StreetMap* sm);
	~PointToPointRouterImpl();
	DeliveryResult generatePointToPointRoute(
		const GeoCoord& start,
		const GeoCoord& end,
		list<StreetSegment>& route,
		double& totalDistanceTravelled) const;
private:
	struct GeoNode
	{
		GeoNode(GeoNode* parentGeo, const GeoCoord& current, const GeoCoord& follow) : parent(parentGeo), curr(current), fol(follow)
		{
			if (parent == nullptr)
				gCost = 0;
			else
				gCost = parent->gCost + distanceEarthMiles(parent->curr, curr);
			hCost = distanceEarthMiles(curr, fol);
		}
		GeoNode* parent;
		GeoCoord curr;
		GeoCoord fol;
		double gCost;
		double hCost;
		double fCost() const
		{
			return gCost + hCost;
		}

	};
	struct GeoNode_set
	{
		bool operator() (const GeoNode* geo1, const GeoNode* geo2) const
		{
			if (geo1 == geo2)
				return false;

			return geo1->fCost() < geo2->fCost();
		}
	};
	vector<GeoNode> getNeighbors(GeoNode* current) const;
	list<StreetSegment> determineRoute(GeoNode* node) const;
	void removePtrs(set<GeoNode*, GeoNode_set> geo, list<GeoNode*> list) const;
	const StreetMap* m_street;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) : m_street(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	set<GeoNode*, GeoNode_set> openList;
	list <GeoNode*> closedList;
	double distance = 0;

	GeoNode* begNode = new GeoNode(nullptr, start, end);
	openList.insert(begNode);

	vector<StreetSegment> segments;
	if (!m_street->getSegmentsThatStartWith(start, segments) || !m_street->getSegmentsThatStartWith(end, segments))
	{
		return BAD_COORD;
		totalDistanceTravelled = 0;
	}

	if (start == end)
	{
		return DELIVERY_SUCCESS;
		totalDistanceTravelled = 0;
	}
	
	while (!openList.empty())
	{
		GeoNode* currentNode = *openList.begin();
		openList.erase(openList.begin());

		vector<GeoNode> neighbors = getNeighbors(currentNode);

		for (GeoNode neighbor : neighbors)
		{
			if (neighbor.curr == end)
			{
				route = determineRoute(&neighbor);
				for (auto it = route.begin(); it != route.end(); it++)
				{
					distance += distanceEarthMiles((*it).start, (*it).end);
				}
				totalDistanceTravelled = distance;
				delete currentNode;
				removePtrs(openList, closedList);
				return DELIVERY_SUCCESS;
			}

			bool ignoreNeighbor = false;
			for (GeoNode* node : openList)
			{
				if (node->curr == neighbor.curr && node->fCost() <= neighbor.fCost())
				{
					ignoreNeighbor = true;
					break;
				}
			}

			for (GeoNode* node : closedList)
			{
				if (node->curr == neighbor.curr && node->fCost() <= neighbor.fCost())
				{
					ignoreNeighbor = true;
					break;
				}
			}

			if (ignoreNeighbor)
				continue;
			else
				openList.insert(new GeoNode(neighbor.parent, neighbor.curr, neighbor.fol));
		}
		closedList.push_back(currentNode);
	}
	removePtrs(openList, closedList);
	return NO_ROUTE;  // Delete this line and implement this function correctly
}

vector<PointToPointRouterImpl::GeoNode> PointToPointRouterImpl::getNeighbors(GeoNode* current) const
{
	vector<StreetSegment> segments;
	vector<GeoNode> nodes;
	m_street->getSegmentsThatStartWith(current->curr, segments);
	for (int i = 0; i < segments.size(); i++)
	{
		GeoNode geo(current, segments[i].end, current->fol);
		nodes.push_back(geo);
	}
	return nodes;
	//create new child nodes
	//pass *parent is the current geonode, start is end coord of any street segments, target is end
}

list<StreetSegment> PointToPointRouterImpl::determineRoute(GeoNode* node) const 
{
	//use get segments that start with
	//node.curr
	//pass that into segments that start with
	list<StreetSegment> route;

	while(node->parent != nullptr)
	{
		vector<StreetSegment> parentSegs;
		m_street->getSegmentsThatStartWith(node->parent->curr, parentSegs);

		for (StreetSegment seg : parentSegs)
		{
			if (seg.end == node->curr && seg.start == node->parent->curr)
			{
				route.push_front(seg);
				node = node->parent;
			}
		}
	}
	return route;
}

void PointToPointRouterImpl::removePtrs(set<GeoNode*, GeoNode_set> geo, list<GeoNode*> geoList) const
{
	for (set<GeoNode*, GeoNode_set>::iterator it = geo.begin(); it != geo.end(); it++)
	{
		delete* it;
	}
	for (list<GeoNode*>::iterator it = geoList.begin(); it != geoList.end(); it++)
	{
		delete* it;
	}
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
	m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
	delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}