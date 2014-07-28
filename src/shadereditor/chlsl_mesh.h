#ifndef CHLSL_MESH_H
#define CHLSL_MESH_H

#include "editorCommon.h"

struct CHLSL_Vertex;
struct CHLSL_Triangle;
class CHLSL_Mesh;

struct CHLSL_Vertex
{
	CHLSL_Vertex();
	~CHLSL_Vertex();
	//CHLSL_Vertex( const CHLSL_Vertex &o );

	float pos[3];
	float normal[3];

	float tangent_s[4];
	float tangent_t[3];

	float uv[3][2];
};

struct CHLSL_Triangle
{
	CHLSL_Triangle();
	~CHLSL_Triangle();
	//CHLSL_Triangle( const CHLSL_Triangle &o );

	CHLSL_Vertex *vertices[3];
};

class CHLSL_Mesh
{
public:
	CHLSL_Mesh();
	~CHLSL_Mesh();
	CHLSL_Mesh( const CHLSL_Mesh &o );

	void CreateSphere( float radius, int subdiv );
	void CreateCube( float size, int subdiv );
	void CreateCylinder( float size, int subdiv );
	void CreatePlane( float size, int subdiv );

	const int GetNumVertices(){ return m_iNumVertices; };
	CHLSL_Vertex *GetVertices(){ return m_Vertices; };

	const int GetNumTriangles(){ return m_iNumTriangles; };
	CHLSL_Triangle *GetTriangles(){ return m_Triangles; };

private:

	CHLSL_Vertex *m_Vertices;
	int m_iNumVertices;

	CHLSL_Triangle *m_Triangles;
	int m_iNumTriangles;

	void GenerateTangentSpace();
};



#endif