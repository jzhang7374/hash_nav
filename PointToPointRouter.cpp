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
		GeoNode(GeoNode* parentGeo, const GeoCoord& current, const GeoCoord& follow) : parent(parentGeo),curr(current),fol(follow) 
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
	vector<GeoNode> getNeighbors(GeoNode* current) const {};
	list<StreetSegment> determineRoute(GeoNode* node) const {};
	void removePtrs(set<GeoNode*,GeoNode_set> geo, list<GeoNode*>) const {};
	const StreetMap* m_street;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm): m_street(sm)
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

	GeoNode* begNode = new GeoNode(nullptr, start, end);
	openList.insert(begNode);

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
