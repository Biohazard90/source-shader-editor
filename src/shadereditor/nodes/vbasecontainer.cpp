
#include "cbase.h"
#include "editorcommon.h"
#include <vgui/ILocalize.h>

//CBaseContainerNode::CBaseContainerNode( CNodeView *p ) : BaseClass( "BaseContainer", p )
CBaseContainerNode::CBaseContainerNode( const char *opName, CNodeView *p ) : BaseClass( opName, p )
{
	m_bAffectSolver = true;

	p->OnContainerAdded( this );
	m_vecContainerExtents.Init( 128, 384, -128 );

	m_iActiveScalingMode = CBORDER_INVALID;
	m_vecMouseStartPos.Init();
	Q_memset( m_vecAccumulatedExtent, 0, sizeof( m_vecAccumulatedExtent ) );
	bLastHierachyErrorTest = false;

	m_flMinSizeX = 50;
	m_flMinSizeY = 30;

	SetupVguiTex( m_iGrabIcon, "shadereditor/grabicon" );

}
CBaseContainerNode::~CBaseContainerNode()
{
	RemoveAllChildren();
	pNodeView->OnContainerRemoved( this );
}

void CBaseContainerNode::Spawn()
{
	BaseClass::Spawn();

	UpdateOnMove();
}

CBaseContainerNode *CBaseContainerNode::GetAsContainer()
{
	return this;
}
void CBaseContainerNode::UpdateContainerBounds()
{
	m_vecBounds.x = m_vecPosition.x - 30;
	m_vecBounds.w = GetContainerMins().y - 30;
	Vector2D maxs = GetContainerMaxs();
	m_vecBounds.z = maxs.x + 30;
	m_vecBounds.y = maxs.y + 30;
}
void CBaseContainerNode::UpdateSize()
{
	int maxJacks = max( m_hInputs.Count(), m_hOutputs.Count() );
	float sizeMin = ( JACK_SIZE_Y + JACK_DELTA_Y ) * maxJacks + JACK_SIZE_Y;
	sizeMin = max( m_flMinSizeY, sizeMin );
	m_vecBorderInfo.Init();
	m_vecSize.Init( max( m_flMinSizeX, 0 ), -sizeMin );
	TouchJacks();
	//UpdateSimpleObjectBounds( m_vecPosition, m_vecSize, m_vecBounds );
	UpdateContainerBounds();
}
void CBaseContainerNode::SetPosition( Vector2D vec, bool bCenter )
{
	BaseClass::SetPosition( vec, bCenter );
	UpdateContainerBounds();
	UpdateOnMove();
}

bool CBaseContainerNode::IsAffectingSolver()
{
	return m_bAffectSolver;
}

void CBaseContainerNode::OnLeftClick( Vector2D &pos )
{
	m_iActiveScalingMode = IsInBorderBounds( pos );
	m_vecMouseStartPos = pos;
	Q_memset( m_vecAccumulatedExtent, 0, sizeof( m_vecAccumulatedExtent ) );

	//if ( input()->IsKeyDown( KEY_RALT ) || input()->IsKeyDown( KEY_LALT ) )
	//{
	//	for ( int i = 0; i < m_hChildren.Count(); i++ )
	//		m_hChildren[i]->SetSelected( true );
	//}
}
void CBaseContainerNode::OnDrag( Vector2D &delta )
{
	if ( m_iActiveScalingMode == CBORDER_INVALID )
		return BaseClass::OnDrag( delta );

	//Vector2D pos = GetPosition();

	Vector4D delta_NESW;
	Vector2D pos_delta;
	delta_NESW.Init();
	pos_delta.Init();

	Vector2D half( delta.x * 0.5f, delta.y * 0.5f );

	switch ( m_iActiveScalingMode )
	{
	default:
		Assert( 0 );
	case CBORDER_TOP_LEFT:
		delta_NESW.x += half.y;
		delta_NESW.z -= half.y;
		delta_NESW.y -= delta.x;
		pos_delta.x += delta.x;
		pos_delta.y += half.y;
		break;
	case CBORDER_TOP:
		delta_NESW.x += half.y;
		delta_NESW.z -= half.y;
		pos_delta.y += half.y;
		break;
	case CBORDER_TOP_RIGHT:
		delta_NESW.x += half.y;
		delta_NESW.z -= half.y;
		delta_NESW.y += delta.x;
		pos_delta.y += half.y;
		break;
	case CBORDER_RIGHT:
		delta_NESW.y += delta.x;
		break;
	case CBORDER_BOTTOM_RIGHT:
		delta_NESW.x -= half.y;
		delta_NESW.z += half.y;
		delta_NESW.y += delta.x;
		pos_delta.y += half.y;
		break;
	case CBORDER_BOTTOM:
		delta_NESW.x -= half.y;
		delta_NESW.z += half.y;
		pos_delta.y += half.y;
		break;
	case CBORDER_BOTTOM_LEFT:
		delta_NESW.x -= half.y;
		delta_NESW.z += half.y;
		delta_NESW.y -= delta.x;
		pos_delta.x += delta.x;
		pos_delta.y += half.y;
		break;
	case CBORDER_LEFT:
		delta_NESW.y -= delta.x;
		pos_delta.x += delta.x;
		break;
	}

	for ( int i = 0; i < 5; i++ )
	{
		if ( m_vecAccumulatedExtent[i] )
		{
			float *target = NULL;
			if ( i >= 3 )
				target = &pos_delta[ i - 3 ];
			else
				target = &delta_NESW[ i ];

			const bool bWasNeg = m_vecAccumulatedExtent[ i ] < 0;
			m_vecAccumulatedExtent[ i ] += *target;
			const bool bIsNeg = m_vecAccumulatedExtent[ i ] < 0;
			if ( bWasNeg && bIsNeg || !bWasNeg && !bIsNeg )
				*target = 0;
			else
			{
				*target = m_vecAccumulatedExtent[ i ] * 1;
				m_vecAccumulatedExtent[ i ] = 0;
			}
		}
	}

	Vector oldExtents = m_vecContainerExtents;
	for ( int i = 0; i < 3; i++ )
		m_vecContainerExtents[i] += delta_NESW[i];

	if ( m_vecContainerExtents[ 0 ] < m_flMinSizeY )
	{
		m_vecAccumulatedExtent[ 0 ] += min( m_vecContainerExtents[ 0 ] - m_flMinSizeY, 0 );
		m_vecAccumulatedExtent[ 4 ] -= min( m_vecContainerExtents[ 0 ] - m_flMinSizeY, 0 ) * Sign( pos_delta.y );

		float _save = oldExtents.x - m_flMinSizeY;
		_save = max( _save, 0 );
		pos_delta.y = _save * Sign( pos_delta.y );
		m_vecContainerExtents[ 0 ] = m_flMinSizeY;
	}
	if ( m_vecContainerExtents[ 2 ] > -m_flMinSizeY )
	{
		m_vecAccumulatedExtent[ 2 ] -= min( abs(m_vecContainerExtents[ 2 ]) - (m_flMinSizeY), 0 );
		m_vecContainerExtents[ 2 ] = -m_flMinSizeY;
	}
	if ( m_vecContainerExtents[ 1 ] < m_flMinSizeX )
	{
		if ( pos_delta.x > 0 )
		{
			float _save = oldExtents.y - m_flMinSizeX;
			_save = max( _save, 0 );
			pos_delta.x = _save;
		}
		m_vecAccumulatedExtent[ 3 ] -= min( m_vecContainerExtents[ 1 ] - m_flMinSizeX, 0 );
		m_vecAccumulatedExtent[ 1 ] += min( m_vecContainerExtents[ 1 ] - m_flMinSizeX, 0 );
		m_vecContainerExtents[ 1 ] = m_flMinSizeX;
	}

	m_vecContainerExtents[ 2 ] = -m_vecContainerExtents[ 0 ];

	Vector2D posOld = GetPosition();
	SetPosition( posOld + pos_delta );
}

void CBaseContainerNode::OnDragEnd()
{
	BaseClass::OnDragEnd();

	m_iActiveScalingMode = CBORDER_INVALID;
}

void CBaseContainerNode::SelectAllInBounds( bool bSelected, CUtlVector<CBaseNode*> *hNodes )
{
	CUtlVector<CBaseNode*> local;
	CUtlVector<CBaseNode*> *nodes = hNodes ? hNodes : &local;

	Assert( nodes );

	int numNodes = pNodeView->GetNumNodes();
	for ( int i = 0; i < numNodes; i++ )
	{
		CBaseNode *n = pNodeView->GetNode( i );
		if ( n == this )
			continue;
		Vector2D nodePos = n->GetContainerSensitiveCenter();

		bool bIsActive = IsInContainerBounds( nodePos );
		if ( !bIsActive )
			continue;

		if ( nodes->HasElement( n ) )
			continue;

		nodes->AddToTail( n );
		n->SetSelected( bSelected );
		if ( n->GetAsContainer() != NULL )
			n->GetAsContainer()->SelectAllInBounds( bSelected, nodes );
	}
}

Vector2D CBaseContainerNode::GetContainerMins()
{
	Vector2D center = GetCenter();
	center.x = GetBoundsMaxNodeSpace().x;
	//center.x += m_vecContainerExtents.y;
	center.y += m_vecContainerExtents.z;
	return center;
}
Vector2D CBaseContainerNode::GetContainerMaxs()
{
	Vector2D center = GetCenter();
	center.x = GetBoundsMaxNodeSpace().x;
	center.x += m_vecContainerExtents.y;
	center.y += m_vecContainerExtents.x;
	return center;
}
bool CBaseContainerNode::IsInContainerBounds( const Vector2D &pos )
{
	Vector2D bmin = GetContainerMins();
	Vector2D bmax = GetContainerMaxs();

	if ( pos.x >= bmin.x && pos.x <= bmax.x &&
		pos.y >= bmin.y && pos.y <= bmax.y )
		return true;
	return false;
}
int CBaseContainerNode::IsInBorderBounds( const Vector2D &pos )
{
	for ( int i = CBORDER_FIRST; i <= CBORDER_LAST; i++ )
	{
		Vector2D min = GetContainerBorderMin( i );
		Vector2D max = GetContainerBorderMax( i );
		if ( pos.x >= min.x && pos.x <= max.x &&
			pos.y >= min.y && pos.y <= max.y )
			return i;
	}
	return CBORDER_INVALID;
}

void CBaseContainerNode::RemoveAllChildren()
{
	const bool bSolver = IsAffectingSolver();

	for ( int i = 0; i < m_hChildren.Count() && bSolver; i++ )
	{
		if ( m_hChildren[i]->RemoveFromContainer( this ) )
			m_hChildren[i]->OnUpdateHierachy( NULL, NULL );
		else
			Assert( 0 );
	}

	m_hChildren.Purge();
}
bool CBaseContainerNode::HasChild( CBaseNode *child )
{
	return m_hChildren.IsValidIndex( m_hChildren.Find( child ) );
}
bool CBaseContainerNode::HasAnyChildren()
{
	return m_hChildren.Count() > 0;
}
//bool CBaseContainerNode::IsSolvable( bool bHierachyUp, CBaseNode *pCaller, CUtlVector< CBaseNode* > *hTested )
bool CBaseContainerNode::IsSolvable( bool bHierachyUp, CUtlVector< CBaseNode* > *hCallerList, CUtlVector< CBaseNode* > *hTested )
{
	if ( !IsAffectingSolver() )
		return false;

	if ( !BaseClass::IsSolvable( bHierachyUp, hCallerList, hTested ) )
		return false;

	CUtlVector< CBaseNode* > hLocalTestList;

	CUtlVector< CBaseNode* > *hTestList = hTested ? hTested : &hLocalTestList;

	CUtlVector< CBaseContainerNode* > hChildrenContainer;
	for ( int i = 0; i < m_hChildren.Count(); i++ )
		if ( m_hChildren[i]->GetAsContainer() != NULL )
			hChildrenContainer.AddToTail( m_hChildren[i]->GetAsContainer() );

	//CBaseNode *pParentCaller = this;
	//if ( pCaller && pCaller->GetAsContainer() != NULL
	//	&& HasContainerParent( pCaller->GetAsContainer() ) )
	//	pParentCaller = pCaller;

	CUtlVector< CBaseNode* > hLocalCallerList;
	const bool bStoresCallerList = hCallerList == NULL;
	if ( bStoresCallerList )
		hCallerList = &hLocalCallerList;

	if ( !hCallerList->HasElement( this ) )
		hCallerList->AddToTail( this );

	for ( int i = 0; i < m_hChildren.Count(); i++ )
	{
		bool bIgnore = false;
		for ( int c = 0; c < hChildrenContainer.Count(); c++ )
		{
			if ( hChildrenContainer[c]->HasChild( m_hChildren[i] ) )
				bIgnore = true;
		}

		//if ( m_hChildren[i]->GetErrorLevel() == ERRORLEVEL_FAIL )
		//	return false;

		if ( !bIgnore &&
			//!m_hChildren[i]->GetAsContainer() &&
			!m_hChildren[i]->IsSolvable( bHierachyUp, /*pParentCaller*/ hCallerList, hTestList ) )
			return false;
	}

	if ( bStoresCallerList )
		hLocalCallerList.Purge();
	
	hChildrenContainer.Purge();
	hLocalTestList.Purge();
	return true;
}
void CBaseContainerNode::UpdateOnMove()
{
	int numNodes = pNodeView->GetNumNodes();
	for ( int i = 0; i < numNodes; i++ )
	{
		CBaseNode *n = pNodeView->GetNode( i );
		if ( n == this )
			continue;
		Vector2D nodePos = n->GetContainerSensitiveCenter();

		bool bIsActive = IsInContainerBounds( nodePos );
		if ( !bIsActive )
			RemoveChild( n );
		else
			AddChild( n );
	}
}
void CBaseContainerNode::RemoveChild( CBaseNode *child )
{
	if ( !IsAffectingSolver() )
		return;

	if ( !HasChild( child ) )
		return;

	child->RemoveFromContainer( this );
	m_hChildren.FindAndRemove( child );
	child->OnUpdateHierachy( NULL, NULL );
}
void CBaseContainerNode::AddChild( CBaseNode *child )
{
	if ( !IsAffectingSolver() )
		return;

	if ( HasChild( child ) )
		return;

	child->InsertToContainer( this );
	m_hChildren.AddToTail( child );
	child->OnUpdateHierachy( NULL, NULL );
}
void CBaseContainerNode::Solve_ContainerEntered()
{
	AddSolver( new CHLSL_Solver_ContainerBasic( GetUniqueIndex() ) );
	AddSolver( new CHLSL_Solver_ContainerBasic( GetUniqueIndex() ) );
}
void CBaseContainerNode::Solve_ContainerCreateVariables()
{
	Assert( 0 );
}
void CBaseContainerNode::Solve_ContainerLeft()
{
	AddSolver( new CHLSL_Solver_ContainerBasic( GetUniqueIndex() ) );
}

bool CBaseContainerNode::MustDragAlone()
{
	return m_iActiveScalingMode != CBORDER_INVALID;
}

void CBaseContainerNode::FullHierachyUpdate()
{
#if 0
	CUtlVector< CBaseNode* > hListTopNodes;
	for ( int i = 0; i < GetNumChildren(); i++ )
	{
		CBaseNode *nChild = GetChild( i );
		nChild->Recursive_AddTailNodes( hListTopNodes, true, false, this );
	}
	for ( int i = 0; i < hListTopNodes.Count(); i++ )
	{
		hListTopNodes[ i ]->OnUpdateHierachy( NULL, NULL );
	}
	hListTopNodes.Purge();
#elif 0
	bool bHierachyError = ERRORLEVEL_NONE;
	int CurrentHierachy = HLSLHIERACHY_NONE;
	for ( int i = 0; i < GetNumChildren(); i++ )
	{
		int NodeHierachy = GetChild( i )->GetHierachyTypeIterateFullyRecursive();
		if ( CurrentHierachy != HLSLHIERACHY_NONE && (NodeHierachy != CurrentHierachy || CurrentHierachy == HLSLHIERACHY_MULTIPLE) )
			bHierachyError = true;
		else
			CurrentHierachy = NodeHierachy;
		if ( bHierachyError )
			break;
	}


	int iErrorLevel = ERRORLEVEL_NONE;
	if ( bHierachyError )
		iErrorLevel = ERRORLEVEL_FAIL;

	for ( int i = 0; i < GetNumChildren(); i++ )
		iErrorLevel = max( iErrorLevel, GetChild( i )->GetErrorLevel() );
	if ( iErrorLevel != ERRORLEVEL_NONE )
	{
		for ( int i = 0; i < GetNumChildren(); i++ )
			GetChild( i )->SetErrorLevel( iErrorLevel );
	}


	for ( int i = 0; i < GetNumChildren(); i++ )
	{
		GetChild( i )->OnUpdateHierachy( NULL, NULL );
	}
#else

	if ( !IsAffectingSolver() )
		return;

	for ( int i = 0; i < GetNumChildren(); i++ )
	{
		GetChild( i )->OnUpdateHierachy( NULL, NULL );
	}
#endif
}
bool CBaseContainerNode::TestFullHierachyUpdate()
{
	if ( !IsAffectingSolver() )
		return false;

	bool bError = false;
	int LastHierachy = HLSLHIERACHY_PENDING;

	for ( int i = 0; i < GetNumChildren(); i++ )
	{
		CBaseNode *n = GetChild( i );
		if ( n->GetAsContainer() )
			continue;

		if ( !n->CanBeInContainer() )
			bError = true;
		int tmpHierachy = n->GetTempHierachyType();
		if ( tmpHierachy == HLSLHIERACHY_PENDING )
		{
			tmpHierachy = n->GetHierachyTypeIterateFullyRecursive();
			n->SetTempHierachyTypeFullyRecursive( tmpHierachy );
		}

		if ( LastHierachy > HLSLHIERACHY_PENDING &&
			LastHierachy != tmpHierachy )
			bError = true;
		LastHierachy = tmpHierachy;
	}

	pNodeView->ResetTempHierachy();
	if ( bLastHierachyErrorTest != bError )
	{
		bLastHierachyErrorTest = bError;
		FullHierachyUpdate();
	}

	if ( UpdateInputsValid() != ERRORLEVEL_NONE )
		bError = true;

	return bError;
}

KeyValues *CBaseContainerNode::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetFloat( "container_extents_x", m_vecContainerExtents.x );
	pKV->SetFloat( "container_extents_y", m_vecContainerExtents.y );
	pKV->SetFloat( "container_extents_z", m_vecContainerExtents.z );

	return pKV;
}
void CBaseContainerNode::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_vecContainerExtents.x = pKV->GetFloat( "container_extents_x", m_vecContainerExtents.x );
	m_vecContainerExtents.y = pKV->GetFloat( "container_extents_y", m_vecContainerExtents.y );
	m_vecContainerExtents.z = pKV->GetFloat( "container_extents_z", m_vecContainerExtents.z );

	UpdateOnMove();
}

bool CBaseContainerNode::IsWithinBounds_Base( const Vector2D &pos )
{
	bool bBaseTest = BaseClass::IsWithinBounds_Base( pos );
	if ( !bBaseTest )
	{
		int BorderTest = IsInBorderBounds( pos );
		bBaseTest = BorderTest != CBORDER_INVALID;
	}
	return bBaseTest;
}
Vector2D CBaseContainerNode::GetContainerBorderMin( int mode )
{
	Vector2D out;
	Vector2D center = GetCenter();
	center.x = GetBoundsMaxNodeSpace().x;
	const float bSize = GetBorderSize();

	switch ( mode )
	{
	default:
		Assert( 0 );
	case CBORDER_TOP_LEFT:
		out.x = center.x;
		out.y = center.y + m_vecContainerExtents.x - bSize;
		break;
	case CBORDER_TOP:
		out.x = center.x + bSize;
		out.y = center.y + m_vecContainerExtents.x - bSize;
		break;
	case CBORDER_TOP_RIGHT:
		out.x = center.x + m_vecContainerExtents.y - bSize;
		out.y = center.y + m_vecContainerExtents.x - bSize;
		break;
	case CBORDER_RIGHT:
		out.x = center.x + m_vecContainerExtents.y - bSize;
		out.y = center.y + m_vecContainerExtents.z + bSize;
		break;
	case CBORDER_BOTTOM_RIGHT:
		out.x = center.x + m_vecContainerExtents.y - bSize;
		out.y = center.y + m_vecContainerExtents.z;
		break;
	case CBORDER_BOTTOM:
		out.x = center.x + bSize;
		out.y = center.y + m_vecContainerExtents.z;
		break;
	case CBORDER_BOTTOM_LEFT:
		out.x = center.x;
		out.y = center.y + m_vecContainerExtents.z;
		break;
	case CBORDER_LEFT:
		out.x = center.x;
		out.y = center.y + m_vecContainerExtents.z + bSize;
		break;
	}

	return out;
}
Vector2D CBaseContainerNode::GetContainerBorderMax( int mode )
{
	Vector2D out;
	Vector2D center = GetCenter();
	center.x = GetBoundsMaxNodeSpace().x;
	const float bSize = GetBorderSize();

	switch ( mode )
	{
	default:
	case CBORDER_TOP_LEFT:
		out.x = center.x + bSize;
		out.y = center.y + m_vecContainerExtents.x;
		break;
	case CBORDER_TOP:
		out.x = center.x + m_vecContainerExtents.y - bSize;
		out.y = center.y + m_vecContainerExtents.x;
		break;
	case CBORDER_TOP_RIGHT:
		out.x = center.x + m_vecContainerExtents.y;
		out.y = center.y + m_vecContainerExtents.x;
		break;
	case CBORDER_RIGHT:
		out.x = center.x + m_vecContainerExtents.y;
		out.y = center.y + m_vecContainerExtents.x - bSize;
		break;
	case CBORDER_BOTTOM_RIGHT:
		out.x = center.x + m_vecContainerExtents.y;
		out.y = center.y + m_vecContainerExtents.z + bSize;
		break;
	case CBORDER_BOTTOM:
		out.x = center.x + m_vecContainerExtents.y - bSize;
		out.y = center.y + m_vecContainerExtents.z + bSize;
		break;
	case CBORDER_BOTTOM_LEFT:
		out.x = center.x + bSize;
		out.y = center.y + m_vecContainerExtents.z + bSize;
		break;
	case CBORDER_LEFT:
		out.x = center.x + bSize;
		out.y = center.y + m_vecContainerExtents.x - bSize;
		break;
	}

	return out;
}


bool CBaseContainerNode::VguiDraw( bool bShadow )
{
#define TITEL_INSET_X NODE_DRAW_TITLE_SPACE
	if ( !ShouldSimpleDrawObject( pNodeView, pNodeView, m_vecBounds ) )
		return false;
	if ( !pNodeView )
		return false;

	Vector2D title_min = GetBoundsTitleMin();
	Vector2D title_max = GetBoundsTitleMax();
	title_max.x -= TITEL_INSET_X;

	Vector2D box_min = GetBoundsBoxMin();
	Vector2D box_max = GetBoundsBoxMax();

	pNodeView->ToPanelSpace( title_min );
	pNodeView->ToPanelSpace( title_max );
	pNodeView->ToPanelSpace( box_min );
	pNodeView->ToPanelSpace( box_max );

	float flZoom = pNodeView->GetZoomScalar();
	float flO = NODE_DRAW_SHADOW_DELTA * flZoom;
	Vector2D offset( flO, flO );

	if ( bShadow )
	{
		title_min += offset;
		title_max += offset;
		box_min += offset;
		box_max += offset;
	}

	const bool bSelected = IsSelected();

	if ( bShadow )
		surface()->DrawSetColor( NODE_DRAW_COLOR_SHADOW );
	else
	{
		Color colTitleNoShadow = NODE_DRAW_COLOR_TITLE;
		if ( GetNumContainers() )
			colTitleNoShadow = NODE_DRAW_COLOR_HASCONTAINER_TITLE;
		else if ( bSelected )
			colTitleNoShadow = NODE_DRAW_COLOR_SELECTED_TITLE;
		surface()->DrawSetColor( colTitleNoShadow );
	}

	surface()->DrawFilledRect( title_min.x, title_min.y, title_max.x, title_max.y );

	if ( !bShadow )
	{
		Color boxNoShadow = NODE_DRAW_COLOR_BOX;
		if ( bSelected )
			boxNoShadow = NODE_DRAW_COLOR_SELECTED_BOX;
		else if ( GetNumSolvers() && !HasDummySolvers() )
			boxNoShadow = NODE_DRAW_COLOR_HASSOLVER_BOX;

		surface()->DrawSetColor( boxNoShadow );
	}

	surface()->DrawFilledRect( box_min.x, box_min.y, box_max.x, box_max.y );

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

	if ( bShadow )
	{
		for ( int i = 0; i < 8; i++ )
			rects_[i] += offset;
	}

	for ( int i = 0; i < 4; i++ )
		surface()->DrawFilledRect( rects_[i*2].x, rects_[i*2+1].y, rects_[i*2+1].x, rects_[i*2].y );


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
		surface()->DrawFilledRect( title_min.x, title_min.y, title_max.x, title_min.y + borderSize );
		surface()->DrawFilledRect( title_min.x, title_max.y - borderSize, title_max.x, title_max.y );
		surface()->DrawFilledRect( title_min.x, title_min.y, title_min.x + borderSize, title_max.y );
		surface()->DrawFilledRect( title_max.x - borderSize, title_min.y, title_max.x, title_max.y );

		surface()->DrawFilledRect( box_min.x, box_min.y, box_max.x, box_min.y + borderSize );
		surface()->DrawFilledRect( box_min.x, box_max.y - borderSize, box_max.x, box_max.y );
		surface()->DrawFilledRect( box_min.x, box_min.y, box_min.x + borderSize, box_max.y );
		//surface()->DrawFilledRect( box_max.x - borderSize, box_min.y, box_max.x, box_max.y );

		surface()->DrawFilledRect( box_max.x, rects_[1].y, box_max.x + borderSize, box_min.y + borderSize );
		surface()->DrawFilledRect( box_max.x, box_max.y - borderSize, box_max.x + borderSize, rects_[4].y );
		surface()->DrawFilledRect( rects_[7].x-borderSize, rects_[7].y, rects_[7].x, rects_[6].y );

		surface()->DrawFilledRect( rects_[2].x, rects_[3].y, rects_[2].x+borderSize, rects_[2].y );
		surface()->DrawFilledRect( rects_[3].x-borderSize, rects_[1].y, rects_[3].x, rects_[4].y );

		surface()->DrawFilledRect( rects_[0].x, rects_[1].y, rects_[1].x, rects_[1].y+borderSize );
		surface()->DrawFilledRect( rects_[4].x, rects_[4].y-borderSize, rects_[5].x, rects_[4].y );

		surface()->DrawFilledRect( rects_[7].x-borderSize, rects_[0].y-borderSize, rects_[2].x+borderSize, rects_[0].y );
		surface()->DrawFilledRect( rects_[7].x-borderSize, rects_[5].y, rects_[2].x+borderSize, rects_[5].y+borderSize );
	}

	Vector2D titleMid = ( title_max - title_min ) * 0.5f + title_min;

	wchar_t szconverted[ 1024 ];
	int fontWide, fontTall;

	if ( !bShadow )
	{
		bool bDraw;
		vgui::HFont hFont_Small = pNodeView->GetFontScaled(12,bDraw);
		if ( bDraw )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( szOpName, szconverted, sizeof(szconverted)  );

			surface()->DrawSetTextFont( hFont_Small );
			surface()->GetTextSize( hFont_Small, szconverted, fontWide, fontTall );
			surface()->DrawSetTextPos( titleMid.x - fontWide * 0.5f, titleMid.y - fontTall * 0.5f );

			surface()->DrawSetTextColor( bSelected ? NODE_DRAW_COLOR_SELECTED_TEXT_OP : NODE_DRAW_COLOR_TEXT_OP );
			surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
		}

		//VguiDraw_Preview();
		DrawGrabIcon();
	}


	if ( Q_strlen( szNodeName ) > 1 )
	{
		surface()->DrawSetTextColor( bShadow ? NODE_DRAW_COLOR_SHADOW : NODE_DRAW_COLOR_CUSTOMTITLE );

		bool bDraw;
		vgui::HFont hFont_Small = pNodeView->GetFontScaled(18,bDraw);
		if ( bDraw )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( szNodeName, szconverted, sizeof(szconverted) );

			surface()->DrawSetTextFont( hFont_Small );
			surface()->GetTextSize( hFont_Small, szconverted, fontWide, fontTall );
			surface()->DrawSetTextPos( title_min.x, title_min.y - fontTall - 3 * pNodeView->GetZoomScalar() );

			surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
		}
	}

	return true;
}
void CBaseContainerNode::DrawGrabIcon()
{
	const float bSize = GetBorderSize();

	surface()->DrawSetColor( Color( 192, 192, 192, 255 ) );
	surface()->DrawSetTexture( m_iGrabIcon );

	Vector2D icooffset( -GRABICO_OFFSET, GRABICO_OFFSET );
	Vector2D gi_max = Vector2D( GetContainerMaxs().x, GetContainerMins().y ) + icooffset;
	Vector2D gi_min = gi_max + Vector2D( -bSize, bSize );

	pNodeView->ToPanelSpace( gi_max );
	pNodeView->ToPanelSpace( gi_min );

	surface()->DrawTexturedRect( gi_min.x, gi_min.y, gi_max.x, gi_max.y );
}