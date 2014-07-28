#include "cbase.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include "vgui/ISurface.h"
#include "ienginevgui.h"
#include <vgui_controls/button.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/richtext.h>

#include "materialsystem/imesh.h"
#include "materialsystem/ITexture.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/imaterialsystem.h"

#include <vgui/ILocalize.h>

#include "editorCommon.h"
#include "vJack.h"
#include "vBaseNode.h"



CJack::CJack( CBaseNode *p, int slot, bool input )
{
	pNode = p;
	pNodeView = p->GetParent();

	m_iJackType = 0;
	m_iJackColorCode = HLSLJACK_COLOR_NORM;

	m_vecPosition.Init();
	m_vecSize.Init( JACK_SIZE_X, JACK_SIZE_Y );

	bHasFocus = false;
	m_bInput = input;
	m_iSlot = slot;

	m_iSmartType = -1;
	bLockSmartType = false;

	m_pVolatileVar = NULL;
	m_iResourceType = RESOURCETYPE_VARIABLE;
	szName[0] = '\0';
}
CJack::~CJack()
{
	delete m_pVolatileVar;

	pNodeView->OnJackRemoved( this );
	PurgeBridges();
}

void CJack::ConnectBridge( CBridge *b )
{
	m_hBridgeList.AddToTail( b );
	pNode->UpdatePreviewAllowed();

	const bool bConnected = b->GetEndJack( this ) && b->GetEndNode( this );
	if ( !bConnected )
		return;

	UpdateSmartType( b );

	pNode->OnUpdateHierachy( b, this );

	CBaseNode *pOtherNode = b->GetEndNode( this );
	pOtherNode->OnUpdateHierachy( b, b->GetEndJack( this ) );
	pOtherNode->UpdatePreviewAllowed();
}
void CJack::OnBridgeRemoved( CBridge *b )
{
	CBaseNode *pOtherNode = b->GetEndNode( this );
	CJack *pOtherJack = b->GetEndJack( this );

	m_hBridgeList.FindAndRemove( b );

	if ( IsInput() )
	{
		UpdateSmartType( NULL );
		pNode->OnUpdateHierachy( NULL, this );
	}

	if ( IsOutput() && pOtherNode && pOtherJack )
		pOtherNode->OnUpdateHierachy( NULL, pOtherJack );

	pNode->UpdatePreviewAllowed();
	if ( pOtherNode )
		pOtherNode->UpdatePreviewAllowed();
}

void CJack::PurgeBridges()
{
	SaveDeleteVector( m_hBridgeList );
}
void CJack::DisconnectBridges()
{
	PurgeBridges();
	//for ( int b = 0; b < m_hBridgeList.Count(); b++ )
	//{
	//	CBridge *pB = m_hBridgeList[ b ];
	//	if ( this == pB->GetDestinationJack() )
	//		pB->DisconnectDestination();
	//	else
	//		pB->DisconnectSource();
	//}
}
const int CJack::GetNumBridges()
{
	return m_hBridgeList.Count();
}
int CJack::GetNumBridgesConnected()
{
	int b = 0;
	for ( int i = 0; i < m_hBridgeList.Count(); i++ )
	{
		if ( m_hBridgeList[i]->GetEndJack( this ) && m_hBridgeList[i]->GetEndNode( this ) )
			b++;
	}
	return b;
}
CBridge *CJack::BridgeBeginBuild()
{
	CBridge *b = new CBridge(pNodeView);
	if ( !IsInput() )
		b->ConnectSource( this, GetParentNode() );
	else
		b->ConnectDestination( this, GetParentNode() );
	return b;
}
void CJack::BridgeEndBuild( CBridge *pBridge )
{
	if ( !IsInput() )
		pBridge->ConnectSource( this, GetParentNode() );
	else
		pBridge->ConnectDestination( this, GetParentNode() );
	pNodeView->AddBridgeToList( pBridge );

	CJack *pJSrc = pBridge->GetInputJack();
	if ( !pJSrc )
		return;

	if ( GetSmartType() == HLSLVAR_PP_MASTER )
	{
		for ( int i = 0; i < pJSrc->GetNumBridges(); i++ )
		{
			CBridge *pB = pJSrc->GetBridge( i );
			if ( pB == pBridge )
				continue;

			delete pB;
			i--;
		}
	}
}

void CJack::UpdateSmartType( CBridge *b )
{
	if ( IsOutput() )
		return;

	if ( !b || !b->GetEndJack( this ) )
		return SetSmartType( -1 );

	//SetSmartType( b->GetEndJack( this )->GetSmartType() );
}
void CJack::SetSmartTypeLocked( const bool b )
{
	bLockSmartType = b;
}

//void CJack::SetSmartTypeFromFlag( const int flag )
//{
//}
void CJack::SetSmartType( const int t )
{
	if ( IsSmartTypeLocked() )
		return;

	m_iSmartType = t;
}

const ResourceType_t &CJack::GetResourceType()
{
	return m_iResourceType;
}
void CJack::SetResourceType( const int &type )
{
	m_iResourceType = ((ResourceType_t)type);
}
CHLSL_Var *CJack::AllocateVarFromSmartType( bool bAssigntoSelf )
{
#if SHOW_SEDIT_ERRORS
	AssertMsg( (GetSmartType() >= 0), "jack not ready!!\n" );
	Assert( IsOutput() );
#endif

	CHLSL_Var *var = new CHLSL_Var( ((HLSLVariableTypes)GetSmartType()) );
	if ( bAssigntoSelf )
		SetTemporaryVarTarget( var );
	return var;
}
void CJack::SetTemporaryVarTarget( CHLSL_Var *var )
{
	m_pVolatileVar = var;
}
CHLSL_Var *CJack::GetTemporaryVarTarget()
{
//#if SHOW_SEDIT_ERRORS
//	Assert(!!m_pVolatileVar);
//#endif
	return m_pVolatileVar;
}
CHLSL_Var *CJack::GetTemporaryVarTarget_End()
{
#if SHOW_SEDIT_ERRORS
	Assert(!!GetNumBridges());
#endif
	return GetBridge(0)->GetEndJack(this)->GetTemporaryVarTarget();
}
CHLSL_Var *CJack::GetTemporaryVarTarget_End_Smart( int varTypes, bool bAllowOverwrite )
{
#if SHOW_SEDIT_ERRORS
	Assert(!!GetNumBridges());
	Assert(IsInput());
#endif
	for ( int i = 0; i < GetNumBridges(); i++ )
	{
		CBridge *pBridge = GetBridge( i );
		CJack *pEndJack = pBridge->GetEndJack( this );
		if ( !pEndJack )
			continue;

		CHLSL_Var *potentialVar = pEndJack->GetTemporaryVarTarget();
		if ( !potentialVar )
			continue;
		if ( !(potentialVar->GetType() & varTypes) )
			continue;

		int numPartnerBridges = pEndJack->GetNumBridges();
		if ( numPartnerBridges < 2 )
			return potentialVar;
	
		bool bTakeThis = true;
		if ( !bAllowOverwrite )
		{
			for ( int a = 0; a < numPartnerBridges; a++ )
			{
				CBridge *pPartnerBridge = pEndJack->GetBridge( a );
				if ( pPartnerBridge->GetEndNode( pEndJack ) != GetParentNode() )
					bTakeThis = false;
			}
		}
		if ( bTakeThis )
			return pEndJack->GetTemporaryVarTarget();
	}
	return NULL;
}
void CJack::ClearTemporaryVarTarget()
{
	m_pVolatileVar = NULL;
}

void CJack::ClearVarFlags()
{
	m_iJackType = 0;
}
void CJack::AddVarFlags( int f )
{
	m_iJackType |= f;
}
void CJack::RemoveVarFlags( int f )
{
	m_iJackType &= ~f;
}
bool CJack::HasVarFlag( int f )
{
	return ( ( m_iJackType & f ) != 0 );
}
void CJack::SetName( const char *name )
{
	if ( Q_strlen( name ) < 1 )
		return;
	Q_snprintf( szName, sizeof(szName), "%s", name );
	pNode->UpdateSize();
}

Vector2D CJack::GetBoundsMin()
{
	return m_vecPosition;
}
Vector2D CJack::GetBoundsMax()
{
	return m_vecPosition + m_vecSize;
}
Vector2D CJack::GetCenter()
{
	return GetBoundsMin() + ( GetBoundsMax() - GetBoundsMin() ) * 0.5f;
}
Vector2D CJack::GetBoundsMinPanelSpace()
{
	Vector2D _min( GetBoundsMin().x, GetBoundsMax().y );
	pNodeView->ToPanelSpace( _min );
	return _min;
}
Vector2D CJack::GetBoundsMaxPanelSpace()
{
	Vector2D _max( GetBoundsMax().x, GetBoundsMin().y );
	pNodeView->ToPanelSpace( _max );
	return _max;
}
Vector2D CJack::GetCenterPanelSpace()
{
	Vector2D pos = GetCenter();
	pNodeView->ToPanelSpace( pos );
	return pos;
}
bool CJack::IsWithinBounds_Base( Vector2D pos )
{
	Vector2D _min, _max;
	_min = GetBoundsMin();
	_max = GetBoundsMax();

	if ( pos.x >= _min.x && pos.y >= _min.y &&
		pos.x <= _max.x && pos.y <= _max.y )
		return true;
	return false;
}

void CJack::SetPosition( Vector2D vec, bool bCenter )
{
	if ( bCenter )
	{
		Vector2D delta = GetBoundsMin() + ( GetBoundsMax() - GetBoundsMin() ) * 0.5f;
		vec -= delta;
	}
	m_vecPosition = vec;
}
void CJack::UpdatePosition()
{
	Vector2D parentBoxStart = pNode->GetBoundsBoxMin();
	Vector2D parentBoxEnd = pNode->GetBoundsBoxMax();
	const bool bContainer = !!pNode->GetAsContainer();

	float x = m_bInput ? parentBoxStart.x : (parentBoxEnd.x + (bContainer?CBORDER_SIZE:0));
	x += (m_bInput ? -JACK_SIZE_X : 0);
	float y = parentBoxStart.y - ( JACK_SIZE_Y + JACK_DELTA_Y ) * (m_iSlot) - JACK_SIZE_Y * 2;

	SetPosition( Vector2D( x, y ) );
}
void CJack::UpdatePositionData()
{
	UpdatePosition();
}
Vector2D CJack::GetPosition()
{
	return m_vecPosition;
}
void CJack::OnParentMoved()
{
	UpdatePosition();
}

void CJack::VguiDraw( bool bShadow )
{
	if ( !pNodeView->ShouldDraw_Jacks() )
		return;

	Vector2D panelMin = GetBoundsMinPanelSpace();
	Vector2D panelMax = GetBoundsMaxPanelSpace();

	float flZoom = pNodeView->GetZoomScalar();
	if ( bShadow )
	{
		float flO = NODE_DRAW_SHADOW_DELTA * flZoom;
		Vector2D offset( flO, flO );
		panelMin += offset;
		panelMax += offset;
	}

	Color colText = JACK_COLOR_DATATYPE;

	switch ( m_iJackColorCode )
	{
	case HLSLJACK_COLOR_NORM:
		surface()->DrawSetColor( JACK_COLOR_NORM );
		break;
	case HLSLJACK_COLOR_RGB:
	case HLSLJACK_COLOR_R:
		surface()->DrawSetColor( JACK_COLOR_R );
		break;
	case HLSLJACK_COLOR_G:
		surface()->DrawSetColor( JACK_COLOR_G );
		break;
	case HLSLJACK_COLOR_B:
		surface()->DrawSetColor( JACK_COLOR_B );
		break;
	case HLSLJACK_COLOR_A:
		surface()->DrawSetColor( JACK_COLOR_A );
		break;
	case HLSLJACK_COLOR_PPMASTER:
		surface()->DrawSetColor( JACK_COLOR_PPMASTER );
		colText = JACK_COLOR_DATATYPE_DARK;
		break;
	}

	if ( HasFocus() )
		surface()->DrawSetColor( JACK_COLOR_FOCUS );

	if ( bShadow )
		surface()->DrawSetColor( NODE_DRAW_COLOR_SHADOW );
	else
		VguiDrawName();

	if ( m_iJackColorCode != HLSLJACK_COLOR_RGB || bShadow || HasFocus() )
	{
		surface()->DrawFilledRect( panelMin.x, panelMin.y, panelMax.x, panelMax.y );
	}
	else
	{
		float deltax = panelMax.x - panelMin.x;
		deltax /= 3;

		surface()->DrawSetColor( JACK_COLOR_R );
		surface()->DrawFilledRect( panelMin.x, panelMin.y, panelMin.x + deltax, panelMax.y );
		surface()->DrawSetColor( JACK_COLOR_G );
		surface()->DrawFilledRect( panelMin.x + deltax, panelMin.y, panelMin.x + deltax * 2, panelMax.y );
		surface()->DrawSetColor( JACK_COLOR_B );
		surface()->DrawFilledRect( panelMin.x + deltax * 2, panelMin.y, panelMin.x + deltax * 3, panelMax.y );
	}

	if ( pNodeView->ShouldDraw_Datatypes() )
	{
		wchar_t szconverted[ 1024 ];
		int fontWide, fontTall;

		if ( !bShadow )
		{
			bool bDraw;
			vgui::HFont hFont_Small = pNodeView->GetFontScaled(3,bDraw);
			if ( bDraw )
			{
				int smartType = GetSmartType();
				if ( smartType >= 0 )
				{
					smartType = GetVarFlagsVarValue( smartType );
					g_pVGuiLocalize->ConvertANSIToUnicode( GetVarTypeName( smartType ), szconverted, sizeof(szconverted) );
					surface()->DrawSetTextColor( colText );
				}
				else
				{
					g_pVGuiLocalize->ConvertANSIToUnicode( "NONE", szconverted, sizeof(szconverted) );
					surface()->DrawSetTextColor( JACK_COLOR_DATATYPE_UNDEFINED );
				}

				surface()->DrawSetTextFont( hFont_Small );
				surface()->GetTextSize( hFont_Small, szconverted, fontWide, fontTall );

				Vector2D mid = panelMin + ( panelMax - panelMin ) * 0.5f;
				surface()->DrawSetTextPos( mid.x - fontWide * 0.5f, mid.y - fontTall * 0.5f );

				surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
			}
		}
	}
}
void CJack::VguiDrawName()
{
	if ( Q_strlen(szName) < 1 )
		return;

	Vector2D _min = GetBoundsMinPanelSpace();
	Vector2D _max = GetBoundsMaxPanelSpace();

	Vector2D drawpos( _min + ( _max - _min ) * 0.5f );

	bool bDraw;
	HFont font = pNodeView->GetFontScaled( 9, bDraw );
	if ( !bDraw )
		return;

	wchar_t szconverted[ 1024 ];
	g_pVGuiLocalize->ConvertANSIToUnicode( szName, szconverted, sizeof(szconverted)  );

	int fontWide, fontTall;
	surface()->DrawSetTextFont( font );
	surface()->GetTextSize( font, szconverted, fontWide, fontTall );

	Vector2D edge = m_bInput ? GetBoundsMax() : GetBoundsMin();
	edge.x = edge.x + ( m_bInput ? JACK_TEXT_INSET : -JACK_TEXT_INSET );
	pNodeView->ToPanelSpace( edge );
	if ( !m_bInput )
		edge.x -= fontWide;

	surface()->DrawSetTextPos( edge.x, drawpos.y - fontTall * 0.5f );

	surface()->DrawSetTextColor( HasFocus() ? JACK_COLOR_NAME_FOCUS : JACK_COLOR_NAME );
	surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
}
int CJack::GetFinalTextInset()
{
	int i = 0;
	if ( Q_strlen(szName) >= 1 )
	{
		float z = 1.15f;
		bool bDraw;
		HFont font = pNodeView->GetFontScaled( 9, bDraw, &z );
		Assert( bDraw );

		wchar_t szconverted[ 1024 ];
		Q_memset( szconverted, 0, sizeof( szconverted ) );
		g_pVGuiLocalize->ConvertANSIToUnicode( szName, szconverted, sizeof(szconverted)  );

		int fontWide, fontTall;
		surface()->DrawSetTextFont( font );
		surface()->GetTextSize( font, szconverted, fontWide, fontTall );

		i = fontWide + JACK_TEXT_INSET + 3;
	}
	return i;
}
