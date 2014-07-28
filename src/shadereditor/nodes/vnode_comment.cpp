
#include "cbase.h"
#include "editorCommon.h"


CNodeComment::CNodeComment( CNodeView *p ) : BaseClass( "Group", p )
{
	m_bAffectSolver = false;

	m_flMinSizeX = 64 + CBORDER_SIZE * 2;

	m_iChannelInfo.SetDefaultChannels();
	m_iChannelInfo.bAllowChars = false;
	m_iChannelInfo.bHasAnyValues = true;
	m_iChannelInfo.iActiveChannels = 3;

	for ( int i = 0; i < 3; i++ )
	{
		m_iChannelInfo.flValue[i] = RandomFloat( 0, 1 );
		m_iChannelInfo.bChannelAsValue[i] = true;
	}

	m_iChannelInfo.PrintTargetString( m_szConstantString, sizeof( m_szConstantString ) );
}

CNodeComment::~CNodeComment()
{
}

const int CNodeComment::GetAllowedFlowgraphTypes()
{
	return CNodeView::FLOWGRAPH_POSTPROC | CNodeView::FLOWGRAPH_HLSL | CNodeView::FLOWGRAPH_HLSL_TEMPLATE;
}

KeyValues *CNodeComment::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetString( "szConstantString", m_szConstantString );

	return pKV;
}

void CNodeComment::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	BaseClass::RestoreFromKeyValues_Specific( pKV );

	char tmp[MAX_PATH];
	Q_snprintf( tmp, sizeof(tmp), "%s", pKV->GetString( "szConstantString" ) );

	m_iChannelInfo.Tokenize( tmp );
	if ( m_iChannelInfo.iActiveChannels >= 3 )
		m_iChannelInfo.flValue[3] = max( 0.1f, m_iChannelInfo.flValue[3] );

	m_iChannelInfo.PrintTargetString( m_szConstantString, MAX_PATH );
}

void CNodeComment::UpdateSize()
{
	BaseClass::UpdateSize();

	//m_vecSize.Init( 0, 60 );
	//UpdateContainerBounds();
}

Vector2D CNodeComment::GetSelectionBoundsMinNodeSpace()
{
	return GetContainerBorderMin( CBORDER_TOP_LEFT ) + Vector2D( CBORDER_SIZE, 0 );
}
Vector2D CNodeComment::GetSelectionBoundsMaxNodeSpace()
{
	return GetContainerBorderMin( CBORDER_TOP_LEFT ) + Vector2D( CBORDER_SIZE + 64, GetBorderSize() + CBORDER_SIZE );
}

bool CNodeComment::IsWithinBounds_Base( const Vector2D &pos )
{
	return IsInBounds_DragMove( pos ) || BaseClass::IsInBorderBounds( pos ) != CBORDER_INVALID; // BaseClass::IsWithinBounds_Base( pos );
}

int CNodeComment::IsInBorderBounds( const Vector2D &pos )
{
	if ( IsInBounds_DragMove( pos ) )
		return CBORDER_INVALID;

	return BaseClass::IsInBorderBounds( pos );
}

bool CNodeComment::IsInBounds_DragMove( const Vector2D &pos )
{
	Vector2D _min, _max;
	_min = GetSelectionBoundsMinNodeSpace();
	_max = GetSelectionBoundsMaxNodeSpace();

	if ( pos.x >= _min.x && pos.y >= _min.y &&
		pos.x <= _max.x && pos.y <= _max.y )
		return true;
	return false;
}

Vector2D CNodeComment::GetContainerSensitiveCenter()
{
	Vector2D sMin = GetSelectionBoundsMinNodeSpace() + Vector2D( 0, GetBorderSize() );
	Vector2D sMax = GetSelectionBoundsMaxNodeSpace();

	return sMin + ( sMax - sMin ) * 0.5f;
}

bool CNodeComment::VguiDraw( bool bShadow )
{
#define TITEL_INSET_X NODE_DRAW_TITLE_SPACE
	if ( !ShouldSimpleDrawObject( pNodeView, pNodeView, m_vecBounds ) )
		return false;
	if ( !pNodeView )
		return false;

	//Vector2D title_min = GetBoundsTitleMin();
	//Vector2D title_max = GetBoundsTitleMax();
	//title_max.x -= TITEL_INSET_X;

	Vector2D box_min = GetBoundsBoxMin();
	Vector2D box_max = GetBoundsBoxMax();

	//pNodeView->ToPanelSpace( title_min );
	//pNodeView->ToPanelSpace( title_max );
	pNodeView->ToPanelSpace( box_min );
	pNodeView->ToPanelSpace( box_max );

	float flZoom = pNodeView->GetZoomScalar();
	float flO = NODE_DRAW_SHADOW_DELTA * flZoom;
	Vector2D offset( flO, flO );

	if ( bShadow )
	{
		//title_min += offset;
		//title_max += offset;
		box_min += offset;
		box_max += offset;
	}

	const bool bSelected = IsSelected();

	//else
	//{
	//	Color colTitleNoShadow = NODE_DRAW_COLOR_TITLE;
	//	if ( GetNumContainers() )
	//		colTitleNoShadow = NODE_DRAW_COLOR_HASCONTAINER_TITLE;
	//	else if ( bSelected )
	//		colTitleNoShadow = NODE_DRAW_COLOR_SELECTED_TITLE;
	//	surface()->DrawSetColor( colTitleNoShadow );
	//}

	//surface()->DrawFilledRect( title_min.x, title_min.y, title_max.x, title_max.y );



	if ( !bShadow )
	{
		Color boxNoShadow = NODE_DRAW_COLOR_BOX_COMMENT;

		if ( bSelected )
			boxNoShadow = NODE_DRAW_COLOR_SELECTED_BOX;
		else if ( GetNumSolvers() && !HasDummySolvers() )
			boxNoShadow = NODE_DRAW_COLOR_HASSOLVER_BOX;
		else
		{
			for ( int i = 0; i < m_iChannelInfo.iActiveChannels; i++ )
				boxNoShadow[i] = m_iChannelInfo.flValue[i] * 255.0f;
		}

		surface()->DrawSetColor( boxNoShadow );
	}
	else
		surface()->DrawSetColor( NODE_DRAW_COLOR_SHADOW );

	//surface()->DrawFilledRect( box_min.x, box_min.y, box_max.x, box_max.y );

	//Vector2D r0_min = GetContainerBorderMin( CBORDER_TOP_LEFT );
	//Vector2D r0_max = GetContainerBorderMax( CBORDER_TOP_RIGHT );
	//Vector2D r1_min = GetContainerBorderMin( CBORDER_RIGHT );
	//Vector2D r1_max = GetContainerBorderMax( CBORDER_RIGHT );
	//Vector2D r2_min = GetContainerBorderMin( CBORDER_BOTTOM_LEFT );
	//Vector2D r2_max = GetContainerBorderMax( CBORDER_BOTTOM_RIGHT );
	//Vector2D r3_min = GetContainerBorderMin( CBORDER_RIGHT );
	//Vector2D r3_max = GetContainerBorderMax( CBORDER_RIGHT );
	Vector2D rects_[8] = {
	GetContainerBorderMin( CBORDER_TOP_LEFT ),
	GetContainerBorderMax( CBORDER_TOP_RIGHT ),
	GetContainerBorderMin( CBORDER_RIGHT ),
	GetContainerBorderMax( CBORDER_RIGHT ),
	GetContainerBorderMin( CBORDER_BOTTOM_LEFT ),
	GetContainerBorderMax( CBORDER_BOTTOM_RIGHT ),
	GetContainerBorderMin( CBORDER_LEFT ),
	GetContainerBorderMax( CBORDER_LEFT ),
	};
	for ( int i = 0; i < 8; i++ )
		pNodeView->ToPanelSpace( rects_[i] );

	Vector2D sMin = GetSelectionBoundsMinNodeSpace() + Vector2D( 0, GetBorderSize() );
	Vector2D sMax = GetSelectionBoundsMaxNodeSpace();

	pNodeView->ToPanelSpace( sMin );
	pNodeView->ToPanelSpace( sMax );

	if ( bShadow )
	{
		for ( int i = 0; i < 8; i++ )
			rects_[i] += offset;

		sMin += offset;
		sMax += offset;
	}

	for ( int i = 0; i < 4; i++ )
		surface()->DrawFilledRect( rects_[i*2].x, rects_[i*2+1].y, rects_[i*2+1].x, rects_[i*2].y );

	surface()->DrawFilledRect( sMin.x, sMax.y, sMax.x, sMin.y );

#if 0
	if ( bShadow )
	{
		VguiDraw_Jacks( true );
	}
	else
	{
		VguiDraw_Jacks( false );
		Color borderCol = NODE_DRAW_COLOR_BORDER;
		if ( GetErrorLevel() == ERRORLEVEL_UNDEFINED )
			borderCol = DRAWCOLOR_ERRORLEVEL_UNDEFINED;
		else if ( GetErrorLevel() == ERRORLEVEL_FAIL )
			borderCol = DRAWCOLOR_ERRORLEVEL_FAIL;
		else if ( IsAllocating() )
			borderCol = NODE_DRAW_COLOR_HASSOLVER_BOX_ALLOC;
		surface()->DrawSetColor( borderCol );

		int borderSize = 1;
		surface()->DrawFilledRect( rects_[0].x-borderSize, rects_[1].y, rects_[0].x, rects_[4].y );
		surface()->DrawFilledRect( rects_[7].x-borderSize, rects_[7].y, rects_[7].x, rects_[6].y );

		surface()->DrawFilledRect( rects_[2].x, rects_[3].y, rects_[2].x+borderSize, rects_[2].y );
		surface()->DrawFilledRect( rects_[3].x-borderSize, rects_[1].y, rects_[3].x, rects_[4].y );

		surface()->DrawFilledRect( rects_[0].x, rects_[1].y, rects_[1].x, rects_[1].y+borderSize );
		surface()->DrawFilledRect( rects_[4].x, rects_[4].y-borderSize, rects_[5].x, rects_[4].y );

		surface()->DrawFilledRect( rects_[7].x-borderSize, rects_[0].y-borderSize, rects_[2].x+borderSize, rects_[0].y );
		surface()->DrawFilledRect( rects_[7].x-borderSize, rects_[5].y, rects_[2].x+borderSize, rects_[5].y+borderSize );
	}
#endif
	if ( !bShadow )
		DrawGrabIcon();

	if ( Q_strlen( szNodeName ) > 1 )
	{
		wchar_t szconverted[ 1024 ];
		int fontWide, fontTall;

		surface()->DrawSetTextColor( bShadow ? NODE_DRAW_COLOR_SHADOW : NODE_DRAW_COLOR_CUSTOMTITLE );

		bool bDraw;
		vgui::HFont hFont_Small = pNodeView->GetFontScaled(18,bDraw);
		if ( bDraw )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( szNodeName, szconverted, sizeof(szconverted) );

			surface()->DrawSetTextFont( hFont_Small );
			surface()->GetTextSize( hFont_Small, szconverted, fontWide, fontTall );
			surface()->DrawSetTextPos( sMin.x, sMax.y - fontTall - 3 * pNodeView->GetZoomScalar() );

			surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
		}
	}
	return true;
}