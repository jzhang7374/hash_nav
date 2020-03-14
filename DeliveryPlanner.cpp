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
    string calculatedirection(const StreetSegment& street) const
    {
        double angle = angleOfLine(street);
        string str;
        if (angle >= 0 && angle < 22.5)
            str = "east";
        else if (angle >= 22.5 && angle < 67.5)
            str = "northeast";
        else if (angle >= 67.5 && angle < 112.5)
            str = "north";
        else if (angle >= 112.5 && angle < 157.5)
            str = "northwest";
        else if (angle >= 157.5 && angle < 202.5)
            str = "west";
        else if (angle >= 202.5 && angle < 247.5)
            str = "southwest";
        else if (angle >= 247.5 && angle < 292.5)
            str = "south";
        else if (angle >= 292.5 && angle < 337.5)
            str = "southeast";
        else if (angle > 337.5)
            str = "east";

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
    vector<StreetSegment> segs;
    if (!street->getSegmentsThatStartWith(depot, segs)) //checks if depot location is valid
    { 
        return BAD_COORD;
    }

    double oldCD;
    double newCD;
    double totalMiles = 0;
    double miles;
    DeliveryOptimizer opt(street);
    PointToPointRouter ptp(street);
    vector<list<StreetSegment>> delivRoutes;
    vector<DeliveryRequest> toDeliv;
    vector<DeliveryCommand> commsVec;
    list<StreetSegment> ptproutes;
    DeliveryCommand comm;

    for (int i = 0; i < deliveries.size(); i++)  //puts deliveries into a vector
    {      
        toDeliv.push_back(deliveries[i]);
    }

    for (int i = 0; i < toDeliv.size(); i++) //makes sure deliveries are valid
    {         
        if (!street->getSegmentsThatStartWith(toDeliv[i].location, segs)) 
        {
            cerr << "here" << endl;
            return BAD_COORD;
        }
    }
    opt.optimizeDeliveryOrder(depot, toDeliv, oldCD, newCD);
    bool depotOverlap = false;
    if (depot == toDeliv[0].location) 
    {
        depotOverlap = true;
        comm.initAsDeliverCommand(toDeliv[0].item);
        commsVec.push_back(comm);
    }

    list<StreetSegment> first;
    list<StreetSegment> temp;
    bool noTurn = false;
    //int count = 0;

    if (ptp.generatePointToPointRoute(depot, toDeliv[0].location, first, miles) == NO_ROUTE) //depot to first delivery location
    {
        return NO_ROUTE;
    }
    totalMiles += miles;

    if (!depotOverlap) //if depot doesnt start at delivery spot
    {    
        for (auto it = first.begin(); it != first.end(); it++) //go through street segs and create commands
        {    
            if (next(it) != first.end()) 
            {
                if (it->name == next(it)->name) //if street segments on the same street
                {     
                    GeoCoord start = it->start;
                    string name = it->name;
                    string dir = calculatedirection(*it);
                    /*count++;
                    cout << count << endl;
                    cout << start.size() << endl;*/
                    while (it->name == next(it)->name)
                    {       //loop until next street is not same name
                        it++;
                        if(next(it) == first.end())
                            break;
                        /*cout << it->name << endl;
                        cout << it->start.latitude << "," << it->start.longitude << endl;
                        cout << it->end.latitude << "," << it->end.longitude << endl;*/
                    }                                               //generate proceed command
                    comm.initAsProceedCommand(dir, name, distanceEarthMiles(start, it->end));
                    commsVec.push_back(comm);
                    it--;
                }

                else if (it != first.begin()) //allowed to turn after first segment
                {       
                    if (angleBetween2Lines(*it, *next(it)) < 1 || angleBetween2Lines(*it, *next(it)) > 359) //proceed command
                    {
                        string dir = calculatedirection(*next(it));
                        comm.initAsProceedCommand(dir, next(it)->name, miles);
                        commsVec.push_back(comm);
                    }
                    else if (angleBetween2Lines(*it, *next(it)) >= 1 && angleBetween2Lines(*it, *next(it)) < 180) //turn left
                    {
                        comm.initAsTurnCommand("left", next(it)->name);       
                        commsVec.push_back(comm);
                    }
                    else if (angleBetween2Lines(*it, *next(it)) >= 180 && angleBetween2Lines(*it, *next(it)) <= 359) //turn right
                    {
                        comm.initAsTurnCommand("right", next(it)->name);      
                        commsVec.push_back(comm);
                    }
                }

                else 
                {        //otherwise just generate a proceed command
                    string direction = calculatedirection(*it);
                    comm.initAsProceedCommand(direction, it->name, distanceEarthMiles(it->start, it->end));
                    commsVec.push_back(comm);
                }
            }
            else 
            {
                noTurn = true;
                comm.initAsDeliverCommand(toDeliv[0].item);
                commsVec.push_back(comm);
            }
        }
    }

    for (int i = 0; i < toDeliv.size(); i++) //create routes for each delivry spot
    {     
        if (i + 1 != toDeliv.size()) 
        {
            if (ptp.generatePointToPointRoute(toDeliv[i].location, toDeliv[i + 1].location, ptproutes, miles) == NO_ROUTE)
            {
                return NO_ROUTE;
            }
            delivRoutes.push_back(ptproutes);
            totalMiles += miles;
        }
        else //creates routes to go back
        {        
            if (ptp.generatePointToPointRoute(toDeliv[i].location, depot, temp, miles) == NO_ROUTE)
            {
                return NO_ROUTE;
            }
            totalMiles += miles;
        }
    }
    for (int i = 0; i < delivRoutes.size(); i++) //loops through and generates street commands
    {       
        for (auto it = delivRoutes[i].begin(); it != delivRoutes[i].end(); it++) 
        {
            if (next(it) != delivRoutes[i].end()) 
            {
                if (it->name == next(it)->name) //combine segs w/ same name into one command
                {       
                    GeoCoord start = it->start;
                    string name = it->name;
                    string dir = calculatedirection(*it);
                    while (it->name == next(it)->name) 
                    {
                        it++;
                        if (next(it) == delivRoutes[i].end())
                            break;
                    }                                           //then generate the proceed command
                    comm.initAsProceedCommand(dir, name, distanceEarthMiles(start, it->end));
                    noTurn = false;
                    commsVec.push_back(comm);
                    it--;
                }
                else 
                {
                    if (!noTurn) //if deliv occured before don't turn
                    {                         
                        if (angleBetween2Lines(*it, *next(it)) < 1 || angleBetween2Lines(*it, *next(it)) > 359) 
                        {
                            string dir = calculatedirection(*next(it));
                            comm.initAsProceedCommand(dir, next(it)->name, miles);
                            commsVec.push_back(comm);
                        }
                        else if (angleBetween2Lines(*it, *next(it)) >= 1 && angleBetween2Lines(*it, *next(it)) < 180) 
                        {
                            comm.initAsTurnCommand("left", next(it)->name);
                            commsVec.push_back(comm);
                        }
                        else if (angleBetween2Lines(*it, *next(it)) >= 180 && angleBetween2Lines(*it, *next(it)) <= 359) 
                        {
                            comm.initAsTurnCommand("right", next(it)->name);
                            commsVec.push_back(comm);
                        }
                    }
                }
            }
            else if (i + 1 != toDeliv.size()) //deliver the item
            {      
                noTurn = true;
                comm.initAsDeliverCommand(toDeliv[i + 1].item);
                commsVec.push_back(comm);
            }
        }
    }
    for (auto it = temp.begin(); it != temp.end(); it++) //create commands to go back
    {   
        if (next(it) != temp.end()) 
        {
            if (it->name == next(it)->name) //check if segments on same street
            {           
                GeoCoord start = it->start;
                string name = it->name;
                string dir = calculatedirection(*it);
                while (it->name == next(it)->name) 
                {
                    it++;
                    if (next(it) == temp.end())
                        break;
                }                                               //generate the proceed command
                comm.initAsProceedCommand(dir, name, distanceEarthMiles(start, it->end));
                commsVec.push_back(comm);
                it--;
            }
            else //street changes, generate a turn command
            {        
                if (angleBetween2Lines(*it, *next(it)) < 1 || angleBetween2Lines(*it, *next(it)) > 359) 
                {
                    string dir = calculatedirection(*next(it));
                    comm.initAsProceedCommand(dir, next(it)->name, miles);
                    commsVec.push_back(comm);
                }
                else if (angleBetween2Lines(*it, *next(it)) >= 1 && angleBetween2Lines(*it, *next(it)) < 180) 
                {
                    comm.initAsTurnCommand("left", next(it)->name);
                    commsVec.push_back(comm);
                }
                else if (angleBetween2Lines(*it, *next(it)) >= 180 && angleBetween2Lines(*it, *next(it)) <= 359) 
                {
                    comm.initAsTurnCommand("right", next(it)->name);
                    commsVec.push_back(comm);
                }
            }
        }
    }
    commands = commsVec;
    totalDistanceTravelled = totalMiles;
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

