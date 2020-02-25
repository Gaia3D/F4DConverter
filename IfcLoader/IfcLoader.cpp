﻿// IfcLoader.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "IfcLoader.h"

#include "ifcpp/model/IfcPPModel.h"
#include "ifcpp/geometry/GeometryConverter.h"
#include "ifcpp/reader/IfcPPReaderSTEP.h"
#include "ifcpp/IFC4/include/IfcSite.h"
#include "ifcpp/IFC4/include/IfcSpace.h"
// for spatial structure
#include "ifcpp/IFC4/include/IfcBuilding.h"
#include "ifcpp/IFC4/include/IfcBuildingStorey.h"
#include "ifcpp/IFC4/include/IfcFooting.h"
#include "ifcpp/IFC4/include/IfcColumn.h"
#include "ifcpp/IFC4/include/IfcSlab.h"
#include "ifcpp/IFC4/include/IfcBeam.h"
#include "ifcpp/IFC4/include/IfcWall.h"
#include "ifcpp/IFC4/include/IfcWallStandardCase.h"
//

// for property value
#include "ifcpp/IFC4/include/IfcAreaMeasure.h"
#include "ifcpp/IFC4/include/IfcBoolean.h"
#include "ifcpp/IFC4/include/IfcIdentifier.h"
#include "ifcpp/IFC4/include/IfcInteger.h"
#include "ifcpp/IFC4/include/IfcLabel.h"
#include "ifcpp/IFC4/include/IfcText.h"
#include "ifcpp/IFC4/include/IfcLengthMeasure.h"
#include "ifcpp/IFC4/include/IfcPlaneAngleMeasure.h"
#include "ifcpp/IFC4/include/IfcPositiveLengthMeasure.h"
#include "ifcpp/IFC4/include/IfcReal.h"
#include "ifcpp/IFC4/include/IfcVolumeMeasure.h"

// for unit
#include "ifcpp/IFC4/include/IfcDerivedUnit.h"
#include "ifcpp/IFC4/include/IfcNamedUnit.h"
#include "ifcpp/IFC4/include/IfcMonetaryUnit.h"
#include "ifcpp/IFC4/include/IfcUnitEnum.h"

#include "./json/json.h"



class MessageWrapper
{
public:
	static void slotMessageWrapper(void* obj_ptr, shared_ptr<StatusCallback::Message> m)
	{
		if (m)
		{
			if (m->m_message_type != StatusCallback::MESSAGE_TYPE_PROGRESS_VALUE && m->m_message_type != StatusCallback::MESSAGE_TYPE_PROGRESS_TEXT)
			{
				std::wcout << m->m_message_text << std::endl;
			}
		}
	}
};

class IfcLoader : public aIfcLoader
{
public:
	IfcLoader();
	virtual ~IfcLoader();

public:
	virtual bool loadIfcFile(std::wstring& filePath);

	virtual void setVertexReductionMode(bool bOn);

	virtual size_t getPolyhedronCount();
	virtual float* getRepresentativeColor(size_t polyhedronIndex);
	virtual void getGuid(size_t polyhedronIndex, wchar_t buffer[]);
	virtual size_t getVertexCount(size_t polyhedronIndex);
	virtual double* getVertexPositions(size_t polyhedronIndex);
	virtual size_t getSurfaceCount(size_t polyhedronIndex);
	virtual size_t getTrialgleCount(size_t polyhedronIndex, size_t surfaceIndex);
	virtual size_t* getTriangleIndices(size_t polyhedronIndex, size_t surfaceIndex);

	virtual size_t getStoryCount();
	virtual size_t getStoryDivisionCount(size_t storyIndex);
	virtual size_t getPolyhedronCount(size_t storyIndex, size_t divisionIndex);
	virtual float* getRepresentativeColor(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex);
	virtual void getGuid(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex, wchar_t buffer[]);
	virtual size_t getVertexCount(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex);
	virtual double* getVertexPositions(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex);
	virtual size_t getSurfaceCount(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex);
	virtual size_t getTrialgleCount(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex, size_t surfaceIndex);
	virtual size_t* getTriangleIndices(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex, size_t surfaceIndex);

	virtual bool loadOnlyPropertiesFromIfc(std::wstring& filePath);
	virtual void setAttributesExtraction(bool bOn);
	virtual std::string getObjectAttributes();
	virtual std::string getProjectAttributes();

#ifdef TMPTEST
	virtual size_t getAttributeTypeCount();
	virtual std::string getAttributeType(size_t i);
	std::vector<std::string> attributeTypes;
	std::map<std::string, std::string> attributeMap;
#endif

private:

	void loadProjectAttributes();
	void loadObjectAttributes(shared_ptr<IfcProduct> ifcProduct, Json::Value& root);
	bool checkIfPropertiesCanBeExtracted(IfcPPEntityEnum ppEnum);
	void parsePropertySingleValue(Json::Value& valueObject, shared_ptr<IfcValue> value);
	std::string convertWideStringToUtf8(std::wstring& sourceString);

	bool bAttributesExtraction;

	bool bVertexReduction;

	struct Surface
	{
		size_t triangleCount;
		size_t* triangleIndices;
	};

	struct Polyhedron
	{
		size_t vertexCount;
		double* vertices;
		float color[4];
		std::vector<Surface*> surfaces;
		std::wstring guid;
	};

	std::vector<Polyhedron*> polyhedrons;

	std::vector<std::vector<std::vector<Polyhedron*>>> stories;
	
	std::vector<Polyhedron*> objectsOutsideStory;

	Json::Value objectPropertyRoot;
	Json::Value projectPropertyRoot;
};

IfcLoader::IfcLoader()
{
	bVertexReduction = false;
	bAttributesExtraction = false;

	Json::Value projectProperty(Json::objectValue);
	projectPropertyRoot["project"] = projectProperty;

	Json::Value objectPropertyList(Json::arrayValue);
	objectPropertyRoot["objects"] = objectPropertyList;
}

IfcLoader::~IfcLoader()
{
	size_t polyhedronCount = polyhedrons.size();
	size_t surfaceCount;
	for (size_t i = 0; i < polyhedronCount; i++)
	{
		delete[] polyhedrons[i]->vertices;
		surfaceCount = polyhedrons[i]->surfaces.size();
		for (size_t j = 0; j < surfaceCount; j++)
		{
			delete[] polyhedrons[i]->surfaces[j]->triangleIndices;
			delete polyhedrons[i]->surfaces[j];
		}

		polyhedrons[i]->surfaces.clear();

		delete polyhedrons[i];
	}

	polyhedrons.clear();
}

bool IfcLoader::loadIfcFile(std::wstring& filePath)
{
	// initializing
	shared_ptr<MessageWrapper> mw(new MessageWrapper());
	shared_ptr<IfcPPModel> ifc_model(new IfcPPModel());
	shared_ptr<GeometryConverter> geometry_converter(new GeometryConverter(ifc_model));
	shared_ptr<IfcPPReaderSTEP> reader(new IfcPPReaderSTEP());

	reader->setMessageCallBack(mw.get(), &MessageWrapper::slotMessageWrapper);
	geometry_converter->setMessageCallBack(mw.get(), &MessageWrapper::slotMessageWrapper);

	// loading
	reader->loadModelFromFile(filePath, ifc_model);

	// relationship information
	// 0. basic preparation
	std::map<double, std::map<unsigned int, std::vector<std::wstring>>> guidToStoryMapper;
	// 1. project
	shared_ptr<IfcProject> ifc_project =  ifc_model->getIfcProject();
	// 2. project's children
	std::vector<weak_ptr<IfcRelAggregates> > projectChildren =   ifc_project->m_IsDecomposedBy_inverse;
	for (size_t i = 0; i < projectChildren.size(); i++)
	{
		// 2-1. project's child
		shared_ptr<IfcRelAggregates> projectChild = projectChildren[i].lock();
		// 3. project's grandchildren
		std::vector<shared_ptr<IfcObjectDefinition>> projectChildObjectDefinitions = projectChild->m_RelatedObjects;
		for (size_t j = 0; j < projectChildObjectDefinitions.size(); j++)
		{
			// 3-1. project's grandchild
			shared_ptr<IfcObjectDefinition> projectChilidObjectDefinition = projectChildObjectDefinitions[j];

			// 3-2. site as project's grandchild type
			shared_ptr<IfcSite> ifc_site = dynamic_pointer_cast<IfcSite>(projectChilidObjectDefinition);
			if (ifc_site != NULL)
			{
				// 4. site's children
				std::vector<weak_ptr<IfcRelAggregates>> siteChildren =  ifc_site->m_IsDecomposedBy_inverse;
				for (size_t k = 0; k < siteChildren.size(); k++)
				{
					// 4-1. site's child
					shared_ptr<IfcRelAggregates> siteChild = siteChildren[k].lock();
					// 5. site's grandchildren
					std::vector<shared_ptr<IfcObjectDefinition>> siteChildObjectDefinitions = siteChild->m_RelatedObjects;
					for (size_t ii = 0; ii < siteChildObjectDefinitions.size(); ii++)
					{
						// 5-1. site's grandchild
						shared_ptr<IfcObjectDefinition> siteChilidObjectDefinition = siteChildObjectDefinitions[ii];
						
						// 5-2. building as site's grandchild type
						shared_ptr<IfcBuilding> ifc_building = dynamic_pointer_cast<IfcBuilding>(siteChilidObjectDefinition);
						if (ifc_building != NULL)
						{
							for (size_t jj = 0; jj < ifc_building->m_ContainsElements_inverse.size(); jj++)
							{
								shared_ptr<IfcRelContainedInSpatialStructure> buildingElements = ifc_building->m_ContainsElements_inverse[jj].lock();
							}

							// 6. building's children
							for (size_t jj = 0; jj < ifc_building->m_IsDecomposedBy_inverse.size(); jj++)
							{
								// 6-1. building's child
								shared_ptr<IfcRelAggregates> buildingChild = ifc_building->m_IsDecomposedBy_inverse[jj].lock();
								// 7. building's grandchildren
								std::vector<shared_ptr<IfcObjectDefinition>> buildingChildObjectDefinitions = buildingChild->m_RelatedObjects;
								for (size_t kk = 0; kk < buildingChildObjectDefinitions.size(); kk++)
								{
									// 7-1. building's grandchild
									shared_ptr<IfcObjectDefinition> buildingChildObjectDefinition = buildingChildObjectDefinitions[kk];
									
									// 7-2. building storey as building's grandchild type
									shared_ptr<IfcBuildingStorey> buildingStorey = dynamic_pointer_cast<IfcBuildingStorey>(buildingChildObjectDefinition);
									if (buildingStorey != NULL)
									{
										// initialize guidToStoryMapper
										guidToStoryMapper[buildingStorey->m_Elevation->m_value] = std::map<unsigned int, std::vector<std::wstring>>();

										// 8-1. elements in building storey
										for (size_t iii = 0; iii < buildingStorey->m_ContainsElements_inverse.size(); iii++)
										{
											shared_ptr<IfcRelContainedInSpatialStructure> element = buildingStorey->m_ContainsElements_inverse[iii].lock();
											
											// 8-1-1. products(element's children)
											for (size_t jjj = 0; jjj < element->m_RelatedElements.size(); jjj++)
											{
												// 9. product(finally, an single object)
												shared_ptr<IfcProduct> product = element->m_RelatedElements[jjj];

												if (dynamic_pointer_cast<IfcFooting>(product) != NULL || dynamic_pointer_cast<IfcColumn>(product) != NULL)
												{
													if (guidToStoryMapper[buildingStorey->m_Elevation->m_value].find(0) == guidToStoryMapper[buildingStorey->m_Elevation->m_value].end())
														guidToStoryMapper[buildingStorey->m_Elevation->m_value][0] = std::vector<std::wstring>();

													guidToStoryMapper[buildingStorey->m_Elevation->m_value][0].push_back( dynamic_pointer_cast<IfcElement>(product)->m_Tag->m_value);
													continue;
												}

												if (dynamic_pointer_cast<IfcSlab>(product) != NULL || dynamic_pointer_cast<IfcBeam>(product) != NULL)
												{
													if (guidToStoryMapper[buildingStorey->m_Elevation->m_value].find(1) == guidToStoryMapper[buildingStorey->m_Elevation->m_value].end())
														guidToStoryMapper[buildingStorey->m_Elevation->m_value][1] = std::vector<std::wstring>();

													guidToStoryMapper[buildingStorey->m_Elevation->m_value][1].push_back(dynamic_pointer_cast<IfcElement>(product)->m_Tag->m_value);
													continue;
												}

												if (dynamic_pointer_cast<IfcWall>(product) != NULL || dynamic_pointer_cast<IfcWallStandardCase>(product) != NULL)
												{
													if (guidToStoryMapper[buildingStorey->m_Elevation->m_value].find(2) == guidToStoryMapper[buildingStorey->m_Elevation->m_value].end())
														guidToStoryMapper[buildingStorey->m_Elevation->m_value][2] = std::vector<std::wstring>();

													guidToStoryMapper[buildingStorey->m_Elevation->m_value][2].push_back(dynamic_pointer_cast<IfcElement>(product)->m_Tag->m_value);
													continue;
												}

												if (std::string(product->className()) == std::string("IfcAnnotation"))
													continue;

												if (guidToStoryMapper[buildingStorey->m_Elevation->m_value].find(3) == guidToStoryMapper[buildingStorey->m_Elevation->m_value].end())
													guidToStoryMapper[buildingStorey->m_Elevation->m_value][3] = std::vector<std::wstring>();

												guidToStoryMapper[buildingStorey->m_Elevation->m_value][3].push_back(dynamic_pointer_cast<IfcElement>(product)->m_Tag->m_value);
											}
										}

										// 8-2. building storey's children;
										for (size_t iii = 0; iii < buildingStorey->m_IsDecomposedBy_inverse.size(); iii++)
										{
											// 8-2-1. building storey's child
											shared_ptr<IfcRelAggregates> storeyChild = buildingStorey->m_IsDecomposedBy_inverse[iii].lock();
											// 9. building storey's grandchildren
											std::vector<shared_ptr<IfcObjectDefinition>> storeyChildObjectDefinitions = storeyChild->m_RelatedObjects;
											for (size_t jjj = 0; jjj < storeyChildObjectDefinitions.size(); jjj++)
											{
												// 9-1. building storey's grandchild
												shared_ptr<IfcObjectDefinition> storeyChildObjectDefinition = storeyChildObjectDefinitions[jjj];

												// 9-2. space as building storey's grandchild type
												shared_ptr<IfcSpace> space = dynamic_pointer_cast<IfcSpace>(storeyChildObjectDefinition);
												if (space != NULL)
												{
													// 10. elements as space's childtren
													for (size_t kkk = 0; kkk < space->m_ContainsElements_inverse.size(); kkk++)
													{
														// 10-1. element(space's child)
														shared_ptr<IfcRelContainedInSpatialStructure> element =  space->m_ContainsElements_inverse[kkk].lock();

														// 11. products(element's children)
														for (size_t iiii = 0; iiii < element->m_RelatedElements.size(); iiii++)
														{
															// 12. product(finally, an single object)
															shared_ptr<IfcProduct> product = element->m_RelatedElements[iiii];

															if (std::string(product->className()) == std::string("IfcAnnotation"))
																continue;

															if (guidToStoryMapper[buildingStorey->m_Elevation->m_value].find(4) == guidToStoryMapper[buildingStorey->m_Elevation->m_value].end())
																guidToStoryMapper[buildingStorey->m_Elevation->m_value][4] = std::vector<std::wstring>();

															guidToStoryMapper[buildingStorey->m_Elevation->m_value][4].push_back(dynamic_pointer_cast<IfcElement>(product)->m_Tag->m_value);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	std::map<std::wstring, std::vector<Polyhedron*>> polyhedronToGuidMapper;

	// conversion raw data into geometries of OSG type
	osg::ref_ptr<osg::Switch> model_switch = new osg::Switch();
	geometry_converter->createGeometryOSG(model_switch);

	// contains the VEF graph for each IfcProduct:
	std::map<int, shared_ptr<ProductShapeInputData> >& map_vef_data = geometry_converter->getShapeInputData();
	double volume_all_products = 0;

	std::map<int, shared_ptr<ProductShapeInputData> >::iterator it;
	for (it = map_vef_data.begin(); it != map_vef_data.end(); ++it)
		//for (auto it = map_vef_data.begin(); it != map_vef_data.end(); ++it)
	{
		// STEP entity id:
		int entity_id = it->first;

		// shape data
		shared_ptr<ProductShapeInputData>& shape_data = it->second;

		// IfcProduct(abstract type)
		shared_ptr<IfcProduct> ifc_product(shape_data->m_ifc_product);

		// filtering out IfcProduct of IfcSpace type
		shared_ptr<IfcSpace> Ifc_Space = dynamic_pointer_cast<IfcSpace>(ifc_product);
		if (Ifc_Space != NULL)
			continue;

		// filtering out IfcProduct of IfcSite type
		shared_ptr<IfcSite> site_elem = dynamic_pointer_cast<IfcSite>(ifc_product);
		if (site_elem != NULL)
			continue;

		// extract basic attributes of this IfcProduct
		std::string productEntityName = ifc_product->className();
		std::wstring productLabel = (ifc_product->m_Name != NULL) ? ifc_product->m_Name->m_value : L"";
		std::wstring productDescription = (ifc_product->m_Description != NULL)? ifc_product->m_Description->m_value : L"";
		std::wstring productGuid, productIdentifier;
		std::vector<std::pair<std::string, shared_ptr<IfcPPObject> > > vec_attributes;
		std::string ifcAttributeTitle, ifcAttributeClassName;
		ifc_product->getAttributes(vec_attributes);
		for (size_t i_attr = 0; i_attr < vec_attributes.size(); i_attr++)
		{
			ifcAttributeTitle = vec_attributes[i_attr].first;

			shared_ptr<IfcPPObject> ifcPpObj = vec_attributes[i_attr].second;
			if (ifcPpObj == NULL)
				continue;

			ifcAttributeClassName = ifcPpObj->className();
			if (ifcAttributeClassName.compare("IfcGloballyUniqueId") == 0)
				productGuid = dynamic_pointer_cast<IfcGloballyUniqueId>(ifcPpObj)->m_value;
			else if (ifcAttributeClassName.compare("IfcIdentifier") == 0)
				productIdentifier = dynamic_pointer_cast<IfcElement>(ifc_product)->m_Tag->m_value;
		}

		// for each IfcProduct, there can be mulitple geometric representation items:
		std::vector<shared_ptr<ProductRepresentationData> >& vec_representations = shape_data->m_vec_representations;
		for (size_t i_representation = 0; i_representation < vec_representations.size(); ++i_representation)
		{
			shared_ptr<ProductRepresentationData>& representation_data = vec_representations[i_representation];

			// a representation item can have multiple item shapes
			std::vector<shared_ptr<ItemShapeInputData> >& vec_item_data = representation_data->m_vec_item_data;
			for (size_t i_item = 0; i_item < vec_item_data.size(); ++i_item)
			{
				shared_ptr<ItemShapeInputData>& item_data = vec_item_data[i_item];

				// appearance data for getting color
				std::vector<shared_ptr<AppearanceData> > vec_items_appearances = item_data->m_vec_item_appearances;

				std::vector<shared_ptr<carve::mesh::MeshSet<3> > > allMeshsets;
				allMeshsets.insert(allMeshsets.end(), item_data->m_meshsets.begin(), item_data->m_meshsets.end());
				allMeshsets.insert(allMeshsets.end(), item_data->m_meshsets_open.begin(), item_data->m_meshsets_open.end());
				Polyhedron* polyhedron;
				Surface* surface;
				for (size_t i_meshset = 0; i_meshset < allMeshsets.size(); ++i_meshset)
				{
					// A meshset == A Polyhedron
					shared_ptr<carve::mesh::MeshSet<3> >& meshset = allMeshsets[i_meshset];
					polyhedron = NULL;

					////////////////////////////////////
					if (bVertexReduction)
					{
						// vertices of this meshset(to add all vertices to a Polyhedron)
						std::map<carve::mesh::Vertex<3> *, size_t> vertices;
						std::map<carve::mesh::Vertex<3> *, size_t>::iterator vertexIter;

						// A meshset is composed of multiple meshes.( mesh = surface)
						std::vector<carve::mesh::Mesh<3>* >& vec_meshes = meshset->meshes;
						for (size_t i_mesh = 0; i_mesh < vec_meshes.size(); ++i_mesh)
						{
							// Mesh == Surface
							carve::mesh::Mesh<3>* mesh = vec_meshes[i_mesh];
							surface = NULL;

							// vertex indices to be used to compose of triangles
							std::vector<size_t> indices;

							// A mesh is composed of multiple faces. (face == triangle)
							std::vector<carve::mesh::Face<3>* >& vec_faces = mesh->faces;
							for (size_t i_face = 0; i_face < vec_faces.size(); ++i_face)
							{
								// Face == Triangle
								carve::mesh::Face<3>* face = vec_faces[i_face];

								// iterate through edges:
								carve::mesh::Edge<3>* edge = face->edge;
								size_t vertexIndex;
								do
								{
									// start vertices of each edge
									// 각 edge의 start vertext만 instance화 한다.
									// 왜냐하면 한 edge의 end vertex는 다음 edge의 start vertex이므로.
									carve::mesh::Vertex<3>* vertex_begin = edge->v1();

									vertexIter = vertices.find(vertex_begin);
									if (vertexIter == vertices.end())
									{
										vertexIndex = vertices.size();
										vertices.insert(std::map<carve::mesh::Vertex<3>*, size_t>::value_type(vertex_begin, vertexIndex));
									}
									else
										vertexIndex = vertexIter->second;

									indices.push_back(vertexIndex);

									// 각 edge의 start vertext만 instance화 한다.
									// 왜냐하면 한 edge의 end vertex는 다음 edge의 start vertex이므로.

									edge = edge->next;
								} while (edge != face->edge);
							}

							// fill triangle count and triangle vertex indices
							if (indices.size() / 3 == 0)
								continue;

							surface = new Surface;

							surface->triangleCount = indices.size() / 3;
							surface->triangleIndices = new size_t[3 * surface->triangleCount];
							memset(surface->triangleIndices, 0x00, sizeof(size_t) * 3 * surface->triangleCount);
							for (size_t iIndex = 0; iIndex < 3 * surface->triangleCount; iIndex++)
								surface->triangleIndices[iIndex] = indices[iIndex];

							// add this surface to this polyhedron
							if (polyhedron == NULL)
								polyhedron = new Polyhedron;

							polyhedron->surfaces.push_back(surface);
						}

						if (polyhedron == NULL)
							continue;

						// fill vertex count and vertex position information
						std::map<size_t, carve::mesh::Vertex<3> *> verticesSorted;
						for (vertexIter = vertices.begin(); vertexIter != vertices.end(); vertexIter++)
							verticesSorted.insert(std::map<size_t, carve::mesh::Vertex<3> *>::value_type(vertexIter->second, vertexIter->first));

						polyhedron->vertexCount = verticesSorted.size();
						polyhedron->vertices = new double[3 * polyhedron->vertexCount];
						memset(polyhedron->vertices, 0x00, sizeof(double) * 3 * polyhedron->vertexCount);
						for (size_t vIndex = 0; vIndex < polyhedron->vertexCount; vIndex++)
						{
							polyhedron->vertices[3 * vIndex] = verticesSorted[vIndex]->v.x;
							polyhedron->vertices[3 * vIndex + 1] = verticesSorted[vIndex]->v.y;
							polyhedron->vertices[3 * vIndex + 2] = verticesSorted[vIndex]->v.z;
						}
					}
					////////////////////////
					else
					{
						// vertices of this meshset(to add all vertices to a Polyhedron)
						std::vector<carve::mesh::Vertex<3> *> vertices;

						// A meshset is composed of multiple meshes.( mesh = surface)
						std::vector<carve::mesh::Mesh<3>* >& vec_meshes = meshset->meshes;
						for (size_t i_mesh = 0; i_mesh < vec_meshes.size(); ++i_mesh)
						{
							// Mesh == Surface
							carve::mesh::Mesh<3>* mesh = vec_meshes[i_mesh];
							surface = NULL;

							size_t prevVertexCount = vertices.size();

							// A mesh is composed of multiple faces. (face == triangle)
							std::vector<carve::mesh::Face<3>* >& vec_faces = mesh->faces;
							for (size_t i_face = 0; i_face < vec_faces.size(); ++i_face)
							{
								// Face == Triangle
								carve::mesh::Face<3>* face = vec_faces[i_face];

								// iterate through edges:
								carve::mesh::Edge<3>* edge = face->edge;
								do
								{
									// start vertices of each edge
									// 각 edge의 start vertext만 instance화 한다.
									// 왜냐하면 한 edge의 end vertex는 다음 edge의 start vertex이므로.
									carve::mesh::Vertex<3>* vertex_begin = edge->v1();

									vertices.push_back(vertex_begin);

									edge = edge->next;
								} while (edge != face->edge);
							}

							if ((vertices.size() - prevVertexCount) / 3 == 0)
								continue;

							surface = new Surface;

							// fill triangle count and triangle vertex indices
							surface->triangleCount = (vertices.size() - prevVertexCount) / 3;
							surface->triangleIndices = new size_t[3 * surface->triangleCount];
							memset(surface->triangleIndices, 0x00, sizeof(size_t) * 3 * surface->triangleCount);
							for (size_t iIndex = 0; iIndex < 3 * surface->triangleCount; iIndex++)
								surface->triangleIndices[iIndex] = prevVertexCount + iIndex;

							// add this surface to this polyhedron
							if (polyhedron == NULL)
								polyhedron = new Polyhedron;

							polyhedron->surfaces.push_back(surface);
						}

						if (polyhedron == NULL)
							continue;

						polyhedron->vertexCount = vertices.size();
						polyhedron->vertices = new double[3 * polyhedron->vertexCount];
						memset(polyhedron->vertices, 0x00, sizeof(double) * 3 * polyhedron->vertexCount);
						for (size_t vIndex = 0; vIndex < polyhedron->vertexCount; vIndex++)
						{
							polyhedron->vertices[3 * vIndex] = vertices[vIndex]->v.x;
							polyhedron->vertices[3 * vIndex + 1] = vertices[vIndex]->v.y;
							polyhedron->vertices[3 * vIndex + 2] = vertices[vIndex]->v.z;
						}
					}

					// get representative color of a Polyhedron, if exist
					if (vec_items_appearances.size() > 0)
					{
						polyhedron->color[0] = vec_items_appearances[0]->m_color_diffuse.x;
						polyhedron->color[1] = vec_items_appearances[0]->m_color_diffuse.y;
						polyhedron->color[2] = vec_items_appearances[0]->m_color_diffuse.z;
						polyhedron->color[3] = vec_items_appearances[0]->m_color_diffuse.w;
					}
					else
						memset(polyhedron->color, 0x00, sizeof(float) * 4);

					// extract and allocate guid into this polyhedron
					//polyhedron->guid = ifc_product->m_GlobalId->m_value;
					//polyhedron->guid = productGuid;
					polyhedron->guid = productIdentifier;

					// add this polyhedron
					polyhedrons.push_back(polyhedron);

					if (polyhedronToGuidMapper.find(productIdentifier) == polyhedronToGuidMapper.end())
						polyhedronToGuidMapper[productIdentifier] = std::vector<Polyhedron*>();

					polyhedronToGuidMapper[productIdentifier].push_back(polyhedron);
				}
			}
		}

		if (this->bAttributesExtraction && checkIfPropertiesCanBeExtracted(ifc_product->m_entity_enum))
			loadObjectAttributes(ifc_product, objectPropertyRoot);
	}

	std::map<double, std::map<unsigned int, std::vector<Polyhedron*>>> tmpStories;
	std::map<double, std::map<unsigned int, std::vector<std::wstring>>>::iterator itrStory =  guidToStoryMapper.begin();
	for (; itrStory != guidToStoryMapper.end(); itrStory++)
	{
		tmpStories[itrStory->first] = std::map<unsigned int, std::vector<Polyhedron*>>();
		std::map<unsigned int, std::vector<std::wstring>>::iterator itrStoryDivision = itrStory->second.begin();
		for (; itrStoryDivision != itrStory->second.end(); itrStoryDivision++)
		{
			tmpStories[itrStory->first][itrStoryDivision->first] = std::vector<Polyhedron*>();
			for (size_t i = 0; i < itrStoryDivision->second.size(); i++)
			{
				for (size_t j = 0; j < polyhedronToGuidMapper[itrStoryDivision->second[i]].size(); j++)
				{
					tmpStories[itrStory->first][itrStoryDivision->first].push_back(polyhedronToGuidMapper[itrStoryDivision->second[i]][j]);
				}
			}
		}
	}

	std::map<double, std::map<unsigned int, std::vector<Polyhedron*>>>::iterator iterSortedStory = tmpStories.begin();
	for (; iterSortedStory != tmpStories.end(); iterSortedStory++)
	{
		stories.push_back(std::vector<std::vector<Polyhedron*>>());
		std::map<unsigned int, std::vector<Polyhedron*>>::iterator iterStoryDivision = iterSortedStory->second.begin();
		for (; iterStoryDivision != iterSortedStory->second.end(); iterStoryDivision++)
		{
			stories[stories.size() - 1].push_back(std::vector<Polyhedron*>());
			stories[stories.size() - 1][stories[stories.size() - 1].size() - 1].assign(iterStoryDivision->second.begin(), iterStoryDivision->second.end());
		}
	}

	/*printf("[TEMP]story count : %zd\n", stories.size());
	for (size_t i = 0; i < stories.size(); i++)
	{
		printf("-[TEMP]division count : %zd\n", stories[i].size());
		for (size_t j = 0; j < stories[i].size(); j++)
		{
			printf("--[TEMP]polyhedron count : %zd\n", stories[i][j].size());
		}
	}*/

	return true;
}

bool IfcLoader::loadOnlyPropertiesFromIfc(std::wstring& filePath)
{
	// initializing
	shared_ptr<MessageWrapper> mw(new MessageWrapper());
	shared_ptr<IfcPPModel> ifc_model(new IfcPPModel());
	shared_ptr<GeometryConverter> geometry_converter(new GeometryConverter(ifc_model));
	shared_ptr<IfcPPReaderSTEP> reader(new IfcPPReaderSTEP());

	reader->setMessageCallBack(mw.get(), &MessageWrapper::slotMessageWrapper);
	geometry_converter->setMessageCallBack(mw.get(), &MessageWrapper::slotMessageWrapper);

	// loading
	reader->loadModelFromFile(filePath, ifc_model);

	// conversion raw data into geometries of OSG type
	osg::ref_ptr<osg::Switch> model_switch = new osg::Switch();
	geometry_converter->createGeometryOSG(model_switch);

	// contains the VEF graph for each IfcProduct:
	std::map<int, shared_ptr<ProductShapeInputData> >& map_vef_data = geometry_converter->getShapeInputData();
	double volume_all_products = 0;

	std::map<int, shared_ptr<ProductShapeInputData> >::iterator it;
	for (it = map_vef_data.begin(); it != map_vef_data.end(); ++it)
		//for (auto it = map_vef_data.begin(); it != map_vef_data.end(); ++it)
	{
		// STEP entity id:
		int entity_id = it->first;

		// shape data
		shared_ptr<ProductShapeInputData>& shape_data = it->second;

		// IfcProduct(abstract type)
		shared_ptr<IfcProduct> ifc_product(shape_data->m_ifc_product);

		// filtering out IfcProduct of IfcSpace type
		shared_ptr<IfcSpace> Ifc_Space = dynamic_pointer_cast<IfcSpace>(ifc_product);
		if (Ifc_Space != NULL)
			continue;

		// filtering out IfcProduct of IfcSite type
		shared_ptr<IfcSite> site_elem = dynamic_pointer_cast<IfcSite>(ifc_product);
		if (site_elem != NULL)
			continue;

		if (!checkIfPropertiesCanBeExtracted(ifc_product->m_entity_enum))
			continue;

		loadObjectAttributes(ifc_product, objectPropertyRoot);
	}

#ifdef TMPTEST
	std::map<std::string, std::string>::iterator iter = attributeMap.begin();
	for (; iter != attributeMap.end(); iter++)
		attributeTypes.push_back(iter->first);
#endif

	return true;
}

void IfcLoader::setVertexReductionMode(bool bOn)
{
	bVertexReduction = bOn;
}

void IfcLoader::setAttributesExtraction(bool bOn)
{
	bAttributesExtraction = bOn;
}

size_t IfcLoader::getPolyhedronCount()
{
	return polyhedrons.size();
}

float* IfcLoader::getRepresentativeColor(size_t polyhedronIndex)
{
	return polyhedrons[polyhedronIndex]->color;
}

void IfcLoader::getGuid(size_t polyhedronIndex, wchar_t buffer[])
{
	size_t length = polyhedrons[polyhedronIndex]->guid.size();
	memcpy(buffer, polyhedrons[polyhedronIndex]->guid.c_str(), sizeof(wchar_t)*length);
}

size_t IfcLoader::getVertexCount(size_t polyhedronIndex)
{
	return polyhedrons[polyhedronIndex]->vertexCount;
}

double* IfcLoader::getVertexPositions(size_t polyhedronIndex)
{
	return polyhedrons[polyhedronIndex]->vertices;
}

size_t IfcLoader::getSurfaceCount(size_t polyhedronIndex)
{
	return polyhedrons[polyhedronIndex]->surfaces.size();
}

size_t IfcLoader::getTrialgleCount(size_t polyhedronIndex, size_t surfaceIndex)
{
	return polyhedrons[polyhedronIndex]->surfaces[surfaceIndex]->triangleCount;
}

size_t* IfcLoader::getTriangleIndices(size_t polyhedronIndex, size_t surfaceIndex)
{
	return polyhedrons[polyhedronIndex]->surfaces[surfaceIndex]->triangleIndices;
}

size_t IfcLoader::getStoryCount()
{
	return stories.size();
}

size_t IfcLoader::getStoryDivisionCount(size_t storyIndex)
{
	return stories[storyIndex].size();
}

size_t IfcLoader::getPolyhedronCount(size_t storyIndex, size_t divisionIndex)
{
	return stories[storyIndex][divisionIndex].size();
}

float* IfcLoader::getRepresentativeColor(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex)
{
	return stories[storyIndex][divisionIndex][polyhedronIndex]->color;
}

void IfcLoader::getGuid(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex, wchar_t buffer[])
{
	size_t length = stories[storyIndex][divisionIndex][polyhedronIndex]->guid.size();
	memcpy(buffer, stories[storyIndex][divisionIndex][polyhedronIndex]->guid.c_str(), sizeof(wchar_t)*length);
}

size_t IfcLoader::getVertexCount(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex)
{
	return stories[storyIndex][divisionIndex][polyhedronIndex]->vertexCount;
}

double* IfcLoader::getVertexPositions(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex)
{
	return stories[storyIndex][divisionIndex][polyhedronIndex]->vertices;
}

size_t IfcLoader::getSurfaceCount(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex)
{
	return stories[storyIndex][divisionIndex][polyhedronIndex]->surfaces.size();
}

size_t IfcLoader::getTrialgleCount(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex, size_t surfaceIndex)
{
	return stories[storyIndex][divisionIndex][polyhedronIndex]->surfaces[surfaceIndex]->triangleCount;
}

size_t* IfcLoader::getTriangleIndices(size_t storyIndex, size_t divisionIndex, size_t polyhedronIndex, size_t surfaceIndex)
{
	return stories[storyIndex][divisionIndex][polyhedronIndex]->surfaces[surfaceIndex]->triangleIndices;
}

std::string IfcLoader::getObjectAttributes()
{
	std::string result;

	Json::StyledWriter writer;
	result = writer.write(objectPropertyRoot);

	return result;
}

std::string IfcLoader::getProjectAttributes()
{
	std::string result;

	Json::StyledWriter writer;
	result = writer.write(projectPropertyRoot);

	return result;
}

void IfcLoader::loadProjectAttributes()
{}

void IfcLoader::loadObjectAttributes(shared_ptr<IfcProduct> ifcProduct, Json::Value& root)
{
	Json::Value properties(Json::objectValue);

	// guid
	std::string guid = convertWideStringToUtf8(ifcProduct->m_GlobalId->m_value);
	properties["guid"] = guid;

	// element type
	IfcPPEntityEnum ppEnum = ifcProduct->m_entity_enum;
	std::string elemType1(ifcProduct->className());
	properties["entityType"] = elemType1;
	properties["propertySets"] = Json::Value(Json::arrayValue);

	// properties
	size_t propertySetCount = ifcProduct->m_IsDefinedBy_inverse.size();
	for (size_t i = 0; i < propertySetCount; i++)
	{
		shared_ptr<IfcRelDefinesByProperties> propertySetWrapper = dynamic_pointer_cast<IfcRelDefinesByProperties>(ifcProduct->m_IsDefinedBy_inverse[i].lock());

		shared_ptr<IfcPropertySet> propertySet = dynamic_pointer_cast<IfcPropertySet>(propertySetWrapper->m_RelatingPropertyDefinition);

		if (propertySet == NULL)
			continue;

		Json::Value aSet(Json::objectValue);

		if (!propertySet->m_Name->m_value.empty())
			aSet["propertySetName"] = convertWideStringToUtf8(propertySet->m_Name->m_value); // @@@property set name
		else
			aSet["propertySetName"] = std::string("unknown");

		Json::Value propertyAggr(Json::objectValue);
		size_t propertyCount = propertySet->m_HasProperties.size();
		for (size_t j = 0; j < propertyCount; j++)
		{

			// property key
			std::string keyName;
			if (!propertySet->m_HasProperties[j]->m_Name->m_value.empty())
				keyName = convertWideStringToUtf8(propertySet->m_HasProperties[j]->m_Name->m_value);
			else
				keyName = std::string("key") + std::to_string(j);

			if (keyName.find(std::string("tag")) != std::string::npos ||
				keyName.find(std::string("TAG")) != std::string::npos)
				int xxx = 0;

			// property value
			if (dynamic_pointer_cast<IfcSimpleProperty>(propertySet->m_HasProperties[j]) != NULL)
			{
				if (dynamic_pointer_cast<IfcPropertySingleValue>(propertySet->m_HasProperties[j]) != NULL)
				{
					shared_ptr<IfcPropertySingleValue> singleValue = dynamic_pointer_cast<IfcPropertySingleValue>(propertySet->m_HasProperties[j]);

					Json::Value valueObject(Json::objectValue);
					this->parsePropertySingleValue(valueObject, singleValue->m_NominalValue);

#ifdef TMPTEST
					attributeMap.insert(std::map<std::string, std::string>::value_type(std::string(singleValue->m_NominalValue->className()), std::string(singleValue->m_NominalValue->className())));
#endif
					if (singleValue->m_Unit != NULL)
					{
						if (dynamic_pointer_cast<IfcNamedUnit>(singleValue->m_Unit) != NULL)
						{
							shared_ptr<IfcNamedUnit> namedUnit = dynamic_pointer_cast<IfcNamedUnit>(singleValue->m_Unit);
							valueObject["unit"] = namedUnit->m_UnitType->m_enum;
						}
						else if (dynamic_pointer_cast<IfcMonetaryUnit>(singleValue->m_Unit) != NULL)
						{
							shared_ptr<IfcMonetaryUnit> monetaryUnit = dynamic_pointer_cast<IfcMonetaryUnit>(singleValue->m_Unit);
							if (!monetaryUnit->m_Currency->m_value.empty())
								valueObject["unit"] = convertWideStringToUtf8(monetaryUnit->m_Currency->m_value);
							else
								valueObject["unit"] = std::string("empty currency unit");
						}
						else if (dynamic_pointer_cast<IfcDerivedUnit>(singleValue->m_Unit) != NULL)
						{
							//shared_ptr<IfcDerivedUnit> derivedUnit = dynamic_pointer_cast<IfcDerivedUnit>(singleValue->m_Unit);
							valueObject["unit"] = std::string("derived unit");
						}
						else
							valueObject["unit"] = std::string("unknown unit");
					}
					propertyAggr[keyName] = valueObject;
				}
				else
					propertyAggr[keyName] = std::string("unknown : ") + std::string(propertySet->m_HasProperties[j]->className());
			}
			else
				propertyAggr[keyName] = std::string("unknown : ") + std::string(propertySet->m_HasProperties[j]->className());
		}
		aSet["properties"] = propertyAggr;

		properties["propertySets"].append(aSet);
	}

	root["objects"].append(properties);
}

bool IfcLoader::checkIfPropertiesCanBeExtracted(IfcPPEntityEnum ppEnum)
{
	switch (ppEnum)
	{
	case IfcPPEntityEnum::IFCANNOTATION:
	case IfcPPEntityEnum::IFCANNOTATIONFILLAREA:
		return false;
	}
	return true;
}

void IfcLoader::parsePropertySingleValue(Json::Value& valueObject, shared_ptr<IfcValue> value)
{
	if (dynamic_pointer_cast<IfcAreaMeasure>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcAreaMeasure>(value)->m_value;
	else if (dynamic_pointer_cast<IfcBoolean>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcBoolean>(value)->m_value;
	else if (dynamic_pointer_cast<IfcIdentifier>(value) != NULL)
	{
		if (dynamic_pointer_cast<IfcIdentifier>(value)->m_value.empty())
			valueObject["value"] = std::string("");
		else
			valueObject["value"] = convertWideStringToUtf8(dynamic_pointer_cast<IfcIdentifier>(value)->m_value);
	}
	else if (dynamic_pointer_cast<IfcInteger>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcInteger>(value)->m_value;
	else if (dynamic_pointer_cast<IfcLabel>(value) != NULL)
	{
		if (dynamic_pointer_cast<IfcLabel>(value)->m_value.empty())
			valueObject["value"] = std::string("");
		else
			valueObject["value"] = convertWideStringToUtf8(dynamic_pointer_cast<IfcLabel>(value)->m_value);
	}
	else if (dynamic_pointer_cast<IfcLengthMeasure>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcLengthMeasure>(value)->m_value;
	else if (dynamic_pointer_cast<IfcPlaneAngleMeasure>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcPlaneAngleMeasure>(value)->m_value;
	else if (dynamic_pointer_cast<IfcPositiveLengthMeasure>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcPositiveLengthMeasure>(value)->m_value;
	else if (dynamic_pointer_cast<IfcReal>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcReal>(value)->m_value;
	else if (dynamic_pointer_cast<IfcVolumeMeasure>(value) != NULL)
		valueObject["value"] = dynamic_pointer_cast<IfcVolumeMeasure>(value)->m_value;
	else
		valueObject["value"] = std::string("unknown : ") + std::string(value->className());
}

#ifdef _WIN32
#include <stringapiset.h>
#endif
std::string IfcLoader::convertWideStringToUtf8(std::wstring& sourceString)
{
#ifdef _WIN32
	int neededLength = WideCharToMultiByte(CP_UTF8, 0, sourceString.c_str(), (int)sourceString.size(), NULL, 0, NULL, NULL);
	char* newStringBuffer = new char[neededLength + 1];
	memset(newStringBuffer, 0x00, sizeof(char)*(neededLength + 1));
	WideCharToMultiByte(CP_UTF8, 0, sourceString.c_str(), (int)sourceString.size(), newStringBuffer, neededLength, NULL, NULL);
	std::string newString(newStringBuffer);
	delete[] newStringBuffer;
	return newString;
#else
	std::string newString(sourceString.begin(), sourceString.end());
	return newString;
#endif
}

#ifdef TMPTEST
size_t IfcLoader::getAttributeTypeCount()
{
	return attributeTypes.size();
}
std::string IfcLoader::getAttributeType(size_t i)
{
	return attributeTypes[i];
}
#endif


aIfcLoader* createIfcLoader()
{
	return new IfcLoader;
}

void destroyIfcLoader(aIfcLoader* aLoader)
{
	delete static_cast<IfcLoader*>(aLoader);
}