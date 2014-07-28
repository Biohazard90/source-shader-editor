
#include "cbase.h"
#include "editorcommon.h"

CNodePP_Base::CNodePP_Base( const char *opName, CNodeView *p ) : BaseClass( opName, p )
{
	SetupVguiTex( m_iPreviewMaterial, "shadereditor/_rt_ppe" );
}
CNodePP_Base::~CNodePP_Base()
{
}
const int CNodePP_Base::GetAllowedFlowgraphTypes()
{
	return CNodeView::FLOWGRAPH_POSTPROC;
}
CNodePP_Base *CNodePP_Base::GetNextPPNode( int dir )
{
	const bool bBack = dir < 0;
	CJack *pJ = bBack ? (GetNumJacks_In()>0)?GetJack_In(0):NULL : (GetNumJacks_Out()>0)?GetJack_Out(0):NULL;
	if ( !pJ )
		return NULL;
	if ( pJ->GetNumBridgesConnected() <= 0 )
		return NULL;
	
	CNodePP_Base *pNode = dynamic_cast<CNodePP_Base*>( pJ->GetBridge(0)->GetEndNode(this) );

	return pNode;
}


CNodePP_Input::CNodePP_Input( CNodeView *p ) : BaseClass( "PP Input", p )
{
	m_flMinSizeX = CRUCIALNODE_MIN_SIZE_X;
	m_flMinSizeY = 0;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "Chain master" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
}
CNodePP_Input::~CNodePP_Input()
{
}
bool CNodePP_Input::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_Start *solver = new CHLSL_Solver_PP_Start( GetUniqueIndex() );
	AddSolver( solver );
	return true;
}


CNodePP_Output::CNodePP_Output( CNodeView *p ) : BaseClass( "PP Output", p )
{
	m_flMinSizeX = CRUCIALNODE_MIN_SIZE_X;
	m_flMinSizeY = 0;

	GenerateJacks_Input( 1 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "Chain master" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	OnUpdateHierachy( NULL, NULL );
}
CNodePP_Output::~CNodePP_Output()
{
}
int CNodePP_Output::UpdateInputsValid()
{
	if ( GetNumJacks_In() > GetNumJacks_In_Connected() )
		return ERRORLEVEL_FAIL;
	return BaseClass::UpdateInputsValid();
}
bool CNodePP_Output::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_Base *solver = new CHLSL_Solver_PP_Base( GetUniqueIndex() );
	AddSolver( solver );
	return true;
}
