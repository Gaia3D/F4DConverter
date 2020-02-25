#include "stdafx.h"
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <proj_api.h>

#include "IndoorGMLReader.h"
#include "../geometry/ColorU4.h"
#include "../ConverterManager.h"
#include "../geometry/Matrix4.h"
#include "../util/json/json.h"

using namespace std;
using namespace xercesc;
using namespace gaia3d;
using namespace std;


IndoorGMLReader::IndoorGMLReader() {}
IndoorGMLReader::~IndoorGMLReader() {}

class ParserUtil {
public:
	ParserUtil();
	~ParserUtil();

	bool isTextNode(DOMNode* node);
	bool isMatchedNodeName(DOMNode* node, string nodeType);
	bool hasNamedChild(DOMNode* node, string s);
	bool hasNamedAttribute(DOMNode* node, string s);
	string changeXMLCh2str(const XMLCh* x);
	string getNamedAttribute(DOMNamedNodeMap* list, string s);

	DOMNode* getNamedNode(DOMNodeList* list, string s);
	DOMNode* getNamedNode(vector<DOMNode*> list, string s);
	bool isNamedNodesExist(DOMNodeList* list, string s);
	vector<DOMNode*> getNamedNodes(DOMNodeList* list, string s);
	vector<DOMNode*> getNamedNodes(vector<DOMNode*> list, string s);
};
ParserUtil::ParserUtil() {};
ParserUtil::~ParserUtil() {};
bool ParserUtil::isTextNode(DOMNode* node) {
	return (node->getNodeType() == DOMNode::TEXT_NODE);
}
bool ParserUtil::isMatchedNodeName(DOMNode* node, string nodeType) {
	if (node == NULL) {
		return false;
	}
	return(!strcmp(XMLString::transcode(node->getNodeName()), nodeType.c_str()));
}
bool ParserUtil::hasNamedChild(DOMNode* node, string s) {
	if (node == NULL) {
		return false;
	}
	if (node->hasChildNodes()) {
		for (int i = 0; i < node->getChildNodes()->getLength(); i++) {
			if (isMatchedNodeName(node->getChildNodes()->item(i), s)) {
				return true;
			}
		}
	}
	return false;
}
bool ParserUtil::hasNamedAttribute(DOMNode* node, string s) {
	if (node == NULL) {
		return false;
	}
	if (node->hasAttributes()) {
		for (int i = 0; i < node->getChildNodes()->getLength(); i++) {
			if (isMatchedNodeName(node->getChildNodes()->item(i), s)) {
				return true;
			}
		}
	}
	return false;
}
string ParserUtil::changeXMLCh2str(const XMLCh* x) {
	return XMLString::transcode(x);
}
string ParserUtil::getNamedAttribute(DOMNamedNodeMap* list, string s) {
	string result;
	for (int j = 0; j < list->getLength(); j++) {
		if (isTextNode(list->item(j))) {
			continue;
		}
		if (isMatchedNodeName(list->item(j), s.c_str())) {
			result = changeXMLCh2str(list->item(j)->getNodeValue());
			//cout << result << endl;
			break;
		}
	}
	return result;
}
DOMNode* ParserUtil::getNamedNode(DOMNodeList* list, string s) {
	DOMNode* result = 0;
	for (int i = 0; i < list->getLength(); i++) {
		if (!isTextNode(list->item(i))) {
			if (isMatchedNodeName(list->item(i), s)) {
				result = list->item(i);
				break;
			}
		}
	}
	return result;
}

DOMNode* ParserUtil::getNamedNode(vector<DOMNode*> list, string s) {
	DOMNode* result = 0;
	for (int i = 0; i < list.size(); i++) {
		if (!isTextNode(list.at(i))) {
			if (isMatchedNodeName(list.at(i), s)) {
				result = list.at(i);
				break;
			}
		}
	}
	return result;
}

vector<DOMNode*> ParserUtil::getNamedNodes(DOMNodeList* list, string s) {
	vector<DOMNode*>result;
	for (int i = 0; i < list->getLength(); i++) {
		if (!isTextNode(list->item(i))) {
			if (isMatchedNodeName(list->item(i), s)) {
				result.push_back(list->item(i));

			}
		}
	}
	return result;
}

bool ParserUtil::isNamedNodesExist(DOMNodeList* list, string s) {
	bool isExist = false;
	for (int i = 0; i < list->getLength(); i++) {
		if (!isTextNode(list->item(i))) {
			if (isMatchedNodeName(list->item(i), s)) {
				isExist = true;
				break;
			}
		}
	}
	return isExist;
}

vector<DOMNode*> ParserUtil::getNamedNodes(vector<DOMNode*> list, string s) {
	vector<DOMNode*>result;
	for (int i = 0; i < list.size(); i++) {
		if (!isTextNode(list.at(i))) {
			if (isMatchedNodeName(list.at(i), s)) {
				result.push_back(list.at(i));

			}
		}
	}
	return result;
}

class IndoorFloor{
protected:
	double id;
	vector<string>nodes;
	vector<string>edges;
	Point3D representativePoint;
	double height;

public:
	IndoorFloor() {}
	IndoorFloor(double f) { id = f; };
	IndoorFloor(double f, Point3D r) { id = f; representativePoint = r; }

	void setId(double f) { id = f; }
	double getId() { return id; };
	void addNode(string nodeId) { nodes.push_back(nodeId); }
	void setNodes(vector<string>nodeIds) { nodes = nodeIds; }
	vector<string> getNodes() { return nodes; }
	void addEdge(string edgeId) { edges.push_back(edgeId); }
	void setEdges(vector<string>edgeIds) { edges = edgeIds; }
	void setHeight(double h) { height = h; }
	double getHeight() { return height; }
	vector<string> getEdges() { return edges; }
	void setRepresentativePoint(Point3D rp) {representativePoint = rp;}
	Point3D getRepresentativePoint() { return representativePoint; }

};

class IndoorGMLState {
protected:
	string id;
	string duality;
	vector<string>connects;
	gaia3d::Point3D geometry;
	int floor;

public:
	
	IndoorGMLState() {
		duality = "";
	}
	void setId(string s) { id = s; }
	string getId() { return id; }
	void setDuality(string d) { duality = d; }
	string getDuality() { return duality; }
	void setConnects(vector<string>c) { connects = c; }
	vector<string> getConnects() { return connects; }
	void setGeometry(gaia3d::Point3D p) { geometry = p; }
	gaia3d::Point3D getGeometry() { return geometry; }
	void setFloor(int f) { floor = f; }
	int getFloor() { return floor; }

};

class IndoorGMLLineString {

protected : 
	string id;
	std::vector<Point3D> m_vertices;
public:
	IndoorGMLLineString(string i) { id = i; }
	IndoorGMLLineString() {};
	void setId(string s) { s = id; }
	string getId() { return id; }
		
	void addVertex(Point3D v) { m_vertices.push_back(v); }
	void setVertices(vector<Point3D>vertices) { m_vertices = vertices; }
	vector<Point3D> getVertices() { return m_vertices; }
	Point3D getLowerPosition() {
		Point3D lowerBoundPoint;
		lowerBoundPoint.set(m_vertices.at(0).x, m_vertices.at(0).y, m_vertices.at(0).z);
		
		for (int i = 1; i < m_vertices.size(); i++) {
			if (lowerBoundPoint.x > m_vertices.at(i).x)
				lowerBoundPoint.x = m_vertices.at(i).x;
			if (lowerBoundPoint.y > m_vertices.at(i).y)
				lowerBoundPoint.y = m_vertices.at(i).y;
			if (lowerBoundPoint.z > m_vertices.at(i).z)
				lowerBoundPoint.z = m_vertices.at(i).z;

		}
		return lowerBoundPoint;
	}
	Point3D getUpperPosition() {
		Point3D upperBoundPoint;
		upperBoundPoint.set(m_vertices.at(0).x, m_vertices.at(0).y, m_vertices.at(0).z);

		for (int i = 1; i < m_vertices.size(); i++) {
			
			if (upperBoundPoint.x < m_vertices.at(i).x)
				upperBoundPoint.x = m_vertices.at(i).x;
			if (upperBoundPoint.y < m_vertices.at(i).y)
				upperBoundPoint.y = m_vertices.at(i).y;
			if (upperBoundPoint.z < m_vertices.at(i).z)
				upperBoundPoint.z = m_vertices.at(i).z;
		}
		return upperBoundPoint;
	}
	Point3D getCenterPosition() {
		Point3D result;
		result.set(0,0,0);
		Point3D lowerBoundPoint;
		Point3D upperBoundPoint;

		lowerBoundPoint.set(m_vertices.at(0).x, m_vertices.at(0).y,m_vertices.at(0).z);
		upperBoundPoint.set(m_vertices.at(0).x, m_vertices.at(0).y, m_vertices.at(0).z);

		for (int i = 1; i < m_vertices.size(); i++) {
			if (lowerBoundPoint.x > m_vertices.at(i).x)
				lowerBoundPoint.x = m_vertices.at(i).x;
			if (lowerBoundPoint.y > m_vertices.at(i).y)
				lowerBoundPoint.y = m_vertices.at(i).y;
			if (lowerBoundPoint.z > m_vertices.at(i).z)
				lowerBoundPoint.z = m_vertices.at(i).z;

			if (upperBoundPoint.x < m_vertices.at(i).x)
				upperBoundPoint.x = m_vertices.at(i).x;
			if (upperBoundPoint.y < m_vertices.at(i).y)
				upperBoundPoint.y = m_vertices.at(i).y;
			if (upperBoundPoint.z < m_vertices.at(i).z)
				upperBoundPoint.z = m_vertices.at(i).z;

		}

		result = (upperBoundPoint + lowerBoundPoint) / 2;
		return result;
	}
	
};

class IndoorGMLTransition {
protected:
	string id;
	string duality;
	vector<string>connects;
	shared_ptr<IndoorGMLLineString> geometry;
	vector<int>floors;
public:
	IndoorGMLTransition() {
		duality = "";
	}
	void setId(string s) { id = s; }
	string getId() { return id; };
	void setDuality(string d) { duality = d; }
	string getDuality() { return duality; }
	void setGeometry(shared_ptr<IndoorGMLLineString> g) { geometry = g; }
	shared_ptr<IndoorGMLLineString> getGeometry() { return geometry; }
	void addConnects(string c) { connects.push_back(c); }
	void setConnects(vector<string>clist) { connects = clist; }
	vector<string>getConnects() { return connects; }
	void addFloor(int f) { floors.push_back(f); }
	void setFloors(vector<int>fs) { floors = fs; }
	vector<int> getFloors() { return floors; }
	Point3D getUpperBoundPosition() {
		return geometry->getUpperPosition();
	}
	Point3D getLowerBoundPosition() {
		return geometry->getLowerPosition();
	}
};

class IndoorGMLLinearRing {
public:
	IndoorGMLLinearRing(const std::string& _id) { id = _id; }

	unsigned int size() const;

	const std::vector<Point3D>& getVertices() const;
	std::vector<Point3D>& getVertices();
	void setVertices(std::vector<Point3D> vertices);

	void addVertex(const Point3D& v);

	void forgetVertices();

	gaia3d::Point3D getCenterPoint() {
		Point3D centerPoint;
		centerPoint.set(0,0,0);
		for (int i = 0; i < m_vertices.size(); i++) {
			centerPoint = centerPoint + m_vertices.at(i);
		}
		centerPoint.set(centerPoint.x/m_vertices.size(), centerPoint.y/m_vertices.size(), centerPoint.z/m_vertices.size());
		return centerPoint;
	}

	gaia3d::Point3D getLowerBoundPoint() {
		Point3D lowerBoundPoint;
		lowerBoundPoint.set(m_vertices.at(0).x, m_vertices.at(0).y, m_vertices.at(0).z);
		for (int i = 1; i < m_vertices.size(); i++) {
			if (lowerBoundPoint.x > m_vertices.at(i).x)
				lowerBoundPoint.x = m_vertices.at(i).x;

			if (lowerBoundPoint.y > m_vertices.at(i).y)
				lowerBoundPoint.y = m_vertices.at(i).y;

			if (lowerBoundPoint.z > m_vertices.at(i).z)
				lowerBoundPoint.z = m_vertices.at(i).z;

		}

		return lowerBoundPoint;
	}

	gaia3d::Point3D getUpperBoundPoint() {
		Point3D upperBoundPoint;
		upperBoundPoint.set(m_vertices.at(0).x, m_vertices.at(0).y, m_vertices.at(0).z);
		for (int i = 1; i < m_vertices.size(); i++) {
			if (upperBoundPoint.x < m_vertices.at(i).x)
				upperBoundPoint.x = m_vertices.at(i).x;

			if (upperBoundPoint.y < m_vertices.at(i).y)
				upperBoundPoint.y = m_vertices.at(i).y;

			if (upperBoundPoint.z < m_vertices.at(i).z)
				upperBoundPoint.z = m_vertices.at(i).z;
		}

		return upperBoundPoint;
	}

protected:
	bool m_exterior;
	string id;
	std::vector<Point3D> m_vertices;

};

unsigned int IndoorGMLLinearRing::size() const
{
	return m_vertices.size();
}

void IndoorGMLLinearRing::addVertex(const Point3D& v)
{
	m_vertices.push_back(v);
}

std::vector<Point3D>& IndoorGMLLinearRing::getVertices()
{
	return m_vertices;
}

void IndoorGMLLinearRing::setVertices(std::vector<Point3D> vertices)
{
	m_vertices = vertices;
}

const std::vector<Point3D>& IndoorGMLLinearRing::getVertices() const
{
	return m_vertices;
}

void IndoorGMLLinearRing::forgetVertices()
{
	m_vertices.clear();
}

class IndoorGMLPolygon
{

public:


	// Get the vertices
	const std::vector<Point3D>& getVertices() const;
	std::vector<Point3D>& getVertices();

	// Get the indices
	const std::vector<unsigned int>& getIndices() const;

	bool negNormal() const;
	void setNegNormal(bool negNormal);

	void addRing(IndoorGMLLinearRing*);

	void setExterior(shared_ptr<IndoorGMLLinearRing> l);
	void setInterior(std::vector<shared_ptr<IndoorGMLLinearRing>> l) { m_interiorRings = l; }
	shared_ptr<IndoorGMLLinearRing> getExterior();
	vector<shared_ptr<IndoorGMLLinearRing>> getInterior() { return m_interiorRings; }
	IndoorGMLPolygon(string _id) { id = _id; }
	gaia3d::Point3D getCenterPoint() {
		return m_exteriorRing->getCenterPoint();
	}
	gaia3d::Point3D getLowerBoundPoint() {
		return m_exteriorRing->getLowerBoundPoint();
	}

	gaia3d::Point3D getUpperBoundPoint() {
		return m_exteriorRing->getUpperBoundPoint();
	}
	virtual ~IndoorGMLPolygon();

protected:

	string id;
	std::vector<Point3D> m_vertices;
	std::vector<unsigned int> m_indices;

	std::shared_ptr<IndoorGMLLinearRing> m_exteriorRing;
	std::vector<std::shared_ptr<IndoorGMLLinearRing> > m_interiorRings;

	bool m_negNormal;
	bool m_finished;

	//std::shared_ptr<Logger> m_logger;
};
const std::vector<Point3D>& IndoorGMLPolygon::getVertices() const
{
	return m_vertices;
}

std::vector<Point3D>& IndoorGMLPolygon::getVertices()
{
	return m_vertices;
}

const std::vector<unsigned int>& IndoorGMLPolygon::getIndices() const
{
	return m_indices;
}

bool IndoorGMLPolygon::negNormal() const
{
	return m_negNormal;
}
void IndoorGMLPolygon::setExterior(shared_ptr<IndoorGMLLinearRing> l) {
	m_exteriorRing = l;
	for (int i = 0; i < l->getVertices().size(); i++) {
		m_indices.push_back(i);
	}
}
shared_ptr<IndoorGMLLinearRing> IndoorGMLPolygon::getExterior() {
	return m_exteriorRing;
}
void IndoorGMLPolygon::setNegNormal(bool negNormal)
{
	m_negNormal = negNormal;
}

IndoorGMLPolygon::~IndoorGMLPolygon()
{
}

class IndoorGMLSolid
{
public:
	BoundingBox bb;

	~IndoorGMLSolid() {}

	bool IndoorGMLSolid::hasExterior() {
		if (exterior.size() == 0) {
			return false;
		}
		else {
			return true;
		}
	}
	bool IndoorGMLSolid::hasInterior() {
		if (interior.size() == 0) {
			return false;
		}
		else {
			return true;
		}
	}
	//const IndoorGMLSolid& IndoorGMLSolid::getExterior() const {	}
	vector<shared_ptr<IndoorGMLPolygon>> IndoorGMLSolid::getExterior() {
		return exterior;
	}
	std::vector<std::shared_ptr<IndoorGMLSolid>> IndoorGMLSolid::getInterior() {
		return interior;
	}
	void IndoorGMLSolid::addInterior(std::shared_ptr<IndoorGMLSolid> s) {
		interior.push_back(s);
	}
	//void deleteInterior(){}
	void IndoorGMLSolid::setExterior(vector<std::shared_ptr<IndoorGMLPolygon>> s) {
		exterior = s;
	}

	IndoorGMLSolid::IndoorGMLSolid(const std::string& _id) { id = _id; }

	void setId(string _id) { id = _id; }
	string getId() { return id; }

	gaia3d::Point3D getCenterPosition() {
		gaia3d::Point3D centerPoint;
		centerPoint.set(0,0,0);
		
		for (int i = 0; i < exterior.size(); i++) {
			centerPoint = centerPoint + exterior.at(i)->getCenterPoint();
		}
		centerPoint.set(centerPoint.x/exterior.size(), centerPoint.y/exterior.size(), centerPoint.z/exterior.size());
		return centerPoint;
	}

	gaia3d::Point3D getLowerBoundPosition() {
		gaia3d::Point3D lowerBoundPosition;
		lowerBoundPosition.set(exterior.at(0)->getLowerBoundPoint().x, exterior.at(0)->getLowerBoundPoint().y, exterior.at(0)->getLowerBoundPoint().z);

		for (int i = 1; i < exterior.size(); i++) {
			if (lowerBoundPosition.x > exterior.at(i)->getLowerBoundPoint().x)
				lowerBoundPosition.x = exterior.at(i)->getLowerBoundPoint().x;
			
			if (lowerBoundPosition.y > exterior.at(i)->getLowerBoundPoint().y)
				lowerBoundPosition.y = exterior.at(i)->getLowerBoundPoint().y;

			if (lowerBoundPosition.z > exterior.at(i)->getLowerBoundPoint().z)
				lowerBoundPosition.z = exterior.at(i)->getLowerBoundPoint().z;
		}

		return lowerBoundPosition;
	}

	gaia3d::Point3D getUpperBoundPosition() {
		gaia3d::Point3D upperBoundPosition;
		upperBoundPosition.set(exterior.at(0)->getUpperBoundPoint().x, exterior.at(0)->getUpperBoundPoint().y, exterior.at(0)->getUpperBoundPoint().z);

		for (int i = 1; i < exterior.size(); i++) {
			if (upperBoundPosition.x < exterior.at(i)->getUpperBoundPoint().x)
				upperBoundPosition.x = exterior.at(i)->getUpperBoundPoint().x;

			if (upperBoundPosition.y < exterior.at(i)->getUpperBoundPoint().y)
				upperBoundPosition.y = exterior.at(i)->getUpperBoundPoint().y;

			if (upperBoundPosition.z < exterior.at(i)->getUpperBoundPoint().z)
				upperBoundPosition.z = exterior.at(i)->getUpperBoundPoint().z;
		}

		return upperBoundPosition;
	}

protected:

	bool m_finished;
	unsigned int m_lod;
	string id;
	std::vector<std::shared_ptr<IndoorGMLPolygon> > exterior;
	std::vector<std::shared_ptr<IndoorGMLSolid>> interior;
};




class GeometryManager
{
public:
	gaia3d::BoundingBox bb;

	size_t getIndoorGMLSolidsCount() const;
	std::shared_ptr<IndoorGMLSolid> getIndoorGMLSolid(size_t i);
	//std::shared_ptr<const gaia3d::IndoorGMLSolid> getIndoorGMLSolid(size_t i) const;
	//std::shared_ptr<gaia3d::IndoorGMLSolid> getIndoorGMLSolid(string id);

	size_t getIndoorGMLPolygonsCount() const;
	std::shared_ptr<IndoorGMLPolygon> getIndoorGMLPolygon(size_t i);
	//std::shared_ptr<const gaia3d::IndoorGMLPolygon> getIndoorGMLPolygon(size_t i) const;
;
	void addIndoorGMLPolygon(std::shared_ptr<IndoorGMLPolygon>);
	void addIndoorGMLSolid(std::shared_ptr<IndoorGMLSolid>);

	~GeometryManager();
	GeometryManager();

protected:

	bool m_finished;
	std::vector<std::shared_ptr<IndoorGMLSolid> > m_IndoorGMLSolids;
	std::vector<std::shared_ptr<IndoorGMLPolygon> > m_IndoorGMLPolygons;
};

GeometryManager::GeometryManager()
{
}


size_t GeometryManager::getIndoorGMLPolygonsCount() const
{
	return m_IndoorGMLPolygons.size();
}

std::shared_ptr<IndoorGMLPolygon> GeometryManager::getIndoorGMLPolygon(size_t i)
{
	return m_IndoorGMLPolygons.at(i);
}

std::shared_ptr<IndoorGMLSolid> GeometryManager::getIndoorGMLSolid(size_t i) {
	return m_IndoorGMLSolids.at(i);
}

size_t GeometryManager::getIndoorGMLSolidsCount() const {
	return m_IndoorGMLSolids.size();
}


GeometryManager::~GeometryManager()
{
}

void GeometryManager::addIndoorGMLPolygon(std::shared_ptr<IndoorGMLPolygon> p)
{
	m_IndoorGMLPolygons.push_back(p);
}

void GeometryManager::addIndoorGMLSolid(std::shared_ptr<IndoorGMLSolid> s) {
	m_IndoorGMLSolids.push_back(s);
}


class GeometryParser {
public:
	shared_ptr<IndoorGMLLinearRing> parseIndoorGMLLinearRing(xercesc::DOMNode* l, gaia3d::BoundingBox* b);

	shared_ptr<IndoorGMLPolygon> parseIndoorGMLPolygon(xercesc::DOMNode* p, gaia3d::BoundingBox* b);

	shared_ptr<IndoorGMLSolid> parseIndoorGMLSolid(xercesc::DOMNode* s, gaia3d::BoundingBox* b);

	shared_ptr<IndoorGMLLineString> parseIndoorGMLLineString(xercesc::DOMNode* s, gaia3d::BoundingBox* b);

};
shared_ptr<IndoorGMLLineString> GeometryParser::parseIndoorGMLLineString(xercesc::DOMNode* l, gaia3d::BoundingBox* b) {
	ParserUtil* parseHelper = new ParserUtil();
	shared_ptr<IndoorGMLLineString> result = shared_ptr<IndoorGMLLineString>(new IndoorGMLLineString(parseHelper->getNamedAttribute(l->getAttributes(), "gml:id")));
	if (parseHelper->hasNamedChild(l, "gml:pos")) {
		double arr[3];
		int count = 0;
		vector<gaia3d::Point3D>pointList;
		for (int i = 0; i < l->getChildNodes()->getLength(); i++) {
			if (!parseHelper->isTextNode(l->getChildNodes()->item(i))) {
				//cout << parseHelper->changeXMLCh2str(l->getChildNodes()->item(i)->getTextContent()) << endl;
				string pointvalue = parseHelper->changeXMLCh2str(l->getChildNodes()->item(i)->getTextContent());
				stringstream ss(pointvalue);
				ss >> arr[0] >> arr[1] >> arr[2];
				b->addPoint(arr[0], arr[1], arr[2]);
				//cout << arr[0] << " " << arr[1] << " " << arr[2] << endl;
				gaia3d::Point3D newPoint;
				newPoint.set(arr[0], arr[1], arr[2]);
				pointList.push_back(newPoint);
			}

		}
		result->setVertices(pointList);

	}
	else if (parseHelper->hasNamedChild(l, "gml:posList")) {
		//TODO: 
		double arr[3];
		int count = 0;
		vector<gaia3d::Point3D>pointList;
		vector<double>singleAxisValues;
		//get single line of the posList 
		for (int i = 0; i < l->getChildNodes()->getLength(); i++) {
			if (!parseHelper->isTextNode(l->getChildNodes()->item(i))) {
				string posListValue = parseHelper->changeXMLCh2str(l->getChildNodes()->item(i)->getTextContent());
				stringstream ss(posListValue);
				for (double s; ss >> s;) {
					singleAxisValues.push_back(s);
				}
				break;
			}
		}
		for (int i = 0; i < singleAxisValues.size(); i += 3) {
			arr[0] = singleAxisValues.at(i);
			arr[1] = singleAxisValues.at(i + 1);
			arr[2] = singleAxisValues.at(i + 2);
			gaia3d::Point3D newPoint;
			newPoint.set(arr[0], arr[1], arr[2]);
			pointList.push_back(newPoint);
		}
		result->setVertices(pointList);
	}
	result->getVertices().pop_back();
	return result;
}
shared_ptr<IndoorGMLLinearRing> GeometryParser::parseIndoorGMLLinearRing(DOMNode* l, gaia3d::BoundingBox* b) {
	ParserUtil* parseHelper = new ParserUtil();
	shared_ptr<IndoorGMLLinearRing> result = shared_ptr<IndoorGMLLinearRing>(new IndoorGMLLinearRing(parseHelper->getNamedAttribute(l->getAttributes(), "gml:id")));
	if (parseHelper->hasNamedChild(l, "gml:pos")) {
		double arr[3];
		int count = 0;
		vector<gaia3d::Point3D>pointList;
		for (int i = 0; i < l->getChildNodes()->getLength(); i++) {
			if (!parseHelper->isTextNode(l->getChildNodes()->item(i))) {
				//cout << parseHelper->changeXMLCh2str(l->getChildNodes()->item(i)->getTextContent()) << endl;
				string pointvalue = parseHelper->changeXMLCh2str(l->getChildNodes()->item(i)->getTextContent());
				stringstream ss(pointvalue);
				ss >> arr[0] >> arr[1] >> arr[2];
				b->addPoint(arr[0], arr[1], arr[2]);
				//cout << arr[0] << " " << arr[1] << " " << arr[2] << endl;
				gaia3d::Point3D newPoint;
				newPoint.set(arr[0], arr[1], arr[2]);
				pointList.push_back(newPoint);
			}

		}
		result->setVertices(pointList);

	}
	else if (parseHelper->hasNamedChild(l, "gml:posList")) {
		//TODO: 
		double arr[3];
		int count = 0;
		vector<gaia3d::Point3D>pointList;
		vector<double>singleAxisValues;
		//get single line of the posList 
		for (int i = 0; i < l->getChildNodes()->getLength(); i++) {
			if (!parseHelper->isTextNode(l->getChildNodes()->item(i))) {
				string posListValue = parseHelper->changeXMLCh2str(l->getChildNodes()->item(i)->getTextContent());
				stringstream ss(posListValue);
				for (double s; ss >> s;) {
					singleAxisValues.push_back(s);
				}
				break;
			}
		}
		for (int i = 0; i < singleAxisValues.size(); i+=3) {
			arr[0] = singleAxisValues.at(i);
			arr[1] = singleAxisValues.at(i+1);
			arr[2] = singleAxisValues.at(i + 2);
			gaia3d::Point3D newPoint;
			newPoint.set(arr[0], arr[1], arr[2]);
			b->addPoint(arr[0], arr[1], arr[2]);
			pointList.push_back(newPoint);
		}
		result->setVertices(pointList);
	}
	result->getVertices().pop_back();
	return result;
}

shared_ptr<IndoorGMLPolygon> GeometryParser::parseIndoorGMLPolygon(DOMNode* p, gaia3d::BoundingBox* b) {
	ParserUtil* parseHelper = new ParserUtil();

	shared_ptr<IndoorGMLPolygon> result = shared_ptr<IndoorGMLPolygon>(new IndoorGMLPolygon(parseHelper->getNamedAttribute(p->getAttributes(), "gml:id")));
	DOMNode* exterior = parseHelper->getNamedNode(p->getChildNodes(), "gml:exterior");
	if (parseHelper->hasNamedChild(exterior, "gml:LinearRing")) {
		result->setExterior(parseIndoorGMLLinearRing(parseHelper->getNamedNode(exterior->getChildNodes(), "gml:LinearRing"), b));
	}
	else if (parseHelper->isMatchedNodeName(exterior, "gml:LineString")) {}
	return result;
}


shared_ptr<IndoorGMLSolid> GeometryParser::parseIndoorGMLSolid(DOMNode* s, gaia3d::BoundingBox* b) {
	ParserUtil* parseHelper = new ParserUtil();
	shared_ptr<IndoorGMLSolid> result = shared_ptr<IndoorGMLSolid>(new IndoorGMLSolid(parseHelper->getNamedAttribute(s->getAttributes(), "gml:id")));
	DOMNode* exterior = parseHelper->getNamedNode(s->getChildNodes(), "gml:exterior");
	DOMNode* shell = parseHelper->getNamedNode(exterior->getChildNodes(), "gml:Shell");
	vector<DOMNode*> surfaceMember = parseHelper->getNamedNodes(shell->getChildNodes(), "gml:surfaceMember");
	vector<DOMNode*> polygonlist;
	//get polygon
	for (int i = 0; i < surfaceMember.size(); i++) {
		DOMNode* p = parseHelper->getNamedNode(surfaceMember.at(i)->getChildNodes(), "gml:Polygon");
		polygonlist.push_back(p);
	}
	//parse polygon
	vector<shared_ptr<IndoorGMLPolygon>>parsedPolygon;
	for (int i = 0; i < surfaceMember.size(); i++) {
		parsedPolygon.push_back(parseIndoorGMLPolygon(polygonlist.at(i), b));
	}
	result->setExterior(parsedPolygon);

	return result;
}

Json::Value changePoint2GeoJSONPoint(gaia3d::Point3D point) {
	Json::Value result(Json::objectValue);
	Json::Value coordinates(Json::arrayValue);
	Json::Value coord(Json::realValue);
	coord = point.x;
	coordinates.append(coord);
	coord = point.y;
	coordinates.append(coord);
	coord = point.z;
	coordinates.append(coord);

	result["coordinates"] = coordinates;
	result["type"] = "point";

	return result;
}

Json::Value changeLineString2GeoJsonLineString(IndoorGMLLineString line) {
	Json::Value result(Json::objectValue);
	Json::Value coordinates(Json::arrayValue);
	Json::Value coord(Json::realValue);

	for (int i = 0; i < line.getVertices().size(); i++) {
		Point3D point = line.getVertices().at(i);
		Json::Value pointCoord(Json::arrayValue);
		coord = point.x;
		pointCoord.append(coord);
		coord = point.y;
		pointCoord.append(coord);
		coord = point.z;
		pointCoord.append(coord);
		coordinates.append(pointCoord);
	}

	result["coordinates"] = coordinates;
	result["type"] = "LineString";

	return result;
}

bool isSameFloor(map<double, int>& floorList, double minimumGapHeight, gaia3d::Point3D lowerBound, gaia3d::Point3D upperBound) {
	map<double, int>::iterator it = floorList.begin();

	for (; it != floorList.end(); it++) {
		if (abs(it->first - lowerBound.z) < minimumGapHeight) {
			break;
		}
	}
	if (abs(it->first - upperBound.z) < minimumGapHeight) {
		return true;
	}
	else
		return false;
}

vector<int> searchFloors(map<double, int>& floorList, double minimumGapHeight, gaia3d::Point3D lowerBound, gaia3d::Point3D upperBound) {
	map<double, int>::iterator it = floorList.begin();
	vector<int> result;
	for (; it != floorList.end(); it++) {
		if (abs(it->first - lowerBound.z) < minimumGapHeight) {
			break;
		}
	}

	if (abs(it->first - upperBound.z) < minimumGapHeight) {
		result.push_back(it->second);
		return result;
	}
	else{
		for (; it != floorList.end(); it++) {
			if (abs(it->first - upperBound.z) < minimumGapHeight) {
				result.push_back(it->second);
				return result;
			}
			else {
				result.push_back(it->second);
			}
		}
	}
}

GeometryManager IndoorGMLReader::parseIndoorGeometry(DOMDocument* dom) {
	
	ParserUtil* parseHelper = new ParserUtil();
	GeometryParser* gmp = new GeometryParser();
	GeometryManager geomManager;
	try {
		DOMElement* rootNode = dom->getDocumentElement();
		//cout << XMLString::transcode(rootNode->getTagName()) << endl;
		DOMNodeList* rootChild = rootNode->getChildNodes();

		DOMNode* primalSpaceFeatures = 0;
		DOMNode* PrimalSpaceFeatures = 0;
		
		DOMNode* multiLayeredGraph = 0;
		DOMNode* MultiLayeredGraph = 0;
		
		DOMNode* spaceLayers = 0;
		vector<DOMNode*> spaceLayerMember;

		vector<DOMNode*> states;

		vector<DOMNode*> cellSpaceMember;
		vector<DOMNode*> cellSpaceBoundaryMember;
		BoundingBox* b = new BoundingBox();

		//primalSpaceFeatures -> PrimalSpaceFeatures
		
		//std::cout << "IndoorGML Parser : Get Document Root" << endl;
		
		string frontTag = "core:";
		string nextTag;
		//primalSpaceFeatures -> PrimalSpaceFeatures
		primalSpaceFeatures = parseHelper->getNamedNode(rootChild, "core:primalSpaceFeatures");
		if (primalSpaceFeatures == 0) {
			primalSpaceFeatures = parseHelper->getNamedNode(rootChild, "primalSpaceFeatures");
			frontTag = "";
			if (primalSpaceFeatures == 0) {
				primalSpaceFeatures = parseHelper->getNamedNode(rootChild, "indoor:primalSpaceFeatures");
				frontTag = "indoor:";
			}
		}
		nextTag = frontTag + "PrimalSpaceFeatures";	
		primalSpaceFeatures = parseHelper->getNamedNode(primalSpaceFeatures->getChildNodes(), nextTag);

		//multiLayeredGraph -> MultiLayeredGraph
		nextTag = frontTag + "multiLayeredGraph";

		multiLayeredGraph = parseHelper->getNamedNode(rootChild, nextTag);
		nextTag = frontTag + "MultiLayeredGraph";

		multiLayeredGraph = parseHelper->getNamedNode(multiLayeredGraph->getChildNodes(), nextTag);


		nextTag = frontTag + "cellSpaceMember";

		cellSpaceMember = parseHelper->getNamedNodes(primalSpaceFeatures->getChildNodes(), nextTag);
		nextTag = frontTag + "cellSpaceBoundaryMember";

		cellSpaceBoundaryMember = parseHelper->getNamedNodes(primalSpaceFeatures->getChildNodes(), nextTag);

		vector<DOMNode*>cellspacelist;
		vector<DOMNode*>cellspaceboundarylist;
		vector<DOMNode*>IndoorGMLSolidList;
		vector<DOMNode*>surfaceList;
		map<string, gaia3d::Point3D> cellSpaceCenterLowerPointList;
		map<string, shared_ptr<IndoorGMLSolid>> cellSpaceSolidList;
		map<double, int> floorList;


		//전체 Bounding box의 lower point, upper point를 구한다. 
		gaia3d::Point3D *lowerBoundingBoxPoint = 0;
		gaia3d::Point3D *upperBoundingBoxPoint = 0;

		//기하에서 층을 구하기 위해 최소 층의 높이를 구한다. 
		double minimumGapHeight = 0;
		nextTag = frontTag + "CellSpace";

		for (int i = 0; i < cellSpaceMember.size(); i++) {
			//cellspacelist.push_back();

			DOMNode* cellSpace = parseHelper->getNamedNode(cellSpaceMember.at(i)->getChildNodes(), nextTag);
			if (cellSpace != 0) {
				for (int j = 0; j < cellSpace->getChildNodes()->getLength(); j++) {
					string nextGeometryTag = frontTag + "cellSpaceGeometry";

					if (parseHelper->isMatchedNodeName(cellSpace->getChildNodes()->item(j), nextGeometryTag)) {
						DOMNode* solid = cellSpace->getChildNodes()->item(j)->getChildNodes()->item(1)->getChildNodes()->item(1);
						std::shared_ptr<IndoorGMLSolid> result = gmp->parseIndoorGMLSolid(solid, b);
						gaia3d::Point3D centerPoint = result->getCenterPosition();
						//층 때문에 z는 lower bound로, 나머지는 기하의 중심점을 쓰기로 하고.
						centerPoint.z = result->getLowerBoundPosition().z;

						//get the lowerBoundingBoxPoint and upperBoundingBoxPoint for calculating height of the building
						gaia3d::Point3D lowerBoundPoint = result->getLowerBoundPosition();
						gaia3d::Point3D upperBoundPoint = result->getUpperBoundPosition();

						//need to multiple unitscalefactor...? nope. It is needed to be applied at the end of the reading process
						//lowerBoundingBoxPoint 초기화
						if (lowerBoundingBoxPoint == 0) {
							lowerBoundingBoxPoint = new gaia3d::Point3D();
							lowerBoundingBoxPoint->set(lowerBoundPoint.x, lowerBoundPoint.y, lowerBoundPoint.z);
						}
						else {
							if (lowerBoundingBoxPoint->x > lowerBoundPoint.x)
								lowerBoundingBoxPoint->x = lowerBoundPoint.x;
							if (lowerBoundingBoxPoint->y > lowerBoundPoint.y)
								lowerBoundingBoxPoint->y = lowerBoundPoint.y;
							if (lowerBoundingBoxPoint->z > lowerBoundPoint.z)
								lowerBoundingBoxPoint->z = lowerBoundPoint.z;
						}

						//upperBoundingBoxPoint 초기화
						if (upperBoundingBoxPoint == 0) {
							upperBoundingBoxPoint = new gaia3d::Point3D();
							upperBoundingBoxPoint->set(lowerBoundPoint.x, lowerBoundPoint.y, lowerBoundPoint.z);
						}
						else {
							if (upperBoundingBoxPoint->x < upperBoundPoint.x)
								upperBoundingBoxPoint->x = upperBoundPoint.x;
							if (upperBoundingBoxPoint->y < upperBoundPoint.y)
								upperBoundingBoxPoint->y = upperBoundPoint.y;
							if (upperBoundingBoxPoint->z < upperBoundPoint.z)
								upperBoundingBoxPoint->z = upperBoundPoint.z;
						}

						if (minimumGapHeight == 0) { 
							minimumGapHeight = upperBoundPoint.z - lowerBoundPoint.z;
						}
						else {
							if (minimumGapHeight > upperBoundPoint.z - lowerBoundPoint.z)
								minimumGapHeight = upperBoundPoint.z - lowerBoundPoint.z;
						}

						//못 찾았을 경우에
						if (floorList.find(lowerBoundPoint.z) == floorList.end()) {
							floorList.insert(pair<double, int>(lowerBoundPoint.z, floorList.size()));
						}

						string cellId = parseHelper->getNamedAttribute(cellSpace->getAttributes(), "gml:id");
						//centerPoint.set(centerPoint.x * unitScaleFactor , centerPoint.y * unitScaleFactor, centerPoint.z * unitScaleFactor);
						cellSpaceSolidList.insert(pair<string, shared_ptr<IndoorGMLSolid>>(cellId, result));
						cellSpaceCenterLowerPointList.insert(pair<string, gaia3d::Point3D>(cellId, centerPoint));
						geomManager.addIndoorGMLSolid(result);
					}
				}
			}

		}

		//calculate the height of the building
		double buildingHeight = upperBoundingBoxPoint->z - lowerBoundingBoxPoint->z;

		//calculate average minimum gap among floors
		if(minimumGapHeight > buildingHeight / floorList.size())
			minimumGapHeight = buildingHeight / floorList.size();

		map<double, int>::iterator floorListIter = floorList.begin();
		map<double, int>::iterator floorListIter2 = floorList.begin();

		//층 간의 높이 차이가 허용범위 내에 있다면 같은 층으로 취급하는 알고리즘
		floorListIter2++;
		for (; floorListIter2 != floorList.end(); floorListIter2++, floorListIter++) {
			if (floorListIter2->first - floorListIter->first < minimumGapHeight) {
				floorListIter2->second = floorListIter->second;
			}
		}

		floorListIter = floorList.begin();
		floorListIter2 = floorList.begin();
		floorListIter2++;

		int index = 0;
		floorListIter->second = index;
		map<double, int>tempFloorList;
		tempFloorList.insert(pair<double, int>(floorListIter->first, index));

		minimumGapHeight = floorListIter2->first - floorListIter->first;


		for (; floorListIter2 != floorList.end(); floorListIter++, floorListIter2++) {
			if (floorListIter2->second == floorListIter->second) {
				//하나의 층이 허용 범위를 가지면서 여러 높이를 가질 수 있다.
				tempFloorList.insert(pair<double, int>(floorListIter2->first, index));
			}
			else {
				//다른 층으로 취급
				index++;
				tempFloorList.insert(pair<double, int>(floorListIter2->first, index));
			}

		}

		floorList = tempFloorList;

		//floor class 가지고 진짜 floor list를 생성할 것

		map<int, IndoorFloor>floorListInfoMap;

		floorListIter = floorList.begin();
		for (; floorListIter != floorList.end(); floorListIter++) {
			IndoorFloor newFloor;
			//floor의 id는 층번호. 지하실을 넣을 수도 있다. 
			newFloor.setId(floorListIter->second);
			newFloor.setHeight(floorListIter->first);
			floorListInfoMap.insert(pair<int,IndoorFloor>(floorListIter->second,newFloor));
		}

		nextTag = frontTag + "CellSpaceBoundary";

		for (int i = 0; i < cellSpaceBoundaryMember.size(); i++) {
			DOMNode* cellSpaceboundary = parseHelper->getNamedNode(cellSpaceBoundaryMember.at(i)->getChildNodes(), nextTag);
			if (cellSpaceboundary != 0) {
				for (int j = 0; j < cellSpaceboundary->getChildNodes()->getLength(); j++) {
					string nextGeometryTag = frontTag + "cellSpaceBoundaryGeometry";

					if (parseHelper->isMatchedNodeName(cellSpaceboundary->getChildNodes()->item(j), nextGeometryTag)) {
						DOMNode* surface = cellSpaceboundary->getChildNodes()->item(j)->getChildNodes()->item(1)->getChildNodes()->item(1);
						if (parseHelper->changeXMLCh2str(surface->getNodeName()) == "gml:Polygon") {

							shared_ptr<IndoorGMLPolygon> result = gmp->parseIndoorGMLPolygon(surface, b);
							geomManager.addIndoorGMLPolygon(result);
						}
					}

				}
			}
		}


		nextTag = frontTag + "spaceLayers";

		spaceLayers = parseHelper->getNamedNode(multiLayeredGraph -> getChildNodes(), nextTag);

		nextTag = frontTag + "spaceLayerMember";

		spaceLayerMember = parseHelper->getNamedNodes(spaceLayers->getChildNodes(), nextTag);

		vector<IndoorGMLState>stateList;
		vector<IndoorGMLTransition>transitionList;


		for (int i = 0; i < spaceLayerMember.size(); i++) {
			nextTag = frontTag + "SpaceLayer";
			DOMNode* spaceLayer = 0;
			spaceLayer = parseHelper->getNamedNode(spaceLayerMember.at(i)->getChildNodes(), nextTag);
			
			DOMNode* edges = 0;
			nextTag = frontTag + "edges";
			edges = parseHelper->getNamedNode(spaceLayer->getChildNodes(), nextTag);
			if (edges != 0) {
				nextTag = frontTag + "transitionMember";
				vector<DOMNode*>tempTransitions = parseHelper->getNamedNodes(edges->getChildNodes(), nextTag);
				if (tempTransitions.size() != 0) {
					nextTag = frontTag + "Transition";
					for (int j = 0; j < tempTransitions.size(); j++) {
						IndoorGMLTransition transitionInstance;
						DOMNode* transition = parseHelper->getNamedNode(tempTransitions.at(j)->getChildNodes(), nextTag);
						DOMNode* transitionGeometry = 0;
						transitionGeometry = parseHelper->getNamedNode(transition->getChildNodes(), frontTag + "geometry");
						if (transitionGeometry != 0) {
							DOMNode* linestring = transitionGeometry->getChildNodes()->item(1);
							shared_ptr<IndoorGMLLineString>parsedLineString = gmp->parseIndoorGMLLineString(linestring,b);
							transitionInstance.setGeometry(parsedLineString);
						}

						string transitionId = parseHelper->getNamedAttribute(transition->getAttributes(), "gml:id");
						transitionInstance.setId(transitionId);
						
						vector<DOMNode*>connects;
						connects = parseHelper->getNamedNodes(transition->getChildNodes(), frontTag + "connects");

						if (connects.size() != 0) {
							vector<string>connectsIds;
							for (int k = 0; k < connects.size(); k++) {
								DOMNode* connect = connects.at(k);
								string connectId = parseHelper->getNamedAttribute(connect->getAttributes(), "xlink:href");
								size_t pos;
								if ((pos = connectId.find("#")) != string::npos) {
									connectId = connectId.substr(1);
								}
								connectsIds.push_back(connectId);
							}
							transitionInstance.setConnects(connectsIds);
						}
						transitionList.push_back(transitionInstance);
					}
				}
			}

			DOMNode* nodes = 0;
			nextTag = frontTag + "nodes";
			nodes = parseHelper->getNamedNode(spaceLayer->getChildNodes(), nextTag);
			if (nodes != 0) {
				nextTag = frontTag + "stateMember";
				vector<DOMNode*>tempStates = parseHelper->getNamedNodes(nodes->getChildNodes(), nextTag);
				if (tempStates.size() != 0) {
					nextTag = frontTag + "State";
					for (int j = 0; j < tempStates.size(); j++) {
						DOMNode* state = parseHelper->getNamedNode(tempStates.at(j)->getChildNodes(), nextTag);
						DOMNode* stateGeometry = 0;
						stateGeometry = parseHelper->getNamedNode(state->getChildNodes(), frontTag + "geometry");
						gaia3d::Point3D stateGeometryPoint;
						stateGeometryPoint.set(0,0,0);
						if (stateGeometry != 0) {
							DOMNode* point = stateGeometry->getChildNodes()->item(1);
							for (int k = 0; k < point->getChildNodes()->getLength(); k++) {
								if (!parseHelper->isTextNode(point->getChildNodes()->item(k))) {
									double arr[3];
									string pointvalue = parseHelper->changeXMLCh2str(point->getChildNodes()->item(k)->getTextContent());
									stringstream ss(pointvalue);
									ss >> arr[0] >> arr[1] >> arr[2];
									stateGeometryPoint.set(arr[0],arr[1], arr[2]);
									break;
								}
							}
						}
						//get the information of the state
						IndoorGMLState stateInstance;
						string stateId = parseHelper->getNamedAttribute(state->getAttributes(), "gml:id");
						stateInstance.setId(stateId);

						//get the duality information of state
						DOMNode* duality = 0;
						duality = parseHelper->getNamedNode(state->getChildNodes(), frontTag + "duality");
						if (duality != 0){
							string dualityId = parseHelper->getNamedAttribute(duality->getAttributes(), "xlink:href");
							dualityId = dualityId.substr(1,dualityId.length());
							stateInstance.setDuality(dualityId);
						}

						//get the connects information of state
						vector<DOMNode*>connects;
						connects = parseHelper->getNamedNodes(state->getChildNodes(), frontTag + "connects");
						
						if (connects.size() != 0) {
							vector<string>connectsIds;
							for (int k = 0; k < connects.size(); k++) {
								DOMNode* connect = connects.at(k);
								string connectId = parseHelper->getNamedAttribute(connect->getAttributes(),"xlink:href");
								size_t pos;
								if ((pos = connectId.find("#")) != string::npos) {
									connectId = connectId.substr(1);
								}
								connectsIds.push_back(connectId);
							}
							stateInstance.setConnects(connectsIds);
						}
						stateInstance.setGeometry(stateGeometryPoint);
						stateList.push_back(stateInstance);
					
					}
					//states.insert(states.end(), tempStates.begin(), tempStates.end());
				}
			}
		}



		//cellSpaceMember -> cellSpace & cellSpaceBoundaryMember -> cellSpaceBoundary

		//state
		for (int i = 0; i < stateList.size(); i++) {
			double stateHeight = stateList.at(i).getGeometry().z;
			
			map<double, int>::iterator floorIt;
			floorIt = floorList.find(stateHeight);
			if (floorIt != floorList.end()) {
				stateList.at(i).setFloor(floorIt->second);
				floorListInfoMap[floorIt->second].addNode(stateList.at(i).getId());
			}
			else {
				floorIt = floorList.begin();
				for (; floorIt != floorList.end(); floorIt++) {
					if (abs(floorIt->first - stateHeight) < minimumGapHeight) {
						stateList.at(i).setFloor(floorIt->second);
						floorListInfoMap[floorIt->second].addNode(stateList.at(i).getId());
						break;
					}
				}
			}
		}

		//transition
		
		for (int i = 0; i < transitionList.size(); i++) {
			Point3D transitionLowerBoundPosition = transitionList.at(i).getLowerBoundPosition();
			Point3D transitionUpperBoundPosition = transitionList.at(i).getUpperBoundPosition();

			vector<int>result = searchFloors(floorList, minimumGapHeight, transitionLowerBoundPosition, transitionUpperBoundPosition);
			transitionList.at(i).setFloors(result);
			for (int j = 0; j < result.size(); j++) {
				floorListInfoMap[result.at(j)].addEdge(transitionList.at(i).getId());
			}	
		}


		geomManager.bb = *b;

		gaia3d::Point3D boundingBoxCenter;
		boundingBoxCenter.set((b->minX+b->maxX)*unitScaleFactor/2,(b->minY+b->maxY)*unitScaleFactor/2,(b->minZ+b->maxZ)*unitScaleFactor/2);

		Json::Value objectNode(Json::objectValue);
		map<string, gaia3d::Point3D>::iterator iter = cellSpaceCenterLowerPointList.begin();
		for (; iter != cellSpaceCenterLowerPointList.end(); iter++) {
			string dataKey = iter->first;
			gaia3d::Point3D dataValue = iter->second;

			Json::Value cellspace(Json::objectValue);
			Json::Value point(Json::objectValue);

			//usf, bbc 적용해야 함
			point["x"] = dataValue.x * unitScaleFactor - boundingBoxCenter.x;
			point["y"] = dataValue.y * unitScaleFactor - boundingBoxCenter.y;
			point["z"] = dataValue.z * unitScaleFactor - boundingBoxCenter.z;
			objectNode[dataKey] = point;

			//objectNode.append(cellspace);
		}

		Json::StyledWriter writer;
		std::string documentContent = writer.write(objectNode);
		std::string lonLatFileFullPath = outputFolderPath + std::string("/cellspacelist.json");
		FILE* file = NULL;
		file = fopen(lonLatFileFullPath.c_str(), "wt");
		std::fprintf(file, "%s", documentContent.c_str());
		std::fclose(file);

		Json::Value networkRootObjectNode(Json::objectValue);
		Json::Value nodesObjectNode(Json::objectValue);
		for (int i = 0; i < stateList.size(); i++) {
			string dataKey = stateList.at(i).getId();
			
			Json::Value state(Json::objectValue);
			Json::Value connects(Json::arrayValue);
			

			if (stateList.at(i).getDuality() != "") {
				state["duality"] = stateList.at(i).getDuality();
			}

			if (stateList.at(i).getConnects().size() != 0) {
				string* connectsArr = new string[stateList.at(i).getConnects().size()];
				for (int j = 0; j < stateList.at(i).getConnects().size(); j++) {
					connects.append(stateList.at(i).getConnects().at(j));
				}
				state["connects"] = connects;
			}

			//usf, bbc 적용해야 함
			Point3D scaledPoint;
			scaledPoint.set(
			stateList.at(i).getGeometry().x * unitScaleFactor - boundingBoxCenter.x,
			stateList.at(i).getGeometry().y * unitScaleFactor - boundingBoxCenter.y,
			stateList.at(i).getGeometry().z * unitScaleFactor - boundingBoxCenter.z
			);
			
			state["geometry"] = changePoint2GeoJSONPoint(scaledPoint);
			nodesObjectNode[dataKey] = state;
		}
		
		networkRootObjectNode["nodes"] = nodesObjectNode;
		Json::Value edgesObjectNode(Json::objectValue);

		for (int i = 0; i < transitionList.size(); i++) {
			string dataKey = transitionList.at(i).getId();

			Json::Value transition(Json::objectValue);
			Json::Value connects(Json::arrayValue);

			if (transitionList.at(i).getDuality() != "") {
				transition["duality"] = transitionList.at(i).getDuality();
			}

			if (transitionList.at(i).getConnects().size() != 0) {
				Json::Value directedConnects(Json::arrayValue);
				Json::Value reversedDirectedConnects(Json::arrayValue);
				directedConnects.append(transitionList.at(i).getConnects().at(0));
				directedConnects.append(transitionList.at(i).getConnects().at(1));
				connects.append(directedConnects);
				reversedDirectedConnects.append(transitionList.at(i).getConnects().at(1));
				reversedDirectedConnects.append(transitionList.at(i).getConnects().at(0));
				connects.append(reversedDirectedConnects);

				transition["connects"] = connects;
			}

			shared_ptr<IndoorGMLLineString> tempLineString = transitionList.at(i).getGeometry();
			vector<Point3D>tempVerticesList = tempLineString->getVertices();
			
			if (tempVerticesList.size() != 0) {
				for (int j = 0; j < tempVerticesList.size(); j++) {
					tempVerticesList.at(j).set(
						tempVerticesList.at(j).x * unitScaleFactor - boundingBoxCenter.x,
						tempVerticesList.at(j).y * unitScaleFactor - boundingBoxCenter.y,
						tempVerticesList.at(j).z * unitScaleFactor - boundingBoxCenter.z
					);
				}
				IndoorGMLLineString tempLineStringInstance;
				tempLineStringInstance.setVertices(tempVerticesList);
				transition["geometry"] = changeLineString2GeoJsonLineString(tempLineStringInstance);
			}

			edgesObjectNode[dataKey] = transition;
		}

		networkRootObjectNode["edges"] = edgesObjectNode;

		Json::Value propertiesObjectNode(Json::objectValue);
		Json::Value floorsObjectNode(Json::objectValue);

		

		if (floorListInfoMap.size() != 0) {
			map<int, IndoorFloor>::iterator floorListInfoMapIt = floorListInfoMap.begin();
			for (; floorListInfoMapIt != floorListInfoMap.end(); floorListInfoMapIt++) {
				Json::Value floorObjectNode(Json::objectValue);
				vector<string> nodes = floorListInfoMapIt->second.getNodes();
				vector<string> edges = floorListInfoMapIt->second.getEdges();
				floorListInfoMapIt->second.getHeight();
				Point3D floorRepresentativePoint;
				floorRepresentativePoint.set(b->minX - boundingBoxCenter.x,b->minY - boundingBoxCenter.y,floorListInfoMapIt->second.getHeight());
				floorObjectNode["representativePoint"] = changePoint2GeoJSONPoint(floorRepresentativePoint);
				if (nodes.size() != 0) {
					Json::Value nodesArrayNode(Json::arrayValue);
					for (int i = 0; i < nodes.size(); i++) {
						nodesArrayNode.append(nodes.at(i));
					}
					floorObjectNode["nodes"] = nodesArrayNode;
				}

				if (edges.size() != 0) {
					Json::Value edgesArrayNode(Json::arrayValue);
					for (int i = 0; i < edges.size(); i++) {
						edgesArrayNode.append(edges.at(i));
					}
					floorObjectNode["edges"] = edgesArrayNode;
				}
				int floorKey = floorListInfoMapIt->first;
				floorsObjectNode[std::to_string(floorKey)] = floorObjectNode;
			}
			propertiesObjectNode["floors"] = floorsObjectNode;
		}
		networkRootObjectNode["properties"] = propertiesObjectNode;

		documentContent = writer.write(networkRootObjectNode);
		lonLatFileFullPath = outputFolderPath + std::string("/statelist.json");
		file = NULL;
		file = fopen(lonLatFileFullPath.c_str(), "wt");
		std::fprintf(file, "%s", documentContent.c_str());
		std::fclose(file);


		//std::cout << "IndoorGML Parser : The Document is parsed" << endl;
	}
	catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
	}
	catch (const DOMException& toCatch) {
		char* message = XMLString::transcode(toCatch.msg);
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
	}
	catch (const SAXParseException& ex) {
		std::cout << XMLString::transcode(ex.getMessage()) << endl;

	}
	catch (...) {
		std::cout << "Unexpected Exception \n";
	}
	

	
	return geomManager;
}
bool IndoorGMLReader::readIndoorSpace(DOMDocument* dom, std::vector<gaia3d::TrianglePolyhedron*>& container, double& lon, double& lat) {
	


	GeometryManager geomManager = parseIndoorGeometry(dom);
	//cout << "start read IndoorGML data" << endl;

	//gaia3d::Matrix4* mat;
	std::string wgs84ProjString("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
	projPJ pjWgs84 = pj_init_plus(wgs84ProjString.c_str());
	gaia3d::Point3D BBcenterPoint;

	geomManager.bb.minX *= unitScaleFactor;
	geomManager.bb.minY *= unitScaleFactor;
	geomManager.bb.minZ *= unitScaleFactor;
	geomManager.bb.maxX *= unitScaleFactor;
	geomManager.bb.maxY *= unitScaleFactor;
	geomManager.bb.maxZ *= unitScaleFactor;

	gaia3d::Point3D lowerBound;
	lowerBound.set(geomManager.bb.minX, geomManager.bb.minY, geomManager.bb.minZ);
	gaia3d::Point3D upperBound;
	upperBound.set(geomManager.bb.maxX, geomManager.bb.maxY, geomManager.bb.maxZ);

	//std::cout << geomManager.bb.minX << "," << geomManager.bb.minY << "," << geomManager.bb.minZ << endl;
	//std::cout << geomManager.bb.maxX << "," << geomManager.bb.maxY << "," << geomManager.bb.maxZ << endl;
	
	projPJ pjSrc = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

	if (pjSrc == NULL || pjWgs84 == NULL)
	{
		printf("[ERROR][proj4]CANNOT initialize SRS\n");
		return false;
	}

	BBcenterPoint.set((lowerBound.x+upperBound.x)/2.0, (lowerBound.y + upperBound.y) / 2.0, (lowerBound.z + upperBound.z) / 2.0);
	
	//lon = BBcenterPoint.x;
	//lat = BBcenterPoint.y;

	double alt = BBcenterPoint.z;

	//lon *= RAD_TO_DEG;
	//lat *= RAD_TO_DEG;

	vector<gaia3d::Triangle>tessellatedResult;
	
	for (size_t i = 0; i < geomManager.getIndoorGMLSolidsCount(); i++) {

		gaia3d::TrianglePolyhedron* newMesh = new gaia3d::TrianglePolyhedron();

		shared_ptr<IndoorGMLSolid> tempIndoorGMLSolid = geomManager.getIndoorGMLSolid(i);
		for (size_t j = 0; j < tempIndoorGMLSolid->getExterior().size(); j++) {
			size_t offset = newMesh->getVertices().size();
			
			Surface* tempSurface = new Surface();
			shared_ptr<IndoorGMLPolygon>tempIndoorGMLPolygon = tempIndoorGMLSolid->getExterior().at(j);
			size_t verticesCount = tempIndoorGMLPolygon->getExterior()->getVertices().size();
			vector<Point3D>tempPointList = tempIndoorGMLPolygon->getExterior()->getVertices();

			double* nx = new double[verticesCount];
			double* ny = new double[verticesCount];
			double* nz = new double[verticesCount];
			
			vector<size_t>verticesIndices;
			vector<size_t>polygonIndices;

			if (tempIndoorGMLSolid->getExterior().at(j)->getInterior().size() != 0) {}
			else {
				for (size_t z = 0; z < verticesCount; z++) {
					polygonIndices.push_back(z);
				}
			}

			// push new vertex at the vertex list of the surface
			for (size_t z = 0; z < verticesCount; z++) {

				nx[z] = tempPointList.at(z).x * unitScaleFactor - BBcenterPoint.x;
				ny[z] = tempPointList.at(z).y * unitScaleFactor - BBcenterPoint.y;
				nz[z] = tempPointList.at(z).z * unitScaleFactor - BBcenterPoint.z;
				Vertex* tempVertex = new Vertex();
				tempVertex->position.set(nx[z], ny[z], nz[z]);
				newMesh->getVertices().push_back(tempVertex);
			}

			//GeometryUtility::tessellate(nx, ny, nz, verticesCount, verticesIndices);
			GeometryUtility::tessellate(nx, ny, nz, verticesCount, polygonIndices, verticesIndices);

			// Cause of dealing single IndoorGMLSolid as one single model. add offset and set the vertices indices of the new vertices
			for (size_t z = 0; z < verticesIndices.size(); z++) {
				size_t tempIndex = verticesIndices.at(z);
				tempIndex += offset;
				verticesIndices.at(z) = tempIndex;
			}

			//Create triangle 
			for (size_t z = 0; z < verticesIndices.size(); z += 3) {
				Vertex* v1 = newMesh->getVertices().at(verticesIndices.at(z));
				Vertex* v2 = newMesh->getVertices().at(verticesIndices.at(z + 1));
				Vertex* v3 = newMesh->getVertices().at(verticesIndices.at(z + 2));

				Point3D vec1 = v1->position - v2->position;
				Point3D vec2 = v1->position - v3->position;

				//if the sum of the each element of the cross product vector between 2 edges of the triangle, then those of the edges are parrallel
				Point3D crossProductOfTriangle = vec1 ^ vec2;
				double tolerance = 1E-6;
				double checkParallelValue = crossProductOfTriangle.x + crossProductOfTriangle.y + crossProductOfTriangle.z;
				if (abs(checkParallelValue) < tolerance)
					continue;

				double r1 = 0, r2 = 0, r3 = 0;
				GeometryUtility::calculatePlaneNormal(
					v1->position.x, v1->position.y, v1->position.z,
					v2->position.x, v2->position.y, v2->position.z,
					v3->position.x, v3->position.y, v3->position.z,
					r1, r2, r3, true
				);
				v1->normal.set(r1,r2,r3);
				v2->normal.set(r1,r2,r3);
				v3->normal.set(r1, r2, r3);

				Triangle* resultTriangle = new Triangle();
				resultTriangle->setNormal(r1, r2, r3);
				resultTriangle->setVertexIndices(verticesIndices.at(z), verticesIndices.at(z + 1), verticesIndices.at(z + 2));
				resultTriangle->setVertices(v1,v2,v3);
				tempSurface->getTriangles().push_back(resultTriangle);
	
			}

			newMesh->getSurfaces().push_back(tempSurface);
			newMesh->addStringAttribute(std::string(ObjectGuid), tempIndoorGMLSolid->getId());
			delete[] nx;
			delete[] ny;
			delete[] nz;
		}

		newMesh->setHasNormals(true);
		newMesh->setId(container.size());
		newMesh->setColorMode(SingleColor);
		newMesh->setSingleColor(MakeColorU4(250, 250, 250));
	
		
		container.push_back(newMesh);

	}

	return true;

}
void IndoorGMLReader::clear()
{
	container.clear();

	//textureContainer.clear();
}
bool IndoorGMLReader::readRawDataFile(std::string& filePath) {
	try {
		const char * xmlFile = filePath.c_str();
		XMLPlatformUtils::Initialize();
		XercesDOMParser* parser = new XercesDOMParser();
		ParserUtil* parseHelper = new ParserUtil();
		GeometryParser* gmp = new GeometryParser();
		
		ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
		parser->setErrorHandler(errHandler);
		parser->setIncludeIgnorableWhitespace(false);
		parser->setDoSchema(true);
		//const char * xmlFile = "../samples/seouluniv21centry.gml";
		parser->parse(xmlFile);

		//cout << xmlFile << ": parse OK" << endl;
		DOMDocument* dom = parser->getDocument();	
		//cout << "Now processing start" << endl;
		readIndoorSpace(dom, container, refLon, refLat);

		delete parser;
		delete errHandler;
		delete parseHelper;
		XMLPlatformUtils::Terminate();
	

	}
	catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return false;
	}
	catch (const DOMException& toCatch) {
		char* message = XMLString::transcode(toCatch.msg);
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return false;
	}
	catch (const SAXParseException& ex) {
		std::cout << XMLString::transcode(ex.getMessage()) << endl;

	}
	catch (...) {
		std::cout << "Unexpected Exception \n";
		return false;
	}

	return true;

}