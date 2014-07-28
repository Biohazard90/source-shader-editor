
#include "cbase.h"
#include "editorCommon.h"

#define BRIDGE_DRAW_SEGMENTS_MIN 8
#define BRIDGE_DRAW_SEGMENTS_MAX 64

#define BRIDGE_DRAW_ARROW_MIN 4
#define BRIDGE_DRAW_ARROW_MAX 16

CBridge::CBridge( CNodeView *p )
{
	pNodeView = p;

	pJ_Src = NULL;
	pJ_Dst = NULL;

	pN_Src = NULL;
	pN_Dst = NULL;

	//bDrawOrange = false;
	iTempColor = TMPCOL_NONE;

	SetupVguiTex( m_iTex_Arrow, "shadereditor/arrow" );

	start_old.Init();
	end_old.Init();

	m_flLength = 0;

	iErrorLevel = ERRORLEVEL_NONE;
}
CBridge::~CBridge()
{
	CJack *j = pJ_Dst;
	pJ_Dst = NULL;
	if ( j )
		j->OnBridgeRemoved( this );
	j = pJ_Src;
	pJ_Src = NULL;
	if ( j )
		j->OnBridgeRemoved( this );
	pNodeView->OnBridgeRemoved( this );
}

void CBridge::ConnectSource( CJack *j, CBaseNode *n )
{
	pJ_Src = j;
	pN_Src = n;

	j->ConnectBridge( this );
}

void CBridge::ConnectDestination( CJack *j, CBaseNode *n )
{
	pJ_Dst = j;
	pN_Dst = n;

	j->ConnectBridge( this );
}

void CBridge::DisconnectSource()
{
	CJack *j = pJ_Src;
	pJ_Src = NULL;
	pN_Src = NULL;
	if ( j )
		j->OnBridgeRemoved( this );

}
void CBridge::DisconnectDestination()
{
	CJack *j = pJ_Dst;
	pJ_Dst = NULL;
	pN_Dst = NULL;
	if ( j )
		j->OnBridgeRemoved( this );
}

CBaseNode *CBridge::GetEndNode( CBaseNode *n )
{
	if ( !n || ( pN_Src != n && pN_Dst != n ) )
		return NULL;
	return (pN_Dst == n) ? pN_Src : pN_Dst;
}
CBaseNode *CBridge::GetEndNode( CJack *j )
{
	return GetEndNode( j->GetParentNode() );
}
CJack *CBridge::GetEndJack( CBaseNode *n )
{
	if ( !n || ( pN_Src != n && pN_Dst != n ) )
		return NULL;
	return (pN_Dst == n) ? pJ_Src : pJ_Dst;
}
CJack *CBridge::GetEndJack( CJack *j )
{
	return GetEndJack( j->GetParentNode() );
}

void CBridge::SetTemporaryTarget( Vector2D tg )
{
	tmp = tg;
}


inline float BSplineSingle( float start, float end, float cp1, float cp2, float fraction )
{
	float tmp = 0;

	float hlp1, hlp2, mid;
	mid = Lerp( fraction, cp1, cp2);
	hlp1 = Lerp( fraction, Lerp( fraction, start, cp1), mid );
	hlp2 = Lerp( fraction, mid, Lerp( fraction, cp2, end) );
	tmp = Lerp( fraction, hlp1, hlp2 );
	return tmp;
}
inline void BSplineVector2D( Vector2D &start, Vector2D &end, Vector2D &cp1, Vector2D &cp2,
							float fraction, bool autosmooth_key1, bool autosmooth_key2, Vector2D &output )
{
	if (autosmooth_key1)
	{
		Vector2D dir = cp1 - start;
		dir.NormalizeInPlace();

		cp1 = start + dir * (end-start).Length() * 0.4f;
	}
	if (autosmooth_key2)
	{
		Vector2D dir = cp2 - end;
		dir.NormalizeInPlace();

		cp2 = end + dir * (end-start).Length() * 0.4f;
	}

	output.Init();
	for (int i = 0; i < 2; i++)
		output[i] = BSplineSingle( start[i], end[i], cp1[i], cp2[i], fraction );
}

void CBridge::SetTemporaryColor( int mode )
{
	if ( iTempColor > mode )
		return;

	iTempColor = mode;
}

void CBridge::VguiDraw( const bool bShadow )
{
	CJack *ji = GetInputJack();
	CJack *jd = GetDestinationJack();

	Vector2D start = ji ? ji->GetCenter() : tmp;
	Vector2D end = jd ? jd->GetCenter() : tmp;

#define BRIDGEHULLEXTRUDE 60.0f
	Vector2D bounds_min = start;
	Vector2D bounds_max = end;
	if ( bounds_max.x < bounds_min.x )
		swap( bounds_min.x, bounds_max.x );
	if ( bounds_max.y < bounds_min.y )
		swap( bounds_min.y, bounds_max.y );
	bounds_min -= Vector2D( BRIDGEHULLEXTRUDE, BRIDGEHULLEXTRUDE );
	bounds_max += Vector2D( BRIDGEHULLEXTRUDE, BRIDGEHULLEXTRUDE );
	swap( bounds_min.y, bounds_max.y );
	if ( !::ShouldSimpleDrawObject( pNodeView, pNodeView, Vector4D( bounds_min.x, bounds_min.y, bounds_max.x, bounds_max.y ) ) )
		return;

	if ( ji )
	{
		Vector2D tmpBounds = ji->GetBoundsMax();
		start.x = tmpBounds.x;
	}
	if ( jd )
	{
		Vector2D tmpBounds = jd->GetBoundsMin();
		end.x = tmpBounds.x;
	}

	if ( bShadow )
	{
		start += Vector2D( NODE_DRAW_SHADOW_DELTA, -NODE_DRAW_SHADOW_DELTA );
		end += Vector2D( NODE_DRAW_SHADOW_DELTA, -NODE_DRAW_SHADOW_DELTA );
	}

	float extend = 30;
	Vector2D delta = end - start;
	float len = delta.Length();
	float addInverse = (delta.x < 0) ? RemapValClamped(abs(delta.x),0,100,0,0.045f) : 0;

	extend += len * ( 0.2f + addInverse );

	addInverse = (delta.x < 0) ? RemapValClamped(abs(delta.x),0,100,0,180) : 0;
	addInverse *= Sign( delta.y ) * RemapValClamped( abs(delta.y), 0, 50, 0, 1 );

	CJack *relativeJack_1 = (delta.y>0) ? jd : ji;
	CBaseNode *pN1 = relativeJack_1 ? relativeJack_1->GetParentNode() : NULL;

	float desiredMultiply1 = pN1 ? (RemapVal( abs( pN1->GetSize().y ), 0, 100, 0, 1.2f )) : 1.2f;
	float desiredMultiply2 = 0.65f;

	float mulAdd_Start = (delta.y < 0) ? desiredMultiply1 : desiredMultiply2;
	float mulAdd_End = (delta.y >= 0) ? desiredMultiply1 : desiredMultiply2;

	float scaleMaster = RemapValClamped( len, 0, 50, 0, 1 );

	Vector2D cp1 = start + Vector2D( extend, addInverse * mulAdd_Start ) * scaleMaster;
	Vector2D cp2 = end - Vector2D( extend, addInverse * mulAdd_End ) * scaleMaster;

	pNodeView->ToPanelSpace( start );
	pNodeView->ToPanelSpace( end );

	pNodeView->ToPanelSpace( cp1 );
	pNodeView->ToPanelSpace( cp2 );

	Color col = Color(0,0,0,128);

	if ( bShadow )
		col = NODE_DRAW_COLOR_SHADOW;
	else
	{
		if ( ji && jd )
		{
			if ( GetErrorLevel() == ERRORLEVEL_UNDEFINED )
				col = DRAWCOLOR_ERRORLEVEL_UNDEFINED;
			else if ( GetErrorLevel() == ERRORLEVEL_FAIL )
				col = DRAWCOLOR_ERRORLEVEL_FAIL;
			else
			{
				col = Color(0,0,0,255);

				if ( ji->GetJackType() & HLSLVAR_PP_MASTER )
					col = JACK_COLOR_PPMASTER;
			}

			if ( iTempColor != TMPCOL_NONE )
			{
				switch ( iTempColor )
				{
				case TMPCOL_ORANGE:
						col = DRAWCOLOR_BRIDGE_TMP_COL_ORANGE;
					break;
				case TMPCOL_TRANS:
						col[3] = DRAWCOLOR_BRIDGE_TMP_COL_TRANS.a();
					break;
				}

				iTempColor = TMPCOL_NONE;
			}
		}
	}

	surface()->DrawSetColor( col );
	static IMaterial *pMatArrow = materials->FindMaterial( "shadereditor/arrow", TEXTURE_GROUP_OTHER );
	static unsigned int iVar = 0;
	IMaterialVar *pVarGlowCol = pMatArrow->FindVarFast( "$glowcolor", &iVar );

	const float scalar = 255.0f;
	if ( pVarGlowCol )
		pVarGlowCol->SetVecValue( col[0] / scalar, col[1] / scalar, col[2] / scalar );

	//if ( ( start_old != start || end_old != end ) && !bShadow )
	//{
	//	double approxLength = 0;
	//	Vector2D append = start;

	//	for ( int i = 0; i <= 1024; i++ )
	//	{
	//		float frac = clamp( i / ( (float) 1024 ), 0, 1 );
	//		Vector2D goal;
	//		BSplineVector2D( start, end, cp1, cp2, frac, false, false, goal );
	//		approxLength += ( goal - append ).LengthSqr();
	//		append = goal;
	//	}

	//	m_flLength = FastSqrt( approxLength );
	//	start_old = start;
	//	end_old = end;
	//}

	m_flLength = (start-end).Length();

	float arrowLength = 0.01f;
	//float normalizedLength = m_flLength / pNodeView->GetZoomScalar();
	float normalizedLength = m_flLength / pNodeView->GetZoomScalar();
	if ( normalizedLength < arrowLength )
		return;

	Vertex_t vert[4];

	Vector2D arrowStart;
	BSplineVector2D( start, end, cp1, cp2, 1.0f - (arrowLength/normalizedLength), false, false, arrowStart );

	Vector crossed3D;
	Vector2D deltaArrow = end - arrowStart;
	deltaArrow.NormalizeInPlace();
	deltaArrow *= 15.0f * pNodeView->GetZoomScalar();
	arrowStart = end - deltaArrow;

	Vector2D offsetX = deltaArrow * 0.15f;

	CrossProduct( Vector( deltaArrow.x, deltaArrow.y, 0 ), Vector( 0,0,1 ), crossed3D );
	Vector2D side( crossed3D.x, crossed3D.y );
	side.NormalizeInPlace();

	side *= deltaArrow.Length();
	vert[0].Init( arrowStart + side * 0.5f + offsetX, Vector2D(0,0) );
	vert[1].Init( end + side * 0.5f + offsetX, Vector2D(1,0) );
	vert[2].Init( end - side * 0.5f + offsetX, Vector2D(1,1) );
	vert[3].Init( arrowStart - side * 0.5f + offsetX, Vector2D(0,1) );

	surface()->DrawSetTexture( m_iTex_Arrow );
	surface()->DrawTexturedPolygon( 4, vert );

	arrowStart = end - deltaArrow * 0.57f;

	int segments = ((int) RemapValClamped( pNodeView->GetZoomScalar(), 0, VIEWZOOM_OUT_MAX, BRIDGE_DRAW_SEGMENTS_MIN, BRIDGE_DRAW_SEGMENTS_MAX ) );
	segments -= RemapValClamped( normalizedLength, 0, 1000, max(1, segments - 10), 0 );


	Vector2D append = start;
	for ( int i = 1; i <= segments; i++ )
	{
		float frac = clamp( i / ( (float) segments ), 0, 1 );
		Vector2D goal;
		BSplineVector2D( start, arrowStart, cp1, cp2, frac, false, false, goal );
		surface()->DrawLine( append.x, append.y - 1, goal.x, goal.y - 1 );
		surface()->DrawLine( append.x, append.y, goal.x, goal.y );
		surface()->DrawLine( append.x, append.y + 1, goal.x, goal.y + 1 );
		append = goal;
	}
}
