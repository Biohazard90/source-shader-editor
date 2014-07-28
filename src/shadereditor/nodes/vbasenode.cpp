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

#include "editorcommon.h"
#include "vBaseNode.h"

HNODE CBaseNode::m_iUniqueIndexCount = HNODE(0);

CBaseNode::CBaseNode( const char *opName, CNodeView *p )
{
	szOpName[0] = '\0';
	szNodeName[0] = '\0';

	m_vecPosition.Init();
	m_flMinSizeX = 80;
	m_flMinSizeY = 80;
	m_flMinSizeY_VS = 0;
	m_flMinSizePREVIEW_X = PREVIEW_SIZE_NODE;

	m_bIsAllocating = false;
	m_bMarkedForDeletion = false;
	m_iTempHierachy = HLSLHIERACHY_PENDING;
	bSelected = false;
	m_bAllInputsRequired = true;

	m_bPreviewEnabled = true;
	m_bAllowPreview = true;
	m_iUniqueIndex = m_iUniqueIndexCount;
	m_vecBorderInfo.Init();

	SetErrorLevel( ERRORLEVEL_NONE );

	pNodeView = p;
	if ( pNodeView )
		pNodeView->OnHierachyChanged();

	m_iUniqueIndexCount++;

	SetupVguiTex( m_iPreviewMaterial, "shadereditor/_rtdebug_pp0" );
	SetName( opName );
}

CBaseNode::~CBaseNode()
{
	RemoveSolvers( true );
	
	//for ( int i = 0; i < GetNumContainers(); i++ )
	while( GetNumContainers() )
		GetContainer( 0 )->RemoveChild( this );
	Assert( !m_hParentContainers.Count() );
	//m_hParentContainers.Purge();
	//for ( int j = 0; j < m_hInputs.Count(); j++ )
	//	m_hInputs[ j ]->DisconnectBridges();
	//for ( int j = 0; j < m_hOutputs.Count(); j++ )
	//	m_hOutputs[ j ]->DisconnectBridges();

	//SweepJackHlslCache(true);

	CUtlVector< CJack* >m_hTmpJacks;
	m_hTmpJacks.AddVectorToTail( m_hInputs );
	m_hTmpJacks.AddVectorToTail( m_hOutputs );

	m_hInputs.Purge();
	m_hOutputs.Purge();
	SaveDeleteVector( m_hTmpJacks );

	//SaveDeleteVector( m_hInputs );
	//SaveDeleteVector( m_hOutputs );

	if ( pNodeView )
		pNodeView->OnHierachyChanged();
}
void CBaseNode::SetName( const char *opName )
{
	Q_snprintf( szOpName, sizeof( szOpName ), "%s", opName );
	UpdateSize();
}
const char *CBaseNode::GetName()
{
	return szOpName;
}
const char *CBaseNode::GetUserName()
{
	return szNodeName;
}
const int CBaseNode::GetAllowedFlowgraphTypes()
{
	return CNodeView::FLOWGRAPH_HLSL | CNodeView::FLOWGRAPH_HLSL_TEMPLATE;
}
int CBaseNode::GetFinalTextSize()
{
	int i = 0;
	if ( Q_strlen(szOpName) >= 1 )
	{
		float z = 1.5f;
		bool bDraw;
		HFont font = pNodeView->GetFontScaled( 9, bDraw, &z );
		Assert( bDraw );

		wchar_t szconverted[ 1024 ];
		Q_memset( szconverted, 0, sizeof( szconverted ) );
		g_pVGuiLocalize->ConvertANSIToUnicode( szOpName, szconverted, sizeof(szconverted)  );

		int fontWide, fontTall;
		surface()->DrawSetTextFont( font );
		surface()->GetTextSize( font, szconverted, fontWide, fontTall );

		i = fontWide + 3;
	}
	return i;
}

void CBaseNode::Spawn()
{
	UpdateSize();
}

//bool CBaseNode::IsNodeCrucial()
//{
//	int type = GetNodeType();
//
//	if ( type == HLSLNODE_VS_IN ||
//		type == HLSLNODE_VS_OUT ||
//		type == HLSLNODE_PS_IN ||
//		type == HLSLNODE_PS_OUT )
//		return true;
//	return false;
//}

void CBaseNode::AddSolver( CHLSL_SolverBase *solver )
{
#if SHOW_SEDIT_ERRORS
	Assert( !m_hLocalSolvers.IsValidIndex( m_hLocalSolvers.Find(solver) ) );
#endif
	m_hLocalSolvers.AddToTail( solver );
}
void CBaseNode::RemoveSolvers( bool bDelete )
{
	if ( bDelete )
		SaveDeleteVector( m_hLocalSolvers );
	else
		m_hLocalSolvers.Purge();
}
const int CBaseNode::GetNumSolvers()
{
	return m_hLocalSolvers.Count();
}
bool CBaseNode::HasDummySolvers()
{
	for ( int i = 0; i < m_hLocalSolvers.Count(); i++ )
		if ( m_hLocalSolvers[i]->IsDummy() )
			return true;
	return false;
}
const CUtlVector< CHLSL_SolverBase* > *CBaseNode::GetSolvers()
{
	return &m_hLocalSolvers;
}
CHLSL_SolverBase *CBaseNode::GetSolver( int idx )
{
#if SHOW_SEDIT_ERRORS
	Assert( m_hLocalSolvers.IsValidIndex( idx ) );
#endif
	return m_hLocalSolvers[ idx ];
}
bool CBaseNode::InvokeCreateSolvers( GenericShaderData *ShaderData )
{
	Assert( ShaderData || !( GetAllowedFlowgraphTypes() & CNodeView::FLOWGRAPH_HLSL ) );

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
	{
		SmartCreateDummySolver();
		return true;
	}

	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		if ( GetJack_In(i)->GetNumBridges() )
			if ( GetJack_In(i)->GetBridge(0)->GetEndNode(this)->GetErrorLevel() != ERRORLEVEL_NONE )
				return false;
	}

	bool bSuccess = CreateSolvers( ShaderData );
	if ( !bSuccess )
	{
		SetErrorLevel( ERRORLEVEL_UNDEFINED );
	}

	if ( !bSuccess && !GetNumSolvers() )
		SmartCreateDummySolver();

	return bSuccess;
}
bool CBaseNode::CreateSolvers( GenericShaderData *ShaderData )
{
#if SHOW_SEDIT_ERRORS
	Warning( "NODETYPE %i HAS NO SOLVER IMPLEMENTED!!!\n", GetNodeType() );
#endif
	return false;
}
void CBaseNode::SmartCreateDummySolver()
{
	if ( GetNumSolvers() )
		return;
	AddSolver( new CHLSL_Solver_Dummy( GetUniqueIndex() ) );
}
void CBaseNode::SweepJackHlslCache()
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		GetJack_In( i )->ClearTemporaryVarTarget();

	for ( int i = 0; i < GetNumJacks_Out(); i++ )
		GetJack_Out( i )->ClearTemporaryVarTarget();
}

bool CBaseNode::InsertToContainer( CBaseContainerNode *container )
{
	Assert( dynamic_cast<CNodeComment*>( container ) == NULL );

	if ( HasContainerParent( container ) )
		return false;
	m_hParentContainers.AddToTail( container );
	return true;
}
bool CBaseNode::HasContainerParent( CBaseContainerNode *container )
{
	if ( !container )
		return false;
	return m_hParentContainers.HasElement( container );
}
bool CBaseNode::RemoveFromContainer( CBaseContainerNode *container )
{
	int idx = m_hParentContainers.Find( container );
	if ( !m_hParentContainers.IsValidIndex(idx) )
		return false;
	m_hParentContainers.Remove( idx );
	Assert( !HasContainerParent( container ) );
	return true;
}
int CBaseNode::GetNumContainers()
{
	return m_hParentContainers.Count();
}
CBaseContainerNode *CBaseNode::GetContainer( int idx )
{
	Assert( m_hParentContainers.IsValidIndex( idx ) );
	return m_hParentContainers[ idx ];
}
Vector2D CBaseNode::GetContainerSensitiveCenter()
{
	return GetPosition() + Vector2D( 45, - 30 );
}
void CBaseNode::ListContainersChronologically_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed,
												CUtlVector< CBaseContainerNode* > &hList )
{
	CBaseContainerNode *Self = GetAsContainer();
	int numContainerParents = GetNumContainers();


	//if ( !numContainerParents )
	{
		if ( Self && !hList.IsValidIndex( hList.Find( Self ) ) )
			hList.AddToHead( Self );
	}
	//else
	{
		for ( int i = 0; i < numContainerParents; i++ )
		{
			CBaseContainerNode *c = m_hParentContainers[i];
			if ( m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( c ) ) )
				continue;
			m_hNodesProcessed.AddToTail( c );
			c->ListContainersChronologically_Internal( m_hNodesProcessed, hList );
		}
	}
}
int ContainerSort( CBaseContainerNode *const *c1, CBaseContainerNode *const *c2 )
{
	if ( (*c1)->HasContainerParent( (*c2) ) )
		return 1;
	else if ( (*c2)->HasContainerParent( (*c1) ) )
		return -1;
	return 0;
}
void CBaseNode::ListContainersChronologically( CUtlVector< CBaseContainerNode* > &hList )
{
	CUtlVector< CBaseNode* > hNodes;
	ListContainersChronologically_Internal( hNodes, hList );
	hNodes.Purge();

	hList.Sort( ContainerSort );
}
bool CBaseNode::RecursiveTestContainerError_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed,
										bool &bLeftContainerOnce, const bool &bHierachyUp, CBaseContainerNode *container )
{
	bool bError = false;
	const bool bIsInContainer = HasContainerParent( container );
	if ( bLeftContainerOnce && bIsInContainer )
		bError = true;
	//else if ( !bIsInContainer )
	//	bLeftContainerOnce = true;

	const int numJacks = bHierachyUp ? GetNumJacks_In() : GetNumJacks_Out();
	for ( int i = 0; i < numJacks; i++ )
	{
		CJack *j = bHierachyUp ? GetJack_In( i ) : GetJack_Out( i );
		for ( int b = 0; b < j->GetNumBridges(); b++ )
		{
			CBridge *pBridge = j->GetBridge( b );
			CBaseNode *pNodeEnd = pBridge->GetEndNode( this );
			if ( !pNodeEnd )
				continue;

			if ( !bIsInContainer )
				bLeftContainerOnce = true;
			bool bOtherInContainer = pNodeEnd->HasContainerParent( container );
			if ( bLeftContainerOnce && bOtherInContainer )
				bError = true;

			if ( m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( pNodeEnd ) ) )
				continue;

			m_hNodesProcessed.AddToTail( pNodeEnd );
			bError = bError || pNodeEnd->RecursiveTestContainerError_Internal( m_hNodesProcessed,
																			bLeftContainerOnce, bHierachyUp, container );
		}
	}
	bLeftContainerOnce = false;
	return bError;
}
bool CBaseNode::RecursiveTestContainerError( const bool &bHierachyUp, CBaseContainerNode *container )
{
	CUtlVector< CBaseNode* >m_hProcessed;
	bool bLeftContainer = false;
	bool bResult = RecursiveTestContainerError_Internal( m_hProcessed, bLeftContainer, bHierachyUp, container );
	m_hProcessed.Purge();
	return bResult;
}

bool CBaseNode::IsSolvable( bool bHierachyUp, CUtlVector< CBaseNode* > *hCallerList, CUtlVector< CBaseNode* > *hTested )
{
	if (hTested && !(hTested->HasElement(this)))
		hTested->AddToTail( this );

	const int numJacks = bHierachyUp ? GetNumJacks_In() : GetNumJacks_Out();
	for ( int i = 0; i < numJacks; i++ )
	{
		CJack *j = bHierachyUp ? GetJack_In( i ) : GetJack_Out( i );
		for ( int b = 0; b < j->GetNumBridges(); b++ )
		{
			CBridge *pBridge = j->GetBridge( b );
			CBaseNode *pNodeEnd = pBridge->GetEndNode( this );
			//if ( pCaller && ( pNodeEnd == pCaller || pNodeEnd->HasContainerParent( pCaller->GetAsContainer() ) ) )
			if ( hCallerList && hCallerList->HasElement( pNodeEnd ) )
				continue;

			bool bContinue = false;
			for ( int c = 0; hCallerList && c < hCallerList->Count(); c++ )
				if ( pNodeEnd->HasContainerParent( hCallerList->Element( c )->GetAsContainer() ) )
					bContinue = true;
			if ( bContinue )
				continue;

			if ( pNodeEnd->GetAsContainer() )
				continue;
			if ( !pNodeEnd->GetNumSolvers() )
				return false;
		}
	}
	if ( !GetAsContainer() )
	{
		for ( int c = 0; c < GetNumContainers(); c++ )
		{
			CBaseContainerNode *container = GetContainer( c );
			if ( hTested && hTested->HasElement(container) )
				continue;
			//if ( pCaller != container &&
			//	!container->IsSolvable( bHierachyUp, pCaller, hTested ) )
			if ( ( !hCallerList || !hCallerList->HasElement( container ) ) &&
				!container->IsSolvable( bHierachyUp, hCallerList, hTested ) )
				return false;
		}
	}
	return true;
}
void CBaseNode::Recursive_AddTailNodes_Internal(	CUtlVector< CBaseNode* > &m_hProcessedNodes,
													CUtlVector< CBaseNode* > &m_hList,
													bool bHierachyUp, bool bOnlyWithoutSolver,
													CBaseContainerNode *pContainer, bool bAddContainers )
{
	m_hProcessedNodes.AddToTail( this );
	Assert( !pContainer || HasContainerParent( pContainer ) );
	const int numJacks = bHierachyUp ? GetNumJacks_In() : GetNumJacks_Out();
	bool bEndOfContainer = pContainer != NULL;
	const bool bIsContainer = GetAsContainer() != NULL;

	if ( bEndOfContainer )
		for ( int i = 0; i < numJacks; i++ )
		{
			CJack *j = bHierachyUp ? GetJack_In( i ) : GetJack_Out( i );
			for ( int b = 0; b < j->GetNumBridges(); b++ )
			{
				CBridge *pBridge = j->GetBridge( b );
				CBaseNode *pNodeEnd = pBridge->GetEndNode( this );
				if ( pNodeEnd && pNodeEnd->HasContainerParent( pContainer ) )
					bEndOfContainer = false;
			}
		}

	const bool bAnyConnected = bHierachyUp ? ( GetNumJacks_In_Connected() != 0 ) : ( GetNumJacks_Out_Connected() != 0 );
	if ( !numJacks || !bAnyConnected || bEndOfContainer )
	{
		if ( !bOnlyWithoutSolver || !GetNumSolvers() )
		{
			if ( m_hList.IsValidIndex( m_hList.Find( this ) ) )
				return;
			if ( !bIsContainer || bAddContainers )
				m_hList.AddToTail( this );
		}
		return;
	}

	bool bCanBeSolved = !bOnlyWithoutSolver || IsSolvable( bHierachyUp );

	for ( int i = 0; i < numJacks; i++ )
	{
		CJack *j = bHierachyUp ? GetJack_In( i ) : GetJack_Out( i );
		const bool bSelfReady = ( !bOnlyWithoutSolver || !GetNumSolvers() );
		bool bOtherReady = !bAnyConnected && !j->GetNumBridgesConnected();
		if ( bOnlyWithoutSolver && j->GetNumBridges() )
		{
			bool bOtherWithNoSolver = false;
			for ( int b = 0; b < j->GetNumBridges(); b++ )
			{
				CBridge *pBridge = j->GetBridge( b );
				CBaseNode *pNodeEnd = pBridge->GetEndNode( this );

				// another container hack 7.2.2012
				if ( !pNodeEnd->GetNumSolvers() && !pNodeEnd->GetAsContainer() )
					bOtherWithNoSolver = true;
			}
			if ( !bOtherWithNoSolver )
				bOtherReady = true;
		}
		if ( bCanBeSolved && bOtherReady && bSelfReady )
		{
			if ( m_hList.IsValidIndex( m_hList.Find( this ) ) )
				continue;
			if ( !bIsContainer || bAddContainers )
				m_hList.AddToTail( this );
			continue;
		}

		for ( int x = 0; x < j->GetNumBridges(); x++ )
		{
			CBridge *b = j->GetBridge( x );
			CBaseNode *next = b->GetEndNode( j );
			if ( next && !m_hProcessedNodes.IsValidIndex( m_hProcessedNodes.Find( next ) ) &&
				(!pContainer || next->HasContainerParent( pContainer ) ) &&
				(!next->GetAsContainer() || bAddContainers) )
				next->Recursive_AddTailNodes_Internal( m_hProcessedNodes, m_hList, bHierachyUp, bOnlyWithoutSolver, pContainer, bAddContainers );
		}
	}
}
void CBaseNode::Recursive_AddTailNodes( CUtlVector< CBaseNode* > &m_hList, bool bHierachyUp, bool bOnlyWithoutSolver,
	CBaseContainerNode *pContainer, bool bAddContainers )
{
	//if ( GetAsContainer() && !bAddContainers )
	//	return;
	CUtlVector< CBaseNode* >m_hProcessed;

	Recursive_AddTailNodes_Internal( m_hProcessed, m_hList, bHierachyUp, bOnlyWithoutSolver, pContainer, bAddContainers );

	m_hProcessed.Purge();
}

void CBaseNode::MarkForDeletion()
{
	m_bMarkedForDeletion = true;
}
const bool CBaseNode::IsMarkedForDeletion()
{
	return m_bMarkedForDeletion;
}
//void CBaseNode::OnUpdateHierachy_Upwards( CBridge *pBridgeInitiator )
//{
//	//if ( pBridgeInitiator )
//	{
//		for ( int i = 0; i < GetNumJacks_In(); i++ )
//		{
//			CJack *j = GetJack_In( i );
//			for ( int a = 0; a < j->GetNumBridges(); a++ )
//			{
//				CBridge *b = j->GetBridge( a );
//				CBaseNode *next = b->GetInputNode();
//				CJack *nextJ = b->GetInputJack();
//				if ( next && nextJ )
//					next->OnUpdateHierachy_Upwards( b );
//			}
//		}
//	}
//
//	UpdatePreviewAllowed();
//}
void CBaseNode::OnUpdateHierachy_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed )
//void CBaseNode::OnUpdateHierachy_Internal(		CUtlVector< CBaseNode* > &m_hNodesProcessed,
//									CUtlVector< CBridge* > &m_hBridgesToUpdate,
//									CUtlVector< CJack* > &m_hJacksToUpdate )
//void CBaseNode::OnUpdateHierachy_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed, CBridge *pBridgeInitiator, CJack *pCaller )
{
	//for ( int __i = 0; __i < m_hJacksToUpdate.Count(); __i++ )
	/*
	if ( !pCaller )
	{
		UpdatePreviewAllowed();
		return;
	}
	*/
	int oldLevel = GetErrorLevel();

	if ( !m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( this ) ) )
		m_hNodesProcessed.AddToTail( this );

	bool bContainerLinkError = false;
	
	int inputErrorLevel = ERRORLEVEL_NONE;
	if ( m_bAllInputsRequired && !JacksAllConnected_In() )
		inputErrorLevel = ERRORLEVEL_UNDEFINED;
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJ = GetJack_In( i );
		for ( int a = 0; a < pJ->GetNumBridges(); a++ )
		{
			CBridge *pB = pJ->GetBridge( a );
			CBaseNode *pN = pB->GetInputNode();
			if ( pB->GetInputJack() )
				pJ->SetSmartType( pB->GetInputJack()->GetSmartType() );
			if ( pN && pN->GetAsContainer() && !HasContainerParent( pN->GetAsContainer() ) )
				bContainerLinkError = true;
		}
	}

	for ( int i = 0; i < GetNumContainers(); i++ )
	{
		if ( RecursiveTestContainerError( false, GetContainer(i) ) )
			inputErrorLevel = max( inputErrorLevel, ERRORLEVEL_FAIL );

		CBaseContainerNode *ctr_A = GetContainer( i );
		if ( ctr_A->TestFullHierachyUpdate() )
			inputErrorLevel = max( inputErrorLevel, ERRORLEVEL_FAIL );

		for ( int a = 0; a < GetNumContainers(); a++ )
		{
			if ( a == i )
				continue;
			CBaseContainerNode *ctr_B = GetContainer( a );
			if ( !ctr_A->HasContainerParent( ctr_B ) &&
				!ctr_B->HasContainerParent( ctr_A ) )
				inputErrorLevel = max( inputErrorLevel, ERRORLEVEL_FAIL );
		}

		int numContainerContainers = ctr_A->GetNumContainers();
		for ( int a = 0; a < numContainerContainers; a++ )
		{
			CBaseContainerNode *ctr_A_A = ctr_A->GetContainer( a );
			if ( !HasContainerParent( ctr_A_A ) )
				inputErrorLevel = max( inputErrorLevel, ERRORLEVEL_FAIL );
		}
	}

	if ( !CanBeInContainer() && GetNumContainers() )
		inputErrorLevel = max( inputErrorLevel, ERRORLEVEL_FAIL );

	inputErrorLevel = max( inputErrorLevel, PerNodeErrorLevel() );

	inputErrorLevel = max( inputErrorLevel, UpdateInputsValid() );

	//if ( inputErrorLevel != ERRORLEVEL_UNDEFINED || GetErrorLevel() == ERRORLEVEL_FAIL )
	{
		for ( int i = 0; i < GetNumJacks_In(); i++ )
		{
			CJack *j = GetJack_In( i );
			for ( int a = 0; a < j->GetNumBridges(); a++ )
			{
				CBridge *b = j->GetBridge(a);
				UpdateBridgeValidity( b, b->GetDestinationJack(), inputErrorLevel );
			}
		}
	}

	if ( bContainerLinkError )
		inputErrorLevel = max( inputErrorLevel, ERRORLEVEL_FAIL );

	SetErrorLevel( inputErrorLevel );
	bool bDeepUpdate = GetErrorLevel() != oldLevel;

	UpdateOutputs();

	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *j = GetJack_Out( i );
		for ( int a = 0; a < j->GetNumBridges(); a++ )
		{
			CBridge *b = j->GetBridge( a );
			CBaseNode *next = b->GetDestinationNode();
			CJack *nextJ = b->GetDestinationJack();

			bool bTargetWithError = next && next->GetErrorLevel() != ERRORLEVEL_NONE;
				
			if ( next && nextJ &&
				(!m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( next ) ) || (bTargetWithError&&bDeepUpdate)) &&
				!next->IsMarkedForDeletion() )
				next->OnUpdateHierachy_Internal( m_hNodesProcessed );
		}
	}
}
void CBaseNode::OnUpdateHierachy( CBridge *pBridgeInitiator, CJack *pCaller )
{
	if ( !pCaller && pBridgeInitiator )
	{
		UpdatePreviewAllowed();
		pNodeView->OnHierachyChanged();
		return;
	}

	CUtlVector< CBaseNode* > m_hNodesProcessed;

	OnUpdateHierachy_Internal( m_hNodesProcessed );

	m_hNodesProcessed.Purge();

	pNodeView->OnHierachyChanged();
}
void CBaseNode::UpdatePreviewAllowed()
{
	int hierachyType = GetHierachyTypeIterateFullyRecursive();
	CheckHierachyTypeFullyRecursive( hierachyType );
}
void CBaseNode::CheckHierachyTypeFullyRecursive( int t )
{
	CUtlVector< CBaseNode* > m_hNodesProcessed;
	m_hNodesProcessed.AddToTail( this );
	CheckHierachyTypeFullyRecursive_Internal( t, m_hNodesProcessed );
	m_hNodesProcessed.Purge();
}
void CBaseNode::CheckHierachyTypeFullyRecursive_Internal( int t, CUtlVector< CBaseNode* > &m_hNodesProcessed )
{
	m_bAllowPreview = t != HLSLHIERACHY_VS;
	UpdateSize();

	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJ = GetJack_In( i );
		for ( int b = 0; b < pJ->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJ->GetBridge( b );
			CBaseNode *pOther = pBridge->GetEndNode( this );
			if ( !pOther || m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( pOther ) ) )
				continue;
			m_hNodesProcessed.AddToTail( this );
			pOther->CheckHierachyTypeFullyRecursive_Internal( t, m_hNodesProcessed );
		}
	}
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *pJ = GetJack_Out( i );
		for ( int b = 0; b < pJ->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJ->GetBridge( b );
			CBaseNode *pOther = pBridge->GetEndNode( this );
			if ( !pOther || m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( pOther ) ) )
				continue;
			m_hNodesProcessed.AddToTail( this );
			pOther->CheckHierachyTypeFullyRecursive_Internal( t, m_hNodesProcessed );
		}
	}
}

int CBaseNode::UpdateInputsValid()
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJIn = GetJack_In( i );
		for ( int a = 0; a < pJIn->GetNumBridges(); a++ )
		{
			CBridge *b = pJIn->GetBridge(a);
			CJack *pEnd = b ? b->GetEndJack(this) : NULL;
			CBaseNode *pOtherNode = b->GetEndNode( this );
			if ( !pEnd || !pOtherNode )
				continue;

			int parentError = pOtherNode->GetErrorLevel();

			bool bDefinedSmarttype_Local = pJIn->GetSmartType() >= 0;
			bool bDefinedSmarttype_Remote = pEnd->GetSmartType() >= 0;
			if ( bDefinedSmarttype_Local &&
				bDefinedSmarttype_Remote &&
				pJIn->GetSmartType() != pEnd->GetSmartType() )
			{
				return ERRORLEVEL_FAIL;
			}
			else if ( parentError == ERRORLEVEL_FAIL )
				return ERRORLEVEL_UNDEFINED;
			else if ( !bDefinedSmarttype_Local || !bDefinedSmarttype_Remote ||
				parentError == ERRORLEVEL_UNDEFINED )
				return ERRORLEVEL_UNDEFINED;
		}
	}

	if ( m_bAllInputsRequired && !JacksAllConnected_In() )
		return ERRORLEVEL_UNDEFINED;
	return ERRORLEVEL_NONE;
}
void CBaseNode::UpdateBridgeValidity( CBridge *pBridge, CJack *pCaller, int inputErrorLevel )
{
	CBaseNode *pNodeViewNode = pBridge->GetInputNode();
	CJack *pOther = pBridge->GetInputJack();

	int iOther_SmartType = pOther ? pOther->GetSmartType() : -1;
	int iLocal_SmartType = pCaller->GetSmartType();

	bool bDefined_Other = iOther_SmartType > -1;
	bool bDefined_Local = iLocal_SmartType > -1;
	//bool bLocked_Other = pOther->IsSmartTypeLocked();
	bool bLocked_Local = pCaller->IsSmartTypeLocked();

	int iParentErrorLevel = pNodeViewNode ? pNodeViewNode->GetErrorLevel() : ERRORLEVEL_UNDEFINED;
	int iGoalErrorLevel = inputErrorLevel;

	bool bCorrectType = iOther_SmartType == iLocal_SmartType;

	// something isn't defined at all
	if ( !bDefined_Local || !bDefined_Other || iParentErrorLevel != ERRORLEVEL_NONE )
			iGoalErrorLevel = ERRORLEVEL_UNDEFINED;
	// wrong type, fail for sure
	else if ( !bCorrectType )
		iGoalErrorLevel = ERRORLEVEL_FAIL;
	else if ( pNodeViewNode && pNodeViewNode->GetAsContainer() && !HasContainerParent( pNodeViewNode->GetAsContainer() ) )
		iGoalErrorLevel = ERRORLEVEL_FAIL;
	else if ( inputErrorLevel == ERRORLEVEL_UNDEFINED )
	{
		// Our parent is ready but we failed locally, nothing wrong with the bridge after all
		if ( iParentErrorLevel == ERRORLEVEL_NONE )
			iGoalErrorLevel = ERRORLEVEL_NONE;
	}
	else if ( inputErrorLevel == ERRORLEVEL_FAIL )
	{
		// we failed although our parent is ready and the target input is static and correct
		if ( iParentErrorLevel == ERRORLEVEL_NONE && bLocked_Local )
			iGoalErrorLevel = ERRORLEVEL_NONE;
	}

	pBridge->SetErrorLevel( iGoalErrorLevel );
}
void CBaseNode::UpdateOutputs()
{
}
void CBaseNode::SetOutputsUndefined()
{
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *pJack = GetJack_Out( i );
		pJack->SetSmartType( -1 );
	}
}
void CBaseNode::OnShowSolverErrored()
{
	CUtlVector< CBaseNode* >hList;
	for ( int j = 0; j < GetNumJacks_Out(); j++ )
	{
		CJack *pJ_Out = GetJack_Out(j);
		for ( int i = 0; pJ_Out && i < pJ_Out->GetNumBridges(); i++ )
		{
			CBridge *pB = pJ_Out->GetBridge(i);
			CBaseNode *pN = pB->GetEndNode( this );
			if ( !pN || hList.HasElement( pN ) )
				continue;
			pN->OnUpdateHierachy( NULL, NULL );
			hList.AddToTail( pN );
		}
	}
	hList.Purge();
}
const int CBaseNode::GetErrorLevel()
{
	if ( ShouldErrorOnUndefined() && iErrorLevel == ERRORLEVEL_UNDEFINED )
		return ERRORLEVEL_FAIL;
	return iErrorLevel;
}
void CBaseNode::SetErrorLevel( const int e )
{
	iErrorLevel = e;
}

int CBaseNode::GetHierachyType()
{
	return HLSLHIERACHY_NONE;
}
int CBaseNode::GetAllowedHierachiesAsFlags()
{
	return HLSLHIERACHY_VS | HLSLHIERACHY_PS; // | HLSLHIERACHY_TEMPLATE_VS | HLSLHIERACHY_TEMPLATE_PS;
}
int CBaseNode::GetHierachyTypeIterateFullyRecursive_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed, int *iAllowances )
{
	int tmp = GetHierachyType();
	if ( tmp > HLSLHIERACHY_NONE )
		return tmp;

	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJ = GetJack_In( i );
		for ( int b = 0; b < pJ->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJ->GetBridge( b );
			CBaseNode *pOther = pBridge->GetInputNode();
			if ( !pOther || m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( pOther ) ) )
				continue;

			m_hNodesProcessed.AddToTail( pOther );
			//m_hNodesProcessed.AddToTail( this );
			if ( iAllowances )
				*iAllowances &= pOther->GetAllowedHierachiesAsFlags();

			int otherHType = pOther->GetHierachyTypeIterateFullyRecursive_Internal( m_hNodesProcessed, iAllowances );

			if ( otherHType == HLSLHIERACHY_NONE )
				continue;
			if ( tmp > HLSLHIERACHY_NONE && tmp == otherHType )
				continue;
			if ( tmp > HLSLHIERACHY_NONE && tmp != otherHType )
				return HLSLHIERACHY_MULTIPLE;
			tmp = otherHType;
		}
	}
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *pJ = GetJack_Out( i );
		for ( int b = 0; b < pJ->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJ->GetBridge( b );
			CBaseNode *pOther = pBridge->GetDestinationNode();
			if ( !pOther || m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( pOther ) ) )
				continue;

			m_hNodesProcessed.AddToTail( pOther );
			//m_hNodesProcessed.AddToTail( this );
			if ( iAllowances )
				*iAllowances &= pOther->GetAllowedHierachiesAsFlags();

			int otherHType = pOther->GetHierachyTypeIterateFullyRecursive_Internal( m_hNodesProcessed, iAllowances );

			if ( otherHType == HLSLHIERACHY_NONE )
				continue;
			if ( tmp > HLSLHIERACHY_NONE && tmp == otherHType )
				continue;
			if ( tmp > HLSLHIERACHY_NONE && tmp != otherHType )
				return HLSLHIERACHY_MULTIPLE;
			tmp = otherHType;
		}
	}

	return tmp;
}
int CBaseNode::GetHierachyTypeIterateFullyRecursive( int *iAllowances )
{
	if ( iAllowances )
		*iAllowances = GetAllowedHierachiesAsFlags();
	if ( GetHierachyType() > HLSLHIERACHY_NONE )
		return GetHierachyType();

	CUtlVector< CBaseNode* > m_hIgnore;
	m_hIgnore.AddToTail( this );

	//CFastTimer timer_;
	//timer_.Start();

	int hSearch = GetHierachyTypeIterateFullyRecursive_Internal( m_hIgnore, iAllowances );

	//timer_.End();
	//Msg( "recursive search took %f seconds\n", timer_.GetDuration() );

	m_hIgnore.Purge();
	return hSearch;
}


void CBaseNode::SetTempHierachyTypeFullyRecursive_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed, int t )
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJ = GetJack_In( i );
		for ( int b = 0; b < pJ->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJ->GetBridge( b );
			CBaseNode *pOther = pBridge->GetInputNode();
			if ( !pOther || m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( pOther ) ) )
				continue;

			m_hNodesProcessed.AddToTail( pOther );
			pOther->SetTempHierachyType( t );
			pOther->SetTempHierachyTypeFullyRecursive_Internal( m_hNodesProcessed, t );
		}
	}
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *pJ = GetJack_Out( i );
		for ( int b = 0; b < pJ->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJ->GetBridge( b );
			CBaseNode *pOther = pBridge->GetDestinationNode();
			if ( !pOther || m_hNodesProcessed.IsValidIndex( m_hNodesProcessed.Find( pOther ) ) )
				continue;

			m_hNodesProcessed.AddToTail( pOther );
			pOther->SetTempHierachyType( t );
			pOther->SetTempHierachyTypeFullyRecursive_Internal( m_hNodesProcessed, t );
		}
	}
}
const int CBaseNode::GetTempHierachyType()
{
	return m_iTempHierachy;
}
void CBaseNode::SetTempHierachyTypeFullyRecursive( const int t )
{
	CUtlVector< CBaseNode* > m_hIgnore;
	m_hIgnore.AddToTail( this );

	SetTempHierachyType( t );
	SetTempHierachyTypeFullyRecursive_Internal( m_hIgnore, t );

	m_hIgnore.Purge();
}
void CBaseNode::SetTempHierachyType( const int t )
{
	m_iTempHierachy = t;
}

/*
int CBaseNode::GetHierachyTypeIterate( bool bUp, int *iAllowances )
{
	int tmp = GetHierachyType();
	if ( tmp > HLSLHIERACHY_NONE )
		return tmp;

	int numJacks = bUp ? GetNumJacks_In() : GetNumJacks_Out();

	for ( int i = 0; i < numJacks; i++ )
	{
		CJack *pJack = bUp ? GetJack_In( i ) : GetJack_Out( i );
		for ( int b = 0; b < pJack->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJack->GetBridge( b );
			CBaseNode *pOther = bUp ? pBridge->GetInputNode() : pBridge->GetDestinationNode();
			if ( !pOther )
				continue;

			if ( iAllowances )
				*iAllowances &= pOther->GetAllowedHierachiesAsFlags();

			int otherHType = pOther->GetHierachyTypeIterate( bUp, iAllowances );
			if ( otherHType == HLSLHIERACHY_NONE )
				continue;

			if ( tmp > HLSLHIERACHY_NONE && tmp == otherHType )
				continue;

			if ( tmp > HLSLHIERACHY_NONE && tmp != otherHType )
				return HLSLHIERACHY_MULTIPLE;

			tmp = otherHType;
		}
	}

	return tmp;
}
*/
/*
int CBaseNode::GetHierachyTypeIterate( int *iAllowances )
{
	//return GetHierachyTypeIterateFullyRecursive( iAllowances );

	if ( iAllowances )
		*iAllowances = GetAllowedHierachiesAsFlags();
	if ( GetHierachyType() > HLSLHIERACHY_NONE )
		return GetHierachyType();

	int hUp = GetHierachyTypeIterate( true, iAllowances );
	int hDown = GetHierachyTypeIterate( false, iAllowances );

	if ( hUp == HLSLHIERACHY_MULTIPLE ||
		hDown == HLSLHIERACHY_MULTIPLE )
		return HLSLHIERACHY_MULTIPLE;

	if ( hUp == HLSLHIERACHY_NONE &&
		hDown == HLSLHIERACHY_NONE )
		return HLSLHIERACHY_NONE;

	if ( hUp != HLSLHIERACHY_NONE &&
		hDown != HLSLHIERACHY_NONE &&
		hUp != hDown )
		return HLSLHIERACHY_MULTIPLE;

	return max( hUp, hDown );
}
*/
bool CBaseNode::IsPreviewVisible()
{
	return m_bAllowPreview && m_bPreviewEnabled;
}

void CBaseNode::UpdateSize()
{
	int maxJacks = max( m_hInputs.Count(), m_hOutputs.Count() );
	float sizeMin = ( JACK_SIZE_Y + JACK_DELTA_Y ) * maxJacks + JACK_SIZE_Y;
	sizeMin = max( sizeMin, m_flMinSizeY_VS );

	//if ( GetHierachyType() == HLSLHIERACHY_PS )
	if ( IsPreviewVisible() )
		sizeMin = max( m_flMinSizeY, sizeMin );

	m_vecBorderInfo.Init();

	for ( int i = 0; i < m_hInputs.Count(); i++ )
		m_vecBorderInfo.x = max( m_vecBorderInfo.x, GetJack_In(i)->GetFinalTextInset() );
	for ( int i = 0; i < m_hOutputs.Count(); i++ )
		m_vecBorderInfo.y = max( m_vecBorderInfo.y, GetJack_Out(i)->GetFinalTextInset() );

	float addSize = IsPreviewVisible() ? m_flMinSizePREVIEW_X : 0;

	float localminX = max( GetFinalTextSize(), m_flMinSizeX );
	m_vecSize.Init( max( localminX, addSize + m_vecBorderInfo.x + m_vecBorderInfo.y ), -sizeMin );

	TouchJacks();

	UpdateSimpleObjectBounds( m_vecPosition, m_vecSize, m_vecBounds );
}
void CBaseNode::GenerateJacks_Input( int num )
{
	if ( num )
		SetErrorLevel( ERRORLEVEL_UNDEFINED );
	else
		SetErrorLevel( ERRORLEVEL_NONE );

	SaveDeleteVector( m_hInputs );

	for ( int i = 0; i < num; i++ )
	{
		CJack *p = new CJack( this, i, true );
		m_hInputs.AddToTail( p );
	}

	UpdateSize();
}
void CBaseNode::GenerateJacks_Output( int num )
{
	SaveDeleteVector( m_hOutputs );

	for ( int i = 0; i < num; i++ )
	{
		CJack *p = new CJack( this, i, false );
		m_hOutputs.AddToTail( p );
	}

	UpdateSize();
}
void CBaseNode::TouchJacks()
{
	for ( int i = 0; i < m_hInputs.Count(); i++ )
		m_hInputs[i]->UpdatePosition();
	for ( int i = 0; i < m_hOutputs.Count(); i++ )
		m_hOutputs[i]->UpdatePosition();
}
int CBaseNode::GetNumJacks_Out()
{
	return m_hOutputs.Count();
}
int CBaseNode::GetNumJacks_In()
{
	return m_hInputs.Count();
}
int CBaseNode::GetNumJacks_Out_Connected()
{
	int o = 0;
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		if ( GetJack_Out( i )->GetNumBridgesConnected() )
			o++;
	}
	return o;
}
int CBaseNode::GetNumJacks_In_Connected()
{
	int o = 0;
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		if ( GetJack_In( i )->GetNumBridgesConnected() )
			o++;
	}
	return o;
}
CJack* CBaseNode::GetJack_Out( int i )
{
	return m_hOutputs[ i ];
}
CJack* CBaseNode::GetJack_In( int i )
{
	return m_hInputs[ i ];
}
CJack *CBaseNode::GetJackByName_Out( const char *name )
{
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
		if ( !Q_stricmp( GetJack_Out( i )->GetName(), name ) )
			return GetJack_Out( i );
	return NULL;
}
CJack *CBaseNode::GetJackByName_In( const char *name )
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		if ( !Q_stricmp( GetJack_In( i )->GetName(), name ) )
			return GetJack_In( i );
	return NULL;
}
CJack *CBaseNode::GetJackByResType_Out( int restype )
{
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
		if ( restype == GetJack_Out(i)->GetResourceType() )
			return GetJack_Out( i );
	return NULL;
}
CJack *CBaseNode::GetJackByResType_In( int restype )
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		if ( restype == GetJack_In(i)->GetResourceType() )
			return GetJack_In( i );
	return NULL;
}
bool CBaseNode::JacksAllConnected_Out()
{
	return GetNumJacks_Out_Connected() == GetNumJacks_Out();
}
bool CBaseNode::JacksAllConnected_In()
{
	return GetNumJacks_In_Connected() == GetNumJacks_In();
}
void CBaseNode::JackHierachyUpdate_Out()
{
	for ( int j = 0; j < GetNumJacks_Out(); j++ )
	{
		CJack *pJack = GetJack_Out( j );
		for ( int b = 0; b < pJack->GetNumBridges(); b++ )
		{
			CBridge *pBridge = pJack->GetBridge( b );
			CJack *pJackNeedsUpdate = pBridge->GetEndJack( this );
			CBaseNode *pNodeNeedsUpdate = pBridge->GetEndNode( this );
			if ( !pJackNeedsUpdate || !pNodeNeedsUpdate )
				continue;
			pJack->UpdateSmartType( pBridge );
			pNodeNeedsUpdate->OnUpdateHierachy( pBridge, pJackNeedsUpdate );
		}
	}
}
CHLSL_Var *CBaseNode::GetInputToWriteTo( int varTypes )
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJIn = GetJack_In( i );
		CHLSL_Var *potentialVar = pJIn->GetTemporaryVarTarget_End_Smart( varTypes );
		if ( !potentialVar )
			continue;
		if ( !potentialVar->CanBeOverwritten() )
			continue;
		if ( !( potentialVar->GetType() & varTypes ) )
			continue;
		return potentialVar;
	}
	return NULL;
}

void CBaseNode::SetJackFlags_MinFloatRequirement( CJack *j, int components )
{
	j->ClearVarFlags();
	switch ( components )
	{
	case 1:
		j->AddVarFlags( HLSLVAR_FLOAT1 );
	case 2:
		j->AddVarFlags( HLSLVAR_FLOAT2 );
	case 3:
		j->AddVarFlags( HLSLVAR_FLOAT3 );
	case 4:
		j->AddVarFlags( HLSLVAR_FLOAT4 );
	}
}
void CBaseNode::SetJackFlags( CJack *j, HLSLJackVarCluster mode )
{
	j->ClearVarFlags();

	switch (mode)
	{
	case HLSLJACKFLAGS_F1:
		j->AddVarFlags( HLSLVAR_FLOAT1 );
		break;
	case HLSLJACKFLAGS_F2:
	case HLSLJACKFLAGS_UV:
		j->AddVarFlags( HLSLVAR_FLOAT2 );
		break;
	case HLSLJACKFLAGS_F3:
		j->AddVarFlags( HLSLVAR_FLOAT3 );
		break;
	case HLSLJACKFLAGS_F4:
		j->AddVarFlags( HLSLVAR_FLOAT4 );
		break;
	case HLSLJACKFLAGS_F1_TO_F4:
		j->AddVarFlags( HLSLVAR_FLOAT1 | HLSLVAR_FLOAT2 | HLSLVAR_FLOAT3 | HLSLVAR_FLOAT4 );
		break;
	case HLSLJACKFLAGS_VECTOR:
		j->AddVarFlags( HLSLVAR_FLOAT3 | HLSLVAR_FLOAT4 );
		break;
	case HLSLJACKFLAGS_MATRIX:
		j->AddVarFlags( HLSLVAR_MATRIX3X3 | HLSLVAR_MATRIX4X3 | HLSLVAR_MATRIX4X4 );
		break;
	case HLSLJACKFLAGS_ALL:
		j->AddVarFlags( HLSLVAR_FLOAT1 | HLSLVAR_FLOAT2 | HLSLVAR_FLOAT3 | HLSLVAR_FLOAT4 );
		j->AddVarFlags( HLSLVAR_MATRIX3X3 | HLSLVAR_MATRIX4X3 | HLSLVAR_MATRIX4X4 );
		break;
	}
}
void CBaseNode::SetJackFlags_Input( int idx, HLSLJackVarCluster mode )
{
	SetJackFlags( GetJack_In( idx ), mode );
}
void CBaseNode::SetJackFlags_Output( int idx, HLSLJackVarCluster mode )
{
	SetJackFlags( GetJack_Out( idx ), mode );
}
void CBaseNode::SetJackFlags_Input_Flags( int idx, int Flags )
{
	CJack *j = GetJack_In( idx );
	j->ClearVarFlags();
	j->AddVarFlags(Flags);
}
void CBaseNode::SetJackFlags_Output_Flags( int idx, int Flags )
{
	CJack *j = GetJack_Out( idx );
	j->ClearVarFlags();
	j->AddVarFlags(Flags);
}
void CBaseNode::SetupJackOutput( int idx, HLSLJackVarCluster mode, const char *name )
{
	GetJack_Out( idx )->SetName( name );
	SetJackFlags( GetJack_Out( idx ), mode );
}
void CBaseNode::SetupJackInput( int idx, HLSLJackVarCluster mode, const char *name )
{
	GetJack_In( idx )->SetName( name );
	SetJackFlags( GetJack_In( idx ), mode );
}
void CBaseNode::LockJackOutput_Flags( int idx, int Flag, const char *name )
{
	CJack *j = GetJack_Out( idx );
	if ( name )
		j->SetName( name );
	j->ClearVarFlags();
	j->AddVarFlags(Flag);

	j->SetSmartTypeLocked( false );
	j->SetSmartType( Flag );
	j->SetSmartTypeLocked( true );
}
void CBaseNode::LockJackInput_Flags( int idx, int Flag, const char *name )
{
	CJack *j = GetJack_In( idx );
	if ( name )
		j->SetName( name );
	j->ClearVarFlags();
	j->AddVarFlags(Flag);

	j->SetSmartTypeLocked( false );
	j->SetSmartType( Flag );
	j->SetSmartTypeLocked( true );
}
int CBaseNode::TestJackFlags_In()
{
	bool bError = false;
	bool bUndefined = false;
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *j = GetJack_In( i );
		int type = j->GetSmartType();
		if ( type < 0 )
			bUndefined = true;
		else if ( !j->HasVarFlag( type ) )
			bError = true;
	}
	if ( bError )
		return ERRORLEVEL_FAIL;
	if ( bUndefined )
		return ERRORLEVEL_UNDEFINED;
	return ERRORLEVEL_NONE;
}
void CBaseNode::PurgeBridges( bool bInputs, bool bOutputs )
{
	if ( bInputs )
		for ( int i = 0; i < GetNumJacks_In(); i++ )
			GetJack_In( i )->PurgeBridges();
	if ( bOutputs )
		for ( int i = 0; i < GetNumJacks_Out(); i++ )
			GetJack_Out( i )->PurgeBridges();
}
void CBaseNode::CreateBridgeRestoreData_Out( BridgeRestoreMode mode, CUtlVector< BridgeRestoreInfo* > &m_hList )
{
	int iJacksInUse = 0;
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *cur = GetJack_Out(i);
		for ( int b = 0; b < cur->GetNumBridges(); b++ )
		{
			CBridge *pB = cur->GetBridge(b);

			if ( pB->GetEndJack( cur ) )
			{
				BridgeRestoreInfo *restore = new BridgeRestoreInfo();
				restore->iMode = mode;
				restore->pJackOther = pB->GetEndJack( cur );

				if ( mode == BY_NAME )
					Q_snprintf( restore->jackSelfName, sizeof(restore->jackSelfName), "%s", cur->GetName() );
				else if ( mode == BY_RESTYPE )
					restore->resType = cur->GetResourceType();
				else if ( mode == BY_COUNT )
					restore->idx = iJacksInUse;
				else
					restore->idx = i;

				m_hList.AddToTail( restore );
			}
		}

		if ( cur->GetNumBridges() > 0 ||
			( GetNumJacks_In() > i && GetJack_In( i )->GetNumBridges() > 0 ) )
		{
			iJacksInUse++;
		}
	}
}
void CBaseNode::CreateBridgeRestoreData_In( BridgeRestoreMode mode, CUtlVector< BridgeRestoreInfo* > &m_hList )
{
	int iJacksInUse = 0;
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *cur = GetJack_In(i);
		for ( int b = 0; b < cur->GetNumBridges(); b++ )
		{
			CBridge *pB = cur->GetBridge(b);

			if ( pB->GetEndJack( cur ) )
			{
				BridgeRestoreInfo *restore = new BridgeRestoreInfo();
				restore->iMode = mode;
				restore->pJackOther = pB->GetEndJack( cur );

				if ( mode == BY_NAME )
					Q_snprintf( restore->jackSelfName, sizeof(restore->jackSelfName), "%s", cur->GetName() );
				else if ( mode == BY_RESTYPE )
					restore->resType = cur->GetResourceType();
				else if ( mode == BY_COUNT )
					restore->idx = iJacksInUse;
				else
					restore->idx = i;

				m_hList.AddToTail( restore );
			}
		}

		if ( cur->GetNumBridges() > 0 ||
			( GetNumJacks_Out() > i && GetJack_Out( i )->GetNumBridges() > 0 ) )
		{
			iJacksInUse++;
		}
	}
}
void CBaseNode::RestoreBridgesFromList_Out( CUtlVector< BridgeRestoreInfo* > &m_hList )
{
	for ( int i = 0; i < m_hList.Count(); i++ )
	{
		BridgeRestoreInfo *inf = m_hList[i];
		CJack *localJack = NULL;
		switch ( inf->iMode )
		{
		default:
				Assert(0);
			break;
		case BY_INDEX:
				localJack = inf->idx < GetNumJacks_Out() ? GetJack_Out( inf->idx ) : NULL;
			break;
		case BY_NAME:
				localJack = GetJackByName_Out( inf->jackSelfName );
			break;
		case BY_RESTYPE:
				localJack = GetJackByResType_Out( inf->resType );
			break;
		case BY_COUNT:
				if ( inf->idx < GetNumJacks_Out() )
					localJack = GetJack_Out( inf->idx );
			break;
		}

		if ( !localJack )
			continue;
		m_hList[i]->pJackOther->BridgeEndBuild( localJack->BridgeBeginBuild() );
	}
	SaveDeleteVector( m_hList );
}
void CBaseNode::RestoreBridgesFromList_In( CUtlVector< BridgeRestoreInfo* > &m_hList )
{
	for ( int i = 0; i < m_hList.Count(); i++ )
	{
		BridgeRestoreInfo *inf = m_hList[i];
		CJack *localJack = NULL;
		switch ( inf->iMode )
		{
		default:
				Assert(0);
			break;
		case BY_INDEX:
				localJack = inf->idx < GetNumJacks_In() ? GetJack_In( inf->idx ) : NULL;
			break;
		case BY_NAME:
				localJack = GetJackByName_In( inf->jackSelfName );
			break;
		case BY_RESTYPE:
				localJack = GetJackByResType_In( inf->resType );
			break;
		case BY_COUNT:
				if ( inf->idx < GetNumJacks_In() )
					localJack = GetJack_In( inf->idx );
			break;
		}

		if ( !localJack )
			continue;
		m_hList[i]->pJackOther->BridgeEndBuild( localJack->BridgeBeginBuild() );
	}
	SaveDeleteVector( m_hList );
}

bool CBaseNode::RecursiveFindNode_Internal( CUtlVector< CBaseNode* > &m_hList, CBaseNode *n, bool bHierachyUp )
{
	m_hList.AddToTail( this );
	bool bFound = false;
	const int numJacks = bHierachyUp ? GetNumJacks_In() : GetNumJacks_Out();
	for ( int i = 0; i < numJacks && !bFound; i++ )
	{
		CJack *j = bHierachyUp ? GetJack_In( i ) : GetJack_Out( i );
		for ( int x = 0; x < j->GetNumBridges() && !bFound; x++ )
		{
			CBridge *b = j->GetBridge( x );
			CBaseNode *next = bHierachyUp ? b->GetInputNode() : b->GetDestinationNode();
			if ( next == NULL )
				continue;
			if ( next == n )
				return true;
			else if ( !m_hList.IsValidIndex( m_hList.Find( next ) ) )
				bFound = bFound || next->RecursiveFindNode_Internal( m_hList, n, bHierachyUp );
		}
	}
	return bFound;
}
bool CBaseNode::RecursiveFindNode( CBaseNode *n, bool bHierachyUp )
{
	CUtlVector< CBaseNode* > m_hList;
	bool bResult = RecursiveFindNode_Internal( m_hList, n, bHierachyUp );
	m_hList.Purge();
	return bResult;
}

Vector2D CBaseNode::GetBoundsMin()
{
	return GetBoundsTitleMin();
}
Vector2D CBaseNode::GetBoundsMax()
{
	return GetBoundsBoxMax();
}
Vector2D CBaseNode::GetBoundsMinNodeSpace()
{
	Vector2D _min = GetBoundsMin();
	Vector2D _max = GetBoundsMax();
	Vector2D out( min( _min.x, _max.x ),
		min( _min.y, _max.y ) );
	return out;
}
Vector2D CBaseNode::GetBoundsMaxNodeSpace()
{
	Vector2D _min = GetBoundsMin();
	Vector2D _max = GetBoundsMax();
	Vector2D out( max( _min.x, _max.x ),
		max( _min.y, _max.y ) );
	return out;
}
Vector2D CBaseNode::GetSelectionBoundsMinNodeSpace()
{
	return GetBoundsMinNodeSpace();
}
Vector2D CBaseNode::GetSelectionBoundsMaxNodeSpace()
{
	return GetBoundsMaxNodeSpace();
}
Vector2D CBaseNode::GetCenter()
{
	Vector2D _min = GetBoundsMinNodeSpace();
	Vector2D _max = GetBoundsMaxNodeSpace();
	return _min + ( _max - _min ) * 0.5f;
}
Vector2D CBaseNode::GetBoundsTitleMin()
{
	return m_vecPosition;
}
Vector2D CBaseNode::GetBoundsTitleMax()
{
	return Vector2D( m_vecPosition + Vector2D( m_vecSize.x, -NODE_DRAW_TITLE_Y ) );
}
Vector2D CBaseNode::GetBoundsBoxMin()
{
	return Vector2D( m_vecPosition - Vector2D( 0, NODE_DRAW_TITLE_Y + NODE_DRAW_TITLE_SPACE ) );
}
Vector2D CBaseNode::GetBoundsBoxMax()
{
	return Vector2D( GetBoundsBoxMin() + m_vecSize );
}
bool CBaseNode::IsWithinBounds_Base( const Vector2D &pos )
{
	Vector2D _min, _max;
	_min = GetBoundsMin();
	_max = GetBoundsMax();

	if ( pos.x >= _min.x && pos.y <= _min.y &&
		pos.x <= _max.x && pos.y >= _max.y )
		return true;
	return false;
}

void CBaseNode::OnLeftClick( Vector2D &pos )
{
}
void CBaseNode::OnDragStart()
{
}
void CBaseNode::OnDrag( Vector2D &delta )
{
	Vector2D pos = GetPosition();
	pos += delta;
	SetPosition( pos );
}
void CBaseNode::OnDragEnd()
{
}
bool CBaseNode::MustDragAlone()
{
	return false;
}

void CBaseNode::UpdateParentContainers()
{
	Vector2D center = GetContainerSensitiveCenter();
	CUtlVector< CBaseContainerNode* > hContainers_Add;
	CUtlVector< CBaseContainerNode* > hContainers_Remove;
	pNodeView->ListContainerAtPos( center, hContainers_Add );

	for ( int i = 0; i < GetNumContainers(); i++ )
	{
		CBaseContainerNode *c = GetContainer( i );
		if ( hContainers_Add.IsValidIndex( hContainers_Add.Find( c ) ) )
			hContainers_Add.FindAndRemove( c );
		else
			hContainers_Remove.AddToTail( c );
	}

	for ( int i = 0; i < hContainers_Remove.Count(); i++ )
		hContainers_Remove[ i ]->RemoveChild( this );
	for ( int i = 0; i < hContainers_Add.Count(); i++ )
		hContainers_Add[ i ]->AddChild( this );

	hContainers_Add.Purge();
	hContainers_Remove.Purge();
}

void CBaseNode::SetPosition( Vector2D vec, bool bCenter )
{
	if ( bCenter )
	{
		Vector2D delta = GetBoundsMin() + ( GetBoundsMax() - GetBoundsMin() ) * 0.5f;
		vec -= delta;
	}
	m_vecPosition = vec;

	TouchJacks();

	UpdateSimpleObjectBounds( m_vecPosition, m_vecSize, m_vecBounds );

	UpdateParentContainers();
}
Vector2D CBaseNode::GetPosition()
{
	return m_vecPosition;
}
Vector2D CBaseNode::GetSize()
{
	return m_vecSize;
}
const Vector4D &CBaseNode::GetBoundsFast()
{
	return m_vecBounds;
}

void CBaseNode::SetAllocating( const bool &a )
{
	m_bIsAllocating = a;
}
bool CBaseNode::IsAllocating()
{
	if ( !GetNumSolvers() || HasDummySolvers() )
		return false;
	return m_bIsAllocating;
}
bool CBaseNode::VguiDraw( bool bShadow )
{
	if ( !ShouldSimpleDrawObject( pNodeView, pNodeView, m_vecBounds ) )
		return false;
	if ( !pNodeView )
		return false;

	Vector2D title_min = GetBoundsTitleMin();
	Vector2D title_max = GetBoundsTitleMax();
	Vector2D box_min = GetBoundsBoxMin();
	Vector2D box_max = GetBoundsBoxMax();

	pNodeView->ToPanelSpace( title_min );
	pNodeView->ToPanelSpace( title_max );
	pNodeView->ToPanelSpace( box_min );
	pNodeView->ToPanelSpace( box_max );

	float flZoom = pNodeView->GetZoomScalar();
	if ( bShadow )
	{
		float flO = NODE_DRAW_SHADOW_DELTA * flZoom;
		Vector2D offset( flO, flO );
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
		else if ( GetNumContainers() )
			boxNoShadow = NODE_DRAW_COLOR_HASCONTAINER_BOX;

		surface()->DrawSetColor( boxNoShadow );
	}

	surface()->DrawFilledRect( box_min.x, box_min.y, box_max.x, box_max.y );

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
		surface()->DrawFilledRect( box_max.x - borderSize, box_min.y, box_max.x, box_max.y );

		//float offsetTextBorder = m_vecSize.x - m_flMinSizeX;

		//if ( m_vecBorderInfo.x > 1 || m_vecBorderInfo.y IsPreviewEnabled() )
		if ( m_vecBorderInfo.x > 1 && ( IsPreviewVisible() ) )
		{
			Vector2D DivideStart( m_vecPosition.x + m_vecBorderInfo.x, m_vecPosition.y );
			pNodeView->ToPanelSpace( DivideStart );
			surface()->DrawFilledRect( DivideStart.x, box_min.y, DivideStart.x + borderSize, box_max.y );
		}
		if ( m_vecBorderInfo.y > 1 && ( m_vecBorderInfo.x > 1 || IsPreviewVisible() ) )
		{
			Vector2D DivideStart( m_vecPosition.x + m_vecSize.x - m_vecBorderInfo.y, m_vecPosition.y );
			pNodeView->ToPanelSpace( DivideStart );
			surface()->DrawFilledRect( DivideStart.x, box_min.y, DivideStart.x + borderSize, box_max.y );
		}
	}

	Vector2D titleMid = ( title_max - title_min ) * 0.5f + title_min;

	wchar_t szconverted[ 256 ];
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

		VguiDraw_Preview();
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
void CBaseNode::VguiDraw_Jacks( bool bShadow )
{
	for ( int i = 0; i < m_hInputs.Count(); i++ )
		m_hInputs[ i ]->VguiDraw(bShadow);
	for ( int i = 0; i < m_hOutputs.Count(); i++ )
		m_hOutputs[ i ]->VguiDraw(bShadow);
}
void CBaseNode::VguiDraw_Preview()
{
	if ( !IsPreviewVisible() )
		return;

	CUtlVector< CUtlVector< CHLSL_SolverBase* >* > hStackList;
	hStackList.AddToTail( &pNodeView->AccessSolverStack_PS() );
	hStackList.AddToTail( &pNodeView->AccessSolverStack_UNDEFINED() );
	hStackList.AddToTail( &pNodeView->AccessSolverStack_POSTPROC() );

	int puzzleIdx = -1;
	for ( int i = 0; i < hStackList.Count() && puzzleIdx < 0; i++ )
	{
		for ( int s = 0; s < hStackList[i]->Count() && puzzleIdx < 0; s++ )
		{
			if ( hStackList[i]->Element(s)->GetData().iNodeIndex == GetUniqueIndex() )
			{
				puzzleIdx = hStackList[i]->Element(s)->GetMapIndex();
				break;
			}
		}
	}

	//if ( puzzleIdx < 0 )
	//	return;
	//Msg("own idx: %u - puzzle: %i\n",GetUniqueIndex(),puzzleIdx);

	float preview_inset = PREVIEWINSET;


	Vector2D prev_min( m_vecPosition.x + m_vecBorderInfo.x + preview_inset, GetBoundsBoxMin().y - preview_inset );
	Vector2D prev_max( m_vecPosition.x + m_vecSize.x - m_vecBorderInfo.y - preview_inset, GetBoundsBoxMax().y + preview_inset );

	float deltax = prev_max.x - prev_min.x;
	float deltay = prev_min.y - prev_max.y;
	float deltamin = min ( deltay, deltax );
	prev_min.y = prev_max.y + deltamin;

	Vector2D _mid = prev_min + ( prev_max - prev_min ) * 0.5f;
	deltamin *= 0.5f;
	prev_min = _mid - Vector2D( deltamin, -deltamin );
	prev_max = _mid + Vector2D( deltamin, -deltamin );

	pNodeView->ToPanelSpace( prev_min );
	pNodeView->ToPanelSpace( prev_max );

	if ( GetErrorLevel() == ERRORLEVEL_NONE && GetNumSolvers() && !GetSolver(0)->IsDummy() && sedit_2dprev_Enable.GetInt() )
	{
		Vector2D a,b;
		GetUVsForPuzzle(puzzleIdx,a,b,true);
		surface()->DrawSetTexture( m_iPreviewMaterial );
		surface()->DrawSetColor( Color( 255,255,255,255 ) );
		surface()->DrawTexturedSubRect(prev_min.x,prev_min.y,
									prev_max.x,prev_max.y,
									a.x,a.y,
									b.x,b.y );
	}
	else
	{
		surface()->DrawSetColor( Color( 0,0,0,64 ) );
		surface()->DrawFilledRect( prev_min.x,prev_min.y,
									prev_max.x,prev_max.y );
	}
}

KeyValues *CBaseNode::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = new KeyValues( VarArgs("Node_%03i", NodeIndex) );

	pKV->SetInt( "iType", GetNodeType() );
	if ( Q_strlen(szNodeName) )
		pKV->SetString( "szName", szNodeName );

	pKV->SetFloat( "pos_x", m_vecPosition.x );
	pKV->SetFloat( "pos_y", m_vecPosition.y );
	pKV->SetFloat( "size_x", m_vecSize.x );
	pKV->SetFloat( "size_y", m_vecSize.y );
	pKV->SetInt( "iPreview", IsPreviewEnabled() ? 1 : 0 );

	KeyValues *pKV_Bridges = new KeyValues( "bridges" );
	pKV->AddSubKey( pKV_Bridges );

	for ( int i = 0; i < m_hInputs.Count(); i++ )
	{
		CJack *j = m_hInputs[ i ];
		if ( j->GetNumBridges() )
		{
			for ( int b = 0; b < j->GetNumBridges(); b++ )
			{
				CBridge *pBridge = j->GetBridge( b );

				KeyValues *pKV_Jack = new KeyValues( VarArgs( "jackIn_%02i", i ) );
				pKV_Bridges->AddSubKey( pKV_Jack );

				CBaseNode *pTargetNode = pBridge->GetEndNode( this );
				CJack *pTargetJack = pBridge->GetEndJack( this );
				if ( pTargetJack && pTargetNode )
				{
					int targetIndex_Node = pNodeView->GetNodeIndex( pTargetNode );
					int targetIndex_Jack = pTargetJack->GetSlot();

					pKV_Jack->SetInt( "iTarget_node", targetIndex_Node );
					pKV_Jack->SetInt( "iTarget_jack", targetIndex_Jack );
				}
			}
		}
	}

	return pKV;
}

void CBaseNode::RestoreFromKeyValues( KeyValues *pKV )
{
//	m_iType = pKV->GetInt( "iType" );
	Q_snprintf( szNodeName, sizeof(szNodeName), "%s", pKV->GetString( "szName" ) );

	SetPosition( Vector2D( pKV->GetFloat( "pos_x" ), pKV->GetFloat( "pos_y" ) ) );
	m_vecSize.x = pKV->GetFloat( "size_x" );
	m_vecSize.y = pKV->GetFloat( "size_y" );
	m_bPreviewEnabled = !!pKV->GetInt( "iPreview" );

	RestoreFromKeyValues_Specific( pKV );

	//UpdatePreviewAllowed();
	UpdateSize();
}
void CBaseNode::RestoreFromKeyValues_CreateBridges( KeyValues *pKV )
{
	PurgeBridges( true, false );

	KeyValues *pKV_Bridges = pKV->FindKey( "bridges" );
	if ( pKV_Bridges )
	{
		for ( int i = 0; i < GetNumJacks_In(); i++ )
		{
			KeyValues *pKV_Jack = pKV_Bridges->FindKey( VarArgs( "jackIn_%02i", i ) );
			if ( pKV_Jack )
			{
				CJack *localJack = GetJack_In( i );

				int iTGNode = pKV_Jack->GetInt( "iTarget_node" );
				int iTGJack = pKV_Jack->GetInt( "iTarget_jack" );

				CBaseNode *tgNode = pNodeView->GetNodeFromIndex( iTGNode );
				CJack *tgJack = ( tgNode && tgNode->GetNumJacks_Out() > iTGJack ) ? tgNode->GetJack_Out( iTGJack ) : NULL;

				if ( tgNode && tgJack && localJack )
				{
					tgJack->BridgeEndBuild( localJack->BridgeBeginBuild() );
				}
			}
		}
	}
}
