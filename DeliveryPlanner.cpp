#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_street;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm):m_street(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    vector<StreetSegment> segs;
    cout << depot.latitudeText << " , " << depot.longitudeText << endl;
    if (!m_street->getSegmentsThatStartWith(depot, segs))
    {
        return BAD_COORD;
    }
    double oldCD;
    double newCD;
    double miles = 0;
    double totalMiles = 0;
    DeliveryOptimizer opt(m_street);
    PointToPointRouter route(m_street);
    vector<DeliveryRequest> optDelivs;
    vector<DeliveryCommand> comm_vec;
    DeliveryCommand comm;
    vector<list<StreetSegment>> paths;
    list<StreetSegment> temp;
    list<StreetSegment> pathBack;

    for (int i = 0; i < deliveries.size(); i++)
    {
        optDelivs.push_back(deliveries[i]);
    }
    cout << optDelivs.size() << endl;

    vector<StreetSegment> segs2;
    for (int i = 0; i < optDelivs.size(); i++)
    {
        if (!m_street->getSegmentsThatStartWith(optDelivs[i].location, segs2))
        {
            cout << optDelivs[i].location.latitude << " , " << optDelivs[i].location.longitude << endl;
            return NO_ROUTE;
        }
    }

    cout << "pass" << endl;
    bool proceed = true;
    opt.optimizeDeliveryOrder(depot, optDelivs, oldCD, newCD);
    route.generatePointToPointRoute(depot, optDelivs[0].location, temp, miles);
    paths.push_back(temp);

    if (optDelivs.empty())
    {
        return NO_ROUTE;
    }

    for (int i = 0; i < optDelivs.size() - 1; i++)
    {
        list<StreetSegment> mid;
        route.generatePointToPointRoute(optDelivs[i].location, optDelivs[i + 1].location, mid, miles);
        totalMiles += miles;
        paths.push_back(mid);
        //generate path back
        if (i == optDelivs.size() - 2)
        {
            route.generatePointToPointRoute(optDelivs[i+1].location, depot, mid, miles);
            totalMiles += miles;
            paths.push_back(mid);
        }
    }

    for (int i = 0; i < paths.size(); i++)
    {
        for (auto it = paths[i].begin(); it != paths[i].end(); it++)
        {

        }
    }
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}

