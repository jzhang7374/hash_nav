#include "provided.h"
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "ExpandableHashMap.h"
#include <fstream>
#include <istream>

using namespace std;

unsigned int hasher(const GeoCoord& g)
{
	return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
	StreetMapImpl();
	~StreetMapImpl();
	bool load(string mapFile);
	bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
	ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_hashMap;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
	ifstream infile(mapFile); //find the map file
	if (!infile)
	{
		cerr << "can't open" << endl;//if cant open data file
		return false;
	}

	string streetName;
	string line;
	string begLatCoord;
	string begLongCoord;
	string finLatCoord;
	string finLongCoord;
	string coordLine;
	int x = 0;

	//read each line of the file, returns true if a line is read, false otherwise
	while (getline(infile, line))
	{
		switch (x)
		{
		case 0:
		{
			streetName = line;
			x--;
			break;
		}
		case-1:
		{
			x = stoi(line); //number of geocoords
			break;
		}
		default:
		{
			istringstream iss(coordLine);
			if (iss >> begLatCoord >> begLongCoord >> finLatCoord >> finLongCoord)
			{
				GeoCoord begSegment = GeoCoord(begLatCoord, begLongCoord);
				GeoCoord finSegment = GeoCoord(finLatCoord, finLongCoord);
				vector<StreetSegment>* findvalsBeg = m_hashMap.find(begSegment);
				StreetSegment newSegment = StreetSegment(begSegment, finSegment, streetName);
				StreetSegment newSegmentRev = StreetSegment(finSegment, begSegment, streetName);

				vector<StreetSegment> keyForward = { newSegment, newSegmentRev };
				vector<StreetSegment> keyReverse = { newSegmentRev, newSegment };

				if (findvalsBeg == nullptr)
				{
					m_hashMap.associate(begSegment, keyForward);
				}
				else
					findvalsBeg->push_back(newSegment);

				vector<StreetSegment>* findvalsFin = m_hashMap.find(finSegment);

				if (findvalsFin == nullptr)
				{
					m_hashMap.associate(begSegment, keyReverse);
				}
				else
					findvalsFin->push_back(newSegmentRev);
			}
			x--;
			break;
		}
		}
	}
	return true;
}


bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	const vector<StreetSegment>* findVals = m_hashMap.find(gc);

	if (findVals == nullptr)
		return false;

	segs = *(m_hashMap.find(gc));
	return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
	m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
	delete m_impl;
}

bool StreetMap::load(string mapFile)
{
	return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	return m_impl->getSegmentsThatStartWith(gc, segs);
}