#include "provided.h"
#include <vector>
#include <iterator>
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
    const StreetMap* street;
    string calculatedirection(StreetSegment street) const
    {
        string str = "90";
        return str;
    };
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm):street(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(const GeoCoord& depot, const vector<DeliveryRequest>& deliveries, vector<DeliveryCommand>& commands, double& totalDistanceTravelled) const
{
    double oldcd;
    double newcd;
    vector<StreetSegment> streetsegs;
    if (!street->getSegmentsThatStartWith(depot, streetsegs)) { //check if depot location is valid
        return BAD_COORD;
    }
    double totalMILES = 0;
    double totalmiles;
    DeliveryOptimizer s(street);
    PointToPointRouter r(street);
    vector<list<StreetSegment>> generatedpaths;
    vector<DeliveryRequest> delivery;
    vector<DeliveryCommand> routes;
    list<StreetSegment> pointtopointroutes;
    DeliveryCommand proc;
    for (int i = 0; i < deliveries.size(); i++) {       //put deliveries into a delivery vector
        delivery.push_back(deliveries[i]);
    }
    /*for (int i = 0; i < delivery.size(); i++) {         //check if all delivery locations are valid
        if (!street->getSegmentsThatStartWith(delivery[i].location, streetsegs)) {
            cerr << "here" << endl;
            return NO_ROUTE;
        }
    }*/
    s.optimizeDeliveryOrder(depot, delivery, oldcd, newcd);
    bool depotnotdelivery = false;
    if (depot == delivery[0].location) {
        depotnotdelivery = true;
        proc.initAsDeliverCommand(delivery[0].item);
        routes.push_back(proc);
    }
    list<StreetSegment> start;
    list<StreetSegment> temp;
    bool skipturn = false;
    int count = 0;

    if (r.generatePointToPointRoute(depot, delivery[0].location, start, totalmiles) == BAD_COORD) { return NO_ROUTE; };

    r.generatePointToPointRoute(depot, delivery[0].location, start, totalmiles);  //depot to first delivery location
    totalMILES += totalmiles;
    if (!depotnotdelivery) {    //if depot does not start at the delivery location
        for (auto it = start.begin(); it != start.end(); it++) {    //go through street segments and create commands
            if (next(it) != start.end()) {
                if (it->name == next(it)->name) {     //if current and next street segment have same name
                    GeoCoord startpoint = it->start;
                    string name = it->name;
                    string direction = calculatedirection(*it);
                    /*count++;
                    cout << count << endl;
                    cout << start.size() << endl;*/
                    while (it->name == next(it)->name)
                    {       //loop until next street is not same name
                        it++;
                        if(next(it) == start.end())
                            break;
                        /*cout << it->name << endl;
                        cout << it->start.latitude << "," << it->start.longitude << endl;
                        cout << it->end.latitude << "," << it->end.longitude << endl;*/
                    }                                               //generate proceed command
                    proc.initAsProceedCommand(direction, name, distanceEarthMiles(startpoint, it->end));
                    routes.push_back(proc);
                    it--;
                }
                else if (it != start.begin()) {       //if it's not at the start, can turn
                    if (angleBetween2Lines(*it, *next(it)) < 1 || angleBetween2Lines(*it, *next(it)) > 359) {
                        string direction = calculatedirection(*next(it));
                        proc.initAsProceedCommand(direction, next(it)->name, totalmiles);
                        routes.push_back(proc);
                    }
                    else if (angleBetween2Lines(*it, *next(it)) >= 1 || angleBetween2Lines(*it, *next(it)) < 180) {
                        proc.initAsTurnCommand("left", next(it)->name);       //generate left direction
                        routes.push_back(proc);
                    }
                    else if (angleBetween2Lines(*it, *next(it)) >= 180 || angleBetween2Lines(*it, *next(it)) <= 359) {
                        proc.initAsTurnCommand("right", next(it)->name);      //generate right direction
                        routes.push_back(proc);
                    }
                }
                else {        //otherwise just generate a proceed command
                    string direction = calculatedirection(*it);
                    proc.initAsProceedCommand(direction, it->name, distanceEarthMiles(it->start, it->end));
                    routes.push_back(proc);
                }
            }
            else {
                skipturn = true;
                proc.initAsDeliverCommand(delivery[0].item);
                routes.push_back(proc);
            }
        }
    }
    for (int i = 0; i < delivery.size(); i++) {     //generate routes for each delivery location
        if (i + 1 != delivery.size()) {
            r.generatePointToPointRoute(delivery[i].location, delivery[i + 1].location, pointtopointroutes, totalmiles);
            generatedpaths.push_back(pointtopointroutes);
            totalMILES += totalmiles;
        }
        else {        //generates the delivery location to go back
            r.generatePointToPointRoute(delivery[i].location, depot, temp, totalmiles);
            totalMILES += totalmiles;
        }
    }
    for (int i = 0; i < generatedpaths.size(); i++) {       //loops through to generate street commands
        for (auto it = generatedpaths[i].begin(); it != generatedpaths[i].end(); it++) {
            if (next(it) != generatedpaths[i].end()) {
                if (it->name == next(it)->name) {       //if same street name, then continue until it is not
                    GeoCoord startpoint = it->start;
                    string name = it->name;
                    string direction = calculatedirection(*it);
                    while (it->name == next(it)->name) 
                    {
                        it++;
                        if (next(it) == generatedpaths[i].end())
                            break;
                    }                                           //then generate the proceed command
                    proc.initAsProceedCommand(direction, name, distanceEarthMiles(startpoint, it->end));
                    skipturn = false;
                    routes.push_back(proc);
                    it--;
                }
                else {
                    if (!skipturn) {                         //if delivery occurred before, don't turn
                        if (angleBetween2Lines(*it, *next(it)) < 1 || angleBetween2Lines(*it, *next(it)) > 359) {
                            string direction = calculatedirection(*next(it));
                            proc.initAsProceedCommand(direction, next(it)->name, totalmiles);
                            routes.push_back(proc);
                        }
                        else if (angleBetween2Lines(*it, *next(it)) >= 1 || angleBetween2Lines(*it, *next(it)) < 180) {
                            proc.initAsTurnCommand("left", next(it)->name);
                            routes.push_back(proc);
                        }
                        else if (angleBetween2Lines(*it, *next(it)) >= 180 || angleBetween2Lines(*it, *next(it)) <= 359) {
                            proc.initAsTurnCommand("right", next(it)->name);
                            routes.push_back(proc);
                        }
                    }
                }
            }
            else if (i + 1 != delivery.size()) {      //deliver the item at the end
                skipturn = true;
                proc.initAsDeliverCommand(delivery[i + 1].item);
                routes.push_back(proc);
            }
        }
    }
    for (auto it = temp.begin(); it != temp.end(); it++) {   //loop through to generate commands on way back
        if (next(it) != temp.end()) {
            if (it->name == next(it)->name) {           //check if street names are the same
                GeoCoord startpoint = it->start;
                string name = it->name;
                string direction = calculatedirection(*it);
                while (it->name == next(it)->name) {
                    it++;
                    if (next(it) == temp.end())
                        break;
                }                                               //generate the proceed command
                proc.initAsProceedCommand(direction, name, distanceEarthMiles(startpoint, it->end));
                routes.push_back(proc);
                it--;
            }
            else {        //different name, generate turn or proceed(if angle is below or above certain amount)
                if (angleBetween2Lines(*it, *next(it)) < 1 || angleBetween2Lines(*it, *next(it)) > 359) {
                    string direction = calculatedirection(*next(it));
                    proc.initAsProceedCommand(direction, next(it)->name, totalmiles);
                    routes.push_back(proc);
                }
                else if (angleBetween2Lines(*it, *next(it)) >= 1 || angleBetween2Lines(*it, *next(it)) < 180) {
                    proc.initAsTurnCommand("left", next(it)->name);
                    routes.push_back(proc);
                }
                else if (angleBetween2Lines(*it, *next(it)) >= 180 || angleBetween2Lines(*it, *next(it)) <= 359) {
                    proc.initAsTurnCommand("right", next(it)->name);
                    routes.push_back(proc);
                }
            }
        }
    }
    commands = routes;
    totalDistanceTravelled = totalMILES;
    return DELIVERY_SUCCESS;

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

