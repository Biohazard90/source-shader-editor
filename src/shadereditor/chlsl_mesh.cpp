
#include "cbase.h"
#include "editorCommon.h"


CHLSL_Vertex::CHLSL_Vertex()
{
	Q_memset( this, 0, sizeof(CHLSL_Vertex) );
}
CHLSL_Vertex::~CHLSL_Vertex()
{
}
//CHLSL_Vertex::CHLSL_Vertex( const CHLSL_Vertex &o )
//{
//}



CHLSL_Triangle::CHLSL_Triangle()
{
	vertices[ 0 ] = NULL;
	vertices[ 1 ] = NULL;
	vertices[ 2 ] = NULL;
}
CHLSL_Triangle::~CHLSL_Triangle()
{
}
//CHLSL_Triangle::CHLSL_Triangle( const CHLSL_Triangle &o )
//{
//}



CHLSL_Mesh::CHLSL_Mesh()
{
	m_Vertices = NULL;
	m_iNumVertices = 0;

	m_Triangles = NULL;
	m_iNumTriangles = 0;
}
CHLSL_Mesh::~CHLSL_Mesh()
{
	delete [] m_Vertices;
	delete [] m_Triangles;
}
CHLSL_Mesh::CHLSL_Mesh( const CHLSL_Mesh &o )
{
	m_iNumVertices = o.m_iNumVertices;
	m_iNumTriangles = o.m_iNumTriangles;

	m_Vertices = new CHLSL_Vertex[m_iNumVertices];
	Q_memcpy(m_Vertices,o.m_Vertices,sizeof(CHLSL_Vertex)*m_iNumVertices);

	m_Triangles = new CHLSL_Triangle[m_iNumTriangles];
	Q_memcpy(m_Triangles,o.m_Triangles,sizeof(CHLSL_Triangle)*m_iNumTriangles);

	int vertexOffset = m_Vertices - o.m_Vertices;
	for ( int i = 0; i < m_iNumTriangles; i++ )
	{
		m_Triangles[i].vertices[0] += vertexOffset;
		m_Triangles[i].vertices[1] += vertexOffset;
		m_Triangles[i].vertices[2] += vertexOffset;
	}
}

void CHLSL_Mesh::CreateCylinder( float size, int subdiv )
{
	subdiv /= 2;

	delete [] m_Vertices;
	delete [] m_Triangles;

	int num_verts_u = subdiv + 1;
	int num_verts_v = subdiv + 1;
	int num_verts_side = num_verts_u * num_verts_v;
	int num_verts_top = 1 + num_verts_u - 1;
	m_iNumVertices = num_verts_side + num_verts_top * 2;

	int num_tris_u = (num_verts_u-1);
	int num_tris_v = (num_verts_v-1);
	int num_tris_side = num_tris_u * num_tris_v * 2;
	int num_tris_top = num_tris_u;
	m_iNumTriangles = num_tris_side + num_tris_top * 2;

	m_Vertices = new CHLSL_Vertex[ m_iNumVertices ];
	m_Triangles = new CHLSL_Triangle[ m_iNumTriangles ];

	QAngle rotate( 0, 0, 0 );

	float yaw_step = 360.0f / (num_verts_u-1);
	float up_step = size * 2.0f / (num_verts_v-1);
	float size_radius = size * 0.5f;

	Vector fwd, right, up, pos;

/// verts for side
	for ( int vert_u = 0; vert_u < num_verts_u; vert_u++ )
	{
		AngleVectors( rotate, &fwd, &right, &up );

		pos = up * size + fwd * size_radius;

		for ( int vert_v = 0; vert_v < num_verts_v; vert_v++ )
		{
			int vindex = vert_u * num_verts_v +
				vert_v;

			Assert( vindex < m_iNumVertices );

			CHLSL_Vertex &vert = m_Vertices[ vindex ];

			Q_memcpy( vert.normal, fwd.Base(), sizeof( float ) * 3 );
			Q_memcpy( vert.pos, pos.Base(), sizeof( float ) * 3 );

			vert.uv[0][0] = vert_u / (float)( num_verts_u - 1 );
			vert.uv[0][1] = vert_v / (float)( num_verts_v - 1 );

			pos -= up * up_step;
		}

		rotate.y += yaw_step;
	}

/// verts for top/bottom
	for ( int v_side = 0; v_side < 2; v_side++ )
	{
		rotate.y = 0;
		float sign = (v_side==1) ? -1.0f : 1.0f;

		up.Init( 0, 0, sign );
		Vector height = up * size;

		for ( int v_u = 0; v_u < num_verts_top; v_u++ )
		{
			int vindex = num_verts_side +
				v_side * num_verts_top +
				v_u;

			Assert( vindex < m_iNumVertices );

			CHLSL_Vertex &vert = m_Vertices[ vindex ];

			Vector2D uv;
			Vector pos = height;

			if ( v_u > 0 )
			{
				AngleVectors( rotate, &fwd, NULL, NULL );
				pos += fwd * size_radius;
				uv.x = -sign * fwd.x * 0.5f + 0.5f;
				uv.y = fwd.y * 0.5f + 0.5f;
				rotate.y += yaw_step * sign;
			}
			else
				uv.Init( 0.5f, 0.5f );

			Q_memcpy( vert.pos, pos.Base(), sizeof( float ) * 3 );
			Q_memcpy( vert.normal, up.Base(), sizeof( float ) * 3 );
			Q_memcpy( vert.uv, uv.Base(), sizeof( float ) * 2 );
		}
	}

/// tris for side
	for ( int tri_u = 0; tri_u < num_tris_u; tri_u++ )
	{
		for ( int tri_v = 0; tri_v < num_tris_v; tri_v++ )
		{
			int tri_0 = tri_u * num_tris_v +
				tri_v;
			int tri_1 = tri_0 + num_tris_side / 2;

			Assert( tri_0 < m_iNumTriangles );
			Assert( tri_1 < m_iNumTriangles );

			CHLSL_Triangle &t0 = m_Triangles[ tri_0 ];
			CHLSL_Triangle &t1 = m_Triangles[ tri_1 ];

			int v00 = tri_u * num_verts_v +
				tri_v;
			int v01 = v00 + 1;
			int v10 = (tri_u+1) * num_verts_v +
				tri_v;
			int v11 = v10 + 1;

			Assert( v00 < m_iNumVertices );
			Assert( v01 < m_iNumVertices );
			Assert( v10 < m_iNumVertices );
			Assert( v11 < m_iNumVertices );

			t0.vertices[ 0 ] = &m_Vertices[ v00 ];
			t0.vertices[ 1 ] = &m_Vertices[ v10 ];
			t0.vertices[ 2 ] = &m_Vertices[ v01 ];

			t1.vertices[ 0 ] = &m_Vertices[ v10 ];
			t1.vertices[ 1 ] = &m_Vertices[ v11 ];
			t1.vertices[ 2 ] = &m_Vertices[ v01 ];
		}
	}

/// tris for top
	for ( int dir = 0; dir < 2; dir++ )
	{
		int v_mid = num_verts_side +
			dir * num_verts_top;

		Assert( v_mid < m_iNumVertices );

		for ( int tri_n = 0; tri_n < num_tris_top; tri_n++ )
		{
			int tIndex = num_tris_side +
				dir * num_tris_top +
				tri_n;

			Assert( tIndex < m_iNumTriangles );

			CHLSL_Triangle &t = m_Triangles[ tIndex ];

			int v00 = v_mid + tri_n + 1;
			int v10 = (tri_n < (num_tris_top-1)) ? v00 + 1 : v_mid + 1;

			Assert( v00 < m_iNumVertices );
			Assert( v10 < m_iNumVertices );

			t.vertices[ 0 ] = &m_Vertices[ v00 ];
			t.vertices[ 1 ] = &m_Vertices[ v_mid ];
			t.vertices[ 2 ] = &m_Vertices[ v10 ];
		}
	}


	GenerateTangentSpace();
}

void CHLSL_Mesh::CreatePlane( float size, int subdiv )
{
	delete [] m_Vertices;
	delete [] m_Triangles;

	float dist_per_vert = size / subdiv * 2;

	int num_verts_side = ( subdiv + 1 );
	m_iNumVertices = num_verts_side * num_verts_side;

	int num_tris_side = subdiv;
	m_iNumTriangles = num_tris_side * num_tris_side * 2;

	m_Vertices = new CHLSL_Vertex[ m_iNumVertices ];
	m_Triangles = new CHLSL_Triangle[ m_iNumTriangles ];

	Vector fwd( 0, 0, 1 );
	Vector right( 0, -1, 0 );
	Vector up( -1, 0, 0 );

	for ( int vert_right = 0; vert_right < num_verts_side; vert_right++ )
	{
		for ( int vert_up = 0; vert_up < num_verts_side; vert_up++ )
		{
			int vertindex = vert_up * num_verts_side +
							vert_right;

			CHLSL_Vertex &vert = m_Vertices[ vertindex ];

			Vector pos = right * -size + up * -size;
			pos += right * vert_right * dist_per_vert;
			pos += up * vert_up * dist_per_vert;

			Vector2D uv( vert_right / (float)(num_verts_side - 1),
				vert_up / (float)(num_verts_side - 1) );

			Vector n = fwd;

			Q_memcpy( vert.normal, n.Base(), sizeof( float ) * 3 );
			Q_memcpy( vert.pos, pos.Base(), sizeof( float ) * 3 );
			Q_memcpy( vert.uv, uv.Base(), sizeof( float ) * 2 );
		}
	}

	for ( int tri_right = 0; tri_right < num_tris_side; tri_right++ )
	{
		for ( int tri_up = 0; tri_up < num_tris_side; tri_up++ )
		{
			int tri_index_0 = tri_up * num_tris_side +
				tri_right;
			int tri_index_1 = tri_index_0 + m_iNumTriangles / 2;

			Assert( tri_index_0 < m_iNumTriangles );
			Assert( tri_index_1 < m_iNumTriangles );

			int v00_index = tri_up * num_verts_side +
				tri_right;
			int v10_index = v00_index + 1;
			int v01_index = (tri_up+1) * num_verts_side +
				tri_right;
			int v11_index = v01_index + 1;

			Assert( v00_index < m_iNumVertices );
			Assert( v01_index < m_iNumVertices );
			Assert( v10_index < m_iNumVertices );
			Assert( v11_index < m_iNumVertices );

			CHLSL_Triangle &t0 = m_Triangles[ tri_index_0 ];
			CHLSL_Triangle &t1 = m_Triangles[ tri_index_1 ];

			t0.vertices[ 0 ] = &m_Vertices[ v00_index ];
			t0.vertices[ 1 ] = &m_Vertices[ v10_index ];
			t0.vertices[ 2 ] = &m_Vertices[ v01_index ];

			t1.vertices[ 0 ] = &m_Vertices[ v10_index ];
			t1.vertices[ 1 ] = &m_Vertices[ v11_index ];
			t1.vertices[ 2 ] = &m_Vertices[ v01_index ];
		}
	}

	GenerateTangentSpace();
}

void CHLSL_Mesh::CreateCube( float size, int subdiv )
{
	subdiv /= 3;
	subdiv = max( subdiv, 1 );

	delete [] m_Vertices;
	delete [] m_Triangles;

	float dist_per_vert = size / subdiv * 2;

	int num_verts_side = ( subdiv + 1 );
	int num_verts_plane = num_verts_side * num_verts_side;

	int num_tris_side = subdiv;
	int num_tris_plane = num_tris_side * num_tris_side * 2;

	m_iNumVertices = num_verts_plane * 6;
	m_iNumTriangles = num_tris_plane * 6;

	m_Vertices = new CHLSL_Vertex[ m_iNumVertices ];
	m_Triangles = new CHLSL_Triangle[ m_iNumTriangles ];

	const Vector orient[6][3] =
	{
		Vector( 1, 0, 0 ),Vector( 0, 1, 0 ),Vector( 0, 0, -1 ),
		Vector( 0, 1, 0 ),Vector( -1, 0, 0 ),Vector( 0, 0, -1 ),
		Vector( -1, 0, 0 ),Vector( 0, -1, 0 ),Vector( 0, 0, -1 ),
		Vector( 0, -1, 0 ),Vector( 1, 0, 0 ),Vector( 0, 0, -1 ),
		Vector( 0, 0, 1 ),Vector( -1, 0, 0 ),Vector( 0, 1, 0 ),
		Vector( 0, 0, -1 ),Vector( -1, 0, 0 ),Vector( 0, -1, 0 ),
	};

	for ( int plane = 0; plane < 6; plane++ )
	{
		int orientIndex = plane;

		Vector fwd = orient[ orientIndex ][0];
		Vector right = orient[ orientIndex ][1];
		Vector up = orient[ orientIndex ][2];

		for ( int vert_right = 0; vert_right < num_verts_side; vert_right++ )
		{
			for ( int vert_up = 0; vert_up < num_verts_side; vert_up++ )
			{
				int vertindex = plane * num_verts_plane +
								vert_up * num_verts_side +
								vert_right;

				Assert( vertindex < m_iNumVertices );

				CHLSL_Vertex &vert = m_Vertices[ vertindex ];

				Vector pos = fwd * size;
				pos -= right * size + up * size;
				pos += right * vert_right * dist_per_vert;
				pos += up * vert_up * dist_per_vert;

				Vector2D uv( vert_right / (float)(num_verts_side - 1),
					vert_up / (float)(num_verts_side - 1) );

				Vector n = fwd;

				Q_memcpy( vert.normal, n.Base(), sizeof( float ) * 3 );
				Q_memcpy( vert.pos, pos.Base(), sizeof( float ) * 3 );
				Q_memcpy( vert.uv, uv.Base(), sizeof( float ) * 2 );
			}
		}
	}

	for ( int plane = 0; plane < 6; plane++ )
	{
		bool bFlip = true; //plane > 2;

		for ( int tri_right = 0; tri_right < num_tris_side; tri_right++ )
		{
			for ( int tri_up = 0; tri_up < num_tris_side; tri_up++ )
			{
				int tri_index_0 = plane * num_tris_plane +
					tri_up * num_tris_side +
					tri_right;
				int tri_index_1 = tri_index_0 + num_tris_plane / 2;

				Assert( tri_index_0 < m_iNumTriangles );
				Assert( tri_index_1 < m_iNumTriangles );

				int v00_index = plane * num_verts_plane +
					tri_up * num_verts_side +
					tri_right;
				int v10_index = v00_index + 1;
				int v01_index = plane * num_verts_plane +
					(tri_up+1) * num_verts_side +
					tri_right;
				int v11_index = v01_index + 1;

				Assert( v00_index < m_iNumVertices );
				Assert( v01_index < m_iNumVertices );
				Assert( v10_index < m_iNumVertices );
				Assert( v11_index < m_iNumVertices );

				CHLSL_Triangle &t0 = m_Triangles[ tri_index_0 ];
				CHLSL_Triangle &t1 = m_Triangles[ tri_index_1 ];

				int idx_1 = bFlip ? 1 : 2;
				int idx_2 = bFlip ? 2 : 1;

				t0.vertices[ 0 ] = &m_Vertices[ v00_index ];
				t0.vertices[ idx_1 ] = &m_Vertices[ v10_index ];
				t0.vertices[ idx_2 ] = &m_Vertices[ v01_index ];

				t1.vertices[ 0 ] = &m_Vertices[ v10_index ];
				t1.vertices[ idx_1 ] = &m_Vertices[ v11_index ];
				t1.vertices[ idx_2 ] = &m_Vertices[ v01_index ];
			}
		}
	}

	GenerateTangentSpace();
}

void CHLSL_Mesh::CreateSphere( float radius, int subdiv )
{
	subdiv = max( subdiv, 1 );

	delete [] m_Vertices;
	delete [] m_Triangles;
	int subdivs_u = subdiv;
	// two halfs
	int subdivs_v = subdivs_u / 2;

	float yaw_change = 360.0f / subdivs_u;
	float pitch_change = 90.0f / subdivs_v;

	int iverts_onehalf = (subdivs_v + 1) * subdivs_u;
	m_iNumVertices = iverts_onehalf * 2;
	m_Vertices = new CHLSL_Vertex[m_iNumVertices];

	int itris_onehalf = (subdivs_v - 1) * subdivs_u * 2 + subdivs_u;
	m_iNumTriangles = itris_onehalf * 2;
	m_Triangles = new CHLSL_Triangle[m_iNumTriangles];

/// SETUP VERTICES
	for ( int goUp = 0; goUp <= subdivs_v; goUp++ )
	{
		for ( int goRound = 0; goRound < subdivs_u; goRound++ )
		{
			QAngle center_d( 90, 0, 0 );
			center_d.x -= goUp * pitch_change;
			center_d.y += goRound * yaw_change;
			if ( goUp == subdivs_v )
				center_d.x = 0;

			Vector dir;
			AngleVectors( center_d, &dir );

			int VertexSlot = goUp + (1+subdivs_v) * goRound;
			//Msg("writing vert: %i\n", VertexSlot);

			Vector normal = dir;
			normal.NormalizeInPlace();
			Vector pos = dir * radius;

			if ( !goUp && goRound )
				m_Vertices[ VertexSlot ] = m_Vertices[ 0 ];
			else
			{
				//CHLSL_Vertex *pV = new CHLSL_Vertex();
				//VectorCopy( pos.Base(), pV->pos );
				//VectorCopy( normal.Base(), pV->normal );
				//m_Vertices[ VertexSlot ] = *pV;
				CHLSL_Vertex pV;
				VectorCopy( pos.Base(), pV.pos );
				VectorCopy( normal.Base(), pV.normal );
				m_Vertices[ VertexSlot ] = pV;
			}
		}
	}
	for ( int goUp = 0; goUp <= subdivs_v; goUp++ )
	{
		for ( int goRound = 0; goRound < subdivs_u; goRound++ )
		{
			QAngle center_d( -90, 0, 0 );
			center_d.x += goUp * pitch_change;
			center_d.y += goRound * yaw_change;
			if ( goUp == subdivs_v )
				center_d.x = 0;

			Vector dir;
			AngleVectors( center_d, &dir );

			int VertexSlot = goUp + (1+subdivs_v) * goRound + iverts_onehalf;
			//Msg("2- writing vert: %i\n", VertexSlot);

			Vector normal = dir;
			normal.NormalizeInPlace();
			Vector pos = dir * radius;

			if ( !goUp && goRound )
				m_Vertices[ VertexSlot ] = m_Vertices[ iverts_onehalf ];
			else
			{
				//CHLSL_Vertex *pV = new CHLSL_Vertex();
				//VectorCopy( pos.Base(), pV->pos );
				//VectorCopy( normal.Base(), pV->normal );
				//m_Vertices[ VertexSlot ] = *pV;
				CHLSL_Vertex pV;
				VectorCopy( pos.Base(), pV.pos );
				VectorCopy( normal.Base(), pV.normal );
				m_Vertices[ VertexSlot ] = pV;
			}
		}
	}

/// SETUP TRIANGLE LINKS
	int TriSlot_A = 0;
	for ( int goUp = 1; goUp < subdivs_v; goUp++ )
	{
		for ( int goRound = 0; goRound < subdivs_u; goRound++ )
		{
			int VertexSlot = goUp + (1+subdivs_v) * goRound;
			TriSlot_A = (goUp-1) * 2 + (subdivs_v-1) * goRound * 2;
			//Msg("writing triangle: %i and %i\n", TriSlot, TriSlot+1);
			int VertexSlot_NextU = goRound + 1;
			if ( VertexSlot_NextU >= subdivs_u )
				VertexSlot_NextU = 0;
			VertexSlot_NextU = goUp + (1+subdivs_v) * VertexSlot_NextU;

			m_Triangles[ TriSlot_A ].vertices[ 0 ] = &m_Vertices[ VertexSlot ];
			m_Triangles[ TriSlot_A ].vertices[ 1 ] = &m_Vertices[ VertexSlot + 1 ];
			m_Triangles[ TriSlot_A ].vertices[ 2 ] = &m_Vertices[ VertexSlot_NextU ];

			TriSlot_A++;
			m_Triangles[ TriSlot_A ].vertices[ 0 ] = &m_Vertices[ VertexSlot + 1 ];
			m_Triangles[ TriSlot_A ].vertices[ 1 ] = &m_Vertices[ VertexSlot_NextU + 1];
			m_Triangles[ TriSlot_A ].vertices[ 2 ] = &m_Vertices[ VertexSlot_NextU ];
		}
	}
	TriSlot_A++;
	int TriSlot_B = 0;
	for ( int goUp = 1; goUp < subdivs_v; goUp++ )
	{
		for ( int goRound = 0; goRound < subdivs_u; goRound++ )
		{
			int VertexSlot = goUp + (1+subdivs_v) * goRound + iverts_onehalf;
			TriSlot_B = TriSlot_A + (goUp-1) * 2 + (subdivs_v-1) * goRound * 2;
			//Msg("writing triangle: %i and %i\n", TriSlot, TriSlot+1);
			int VertexSlot_NextU = goRound + 1;
			if ( VertexSlot_NextU >= subdivs_u )
				VertexSlot_NextU = 0;
			VertexSlot_NextU = goUp + (1+subdivs_v) * VertexSlot_NextU + iverts_onehalf;

			m_Triangles[ TriSlot_B ].vertices[ 0 ] = &m_Vertices[ VertexSlot + 1 ];
			m_Triangles[ TriSlot_B ].vertices[ 1 ] = &m_Vertices[ VertexSlot ];
			m_Triangles[ TriSlot_B ].vertices[ 2 ] = &m_Vertices[ VertexSlot_NextU ];

			TriSlot_B++;
			m_Triangles[ TriSlot_B ].vertices[ 0 ] = &m_Vertices[ VertexSlot + 1 ];
			m_Triangles[ TriSlot_B ].vertices[ 1 ] = &m_Vertices[ VertexSlot_NextU ];
			m_Triangles[ TriSlot_B ].vertices[ 2 ] = &m_Vertices[ VertexSlot_NextU + 1];
		}
	}
	TriSlot_B++;
	for ( int goRound = 0; goRound < subdivs_u; goRound++ )
	{
		int VertexSlot = (1+subdivs_v) * goRound;

		int VertexSlot_u_plus = goRound + 1;
		if ( VertexSlot_u_plus >= subdivs_u )
			VertexSlot_u_plus = 0;
		VertexSlot_u_plus = (1+subdivs_v) * VertexSlot_u_plus + 1;
		//Msg("writing triangle: %i\n", TriSlot_B);

		m_Triangles[ TriSlot_B ].vertices[ 1 ] = &m_Vertices[ VertexSlot ];
		m_Triangles[ TriSlot_B ].vertices[ 2 ] = &m_Vertices[ VertexSlot + 1 ];
		m_Triangles[ TriSlot_B ].vertices[ 0 ] = &m_Vertices[ VertexSlot_u_plus ];
		TriSlot_B++;
	}

	for ( int goRound = 0; goRound < subdivs_u; goRound++ )
	{
		int VertexSlot = (1+subdivs_v) * goRound + iverts_onehalf;

		int VertexSlot_u_plus = goRound + 1;
		if ( VertexSlot_u_plus >= subdivs_u )
			VertexSlot_u_plus = 0;
		VertexSlot_u_plus = (1+subdivs_v) * VertexSlot_u_plus + iverts_onehalf + 1;
		//Msg("writing triangle: %i\n", TriSlot_B);

		m_Triangles[ TriSlot_B ].vertices[ 0 ] = &m_Vertices[ VertexSlot + 1 ];
		m_Triangles[ TriSlot_B ].vertices[ 1 ] = &m_Vertices[ VertexSlot ];
		m_Triangles[ TriSlot_B ].vertices[ 2 ] = &m_Vertices[ VertexSlot_u_plus ];
		TriSlot_B++;
	}

/// SETUP UVS
	// m_iNumTriangles
	// m_iNumVertices
	// iverts_onehalf
	for ( int side = 0; side <= 1; side++ )
	{
		for ( int goUp = 0; goUp <= subdivs_v; goUp++ )
		{
			for ( int goRound = 0; goRound < subdivs_u; goRound++ )
			{
				int VertexSlot = goUp + (1+subdivs_v) * goRound + side * iverts_onehalf;
				CHLSL_Vertex &vert = m_Vertices[ VertexSlot ];
				Vector dir( vert.pos[0], vert.pos[1], 0 );

				dir.NormalizeInPlace();
				dir *= 0.5f * goUp/(float)subdivs_v;

				vert.uv[0][ 0 ] = 0.5f + dir.x;
				vert.uv[0][ 1 ] = 0.5f + dir.y;
			}
		}
	}
	//for ( int side = 0; side <= 1; side++ )
	//{
	//	for ( int goUp = 0; goUp <= subdivs_v; goUp++ )
	//	{
	//		for ( int goRound = 0; goRound < subdivs_u; goRound++ )
	//		{
	//			int VertexSlot = goUp + (1+subdivs_v) * goRound + side * iverts_onehalf;
	//			CHLSL_Vertex &vert = m_Vertices[ VertexSlot ];

	//			float _a = goRound / float(subdivs_u-1);
	//			float _b = goUp / float(subdivs_v);

	//			vert.uv[0][ 0 ] = _a;
	//			vert.uv[0][ 1 ] = _b;
	//		}
	//	}
	//}

	GenerateTangentSpace();
}

void CHLSL_Mesh::GenerateTangentSpace()
{
	//for ( int t = 0; t < m_iNumTriangles; t++ )
	//{
	//	CHLSL_Triangle &tri = m_Triangles[ t ];
	//	for ( int v = 0; v < 3; v++ )
	//	{
	//		CHLSL_Vertex &vert = tri.vertices[ v ];
	//	}
	//}

	for (int a = 0; a < m_iNumVertices; a++)
	{
		CHLSL_Vertex &vert = m_Vertices[a];
		Q_memset( vert.tangent_s, 0, sizeof( float ) * 4 );
		Q_memset( vert.tangent_t, 0, sizeof( float ) * 3 );
	}

	CHLSL_Triangle *tri = m_Triangles;
    for (int a = 0; a < m_iNumTriangles; a++)
    {
		CHLSL_Vertex &vert_1 = *tri->vertices[0];
		CHLSL_Vertex &vert_2 = *tri->vertices[1];
		CHLSL_Vertex &vert_3 = *tri->vertices[2];

		Vector v1;
		Vector v2;
		Vector v3;
		VectorCopy( vert_1.pos, v1.Base() );
		VectorCopy( vert_2.pos, v2.Base() );
		VectorCopy( vert_3.pos, v3.Base() );
#if 0
		Vector w1( vert_1.uv[0][0], vert_1.uv[0][1], 0 );
		Vector w2( vert_2.uv[0][0], vert_2.uv[0][1], 0 );
		Vector w3( vert_3.uv[0][0], vert_3.uv[0][1], 0 );

		Vector tan_s_2d( 1, 0, 0 );
		Vector tan_t_2d( 0, 1, 0 );
		Vector delta_vert2 = v2 - v1;
		Vector delta_vert3 = v3 - v1;
		Vector delta_uv2 = w2 - w1;
		Vector delta_uv3 = w3 - w1;

		Vector n;
		VectorCopy( vert_1.normal, n.Base() );
		delta_vert2 -= DotProduct( delta_vert2, n ) * n;
		delta_vert3 -= DotProduct( delta_vert3, n ) * n;

		//delta_vert2.NormalizeInPlace();
		//delta_vert3.NormalizeInPlace();
		delta_uv2.NormalizeInPlace();
		delta_uv3.NormalizeInPlace();

		Vector sdir( vec3_origin );
		Vector tdir( vec3_origin );

		sdir += DotProduct( tan_s_2d, delta_uv2 ) * delta_vert2;
		sdir += DotProduct( tan_s_2d, delta_uv3 ) * delta_vert3;

		tdir += DotProduct( tan_t_2d, delta_uv2 ) * delta_vert2;
		tdir += DotProduct( tan_t_2d, delta_uv3 ) * delta_vert3;
#else
		
		Vector2D w1( vert_1.uv[0][0], vert_1.uv[0][1] );
		Vector2D w2( vert_2.uv[0][0], vert_2.uv[0][1] );
		Vector2D w3( vert_3.uv[0][0], vert_3.uv[0][1] );

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = (s1 * t2 - s2 * t1);
		if ( r != 0 )
			r = 1.0f / r;

		Vector sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
				(t2 * z1 - t1 * z2) * r);
		Vector tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
				(s1 * z2 - s2 * z1) * r);
	
#endif
		for ( int i = 0; i < 3; i++ )
		{
			Assert( IsFinite( vert_1.tangent_s[i] ) );
			Assert( IsFinite( vert_2.tangent_s[i] ) );
			Assert( IsFinite( vert_3.tangent_s[i] ) );
			Assert( IsFinite( vert_1.tangent_t[i] ) );
			Assert( IsFinite( vert_2.tangent_t[i] ) );
			Assert( IsFinite( vert_3.tangent_t[i] ) );

			vert_1.tangent_s[i] += sdir[i];
			vert_2.tangent_s[i] += sdir[i];
			vert_3.tangent_s[i] += sdir[i];

			vert_1.tangent_t[i] += tdir[i];
			vert_2.tangent_t[i] += tdir[i];
			vert_3.tangent_t[i] += tdir[i];
		}
        //tan1[i1] += sdir;
        //tan1[i2] += sdir;
        //tan1[i3] += sdir;
        //
        //tan2[i1] += tdir;
        //tan2[i2] += tdir;
        //tan2[i3] += tdir;
        
        tri++;
    }
    for (int a = 0; a < m_iNumVertices; a++)
	{
		CHLSL_Vertex &vert = m_Vertices[a];

		Vector n;
		Vector s;
		Vector t;
		VectorCopy( vert.normal, n.Base() );
		VectorCopy( vert.tangent_s, s.Base() );
		VectorCopy( vert.tangent_t, t.Base() );

		n.NormalizeInPlace();
		s.NormalizeInPlace();
		t.NormalizeInPlace();

#if 0
		Vector delta = s + ( t - s ) * 0.5f;
		Vector bidelta;
		CrossProduct( delta, n, bidelta );
		t = bidelta + ( delta - bidelta ) * 0.5f;
		t -= n * DotProduct( t, n );
		t.NormalizeInPlace();
		CrossProduct( n, t, s );
		s.NormalizeInPlace();
#endif
#if 1
		//if ( !IsFinite(t.x) || !IsFinite(t.y) || !IsFinite(t.z) )
		//	t.Init(0,0,1);
		//if ( !IsFinite(s.x) || !IsFinite(s.y) || !IsFinite(s.z) )
		//	s.Init(0,0,1);
		s = (s - n * DotProduct(n, s));
		t = (t - n * DotProduct(n, t)) * -1.0f;
		s.NormalizeInPlace();
		t.NormalizeInPlace();
		float w = (DotProduct(CrossProduct(n, s), t) < 0.0F) ? 1.0F : -1.0F;

#endif

		//t *= -1.0f;
		VectorCopy( s.Base(), vert.tangent_s );
		VectorCopy( t.Base(), vert.tangent_t );

		vert.tangent_s[3] = w;

	}
    
  //  for (long a = 0; a < m_iNumVertices; a++)
  //  {
		//CHLSL_Vertex &vert = *m_Vertices[a];
  //     // const Vector& n = 
		//Vector normal( vert.pos[0], vert.pos[1], vert.pos[2] );
  //      const Vector& t = tan1[a];
  //      
  //      // Gram-Schmidt orthogonalize
  //      tangent[a] = VectorNormalize( (t - n * DotProduct(n, t)) );
  //      
  //      // Calculate handedness
  //      tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
  //  }
}