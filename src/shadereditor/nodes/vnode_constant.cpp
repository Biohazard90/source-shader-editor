
#include "cbase.h"
#include "editorCommon.h"


CNodeConstant::CNodeConstant( CNodeView *p ) : BaseClass( "Constant", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	m_iChannelInfo.SetDefaultChannels();
	m_iChannelInfo.bAllowChars = false;
	m_szConstantString[0] = '\0';

	UpdateNode();
}

CNodeConstant::~CNodeConstant()
{
}

KeyValues *CNodeConstant::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetString( "szConstantString", m_szConstantString );

	return pKV;
}
void CNodeConstant::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	char tmp[MAX_PATH];
	Q_snprintf( tmp, sizeof(tmp), "%s", pKV->GetString( "szConstantString" ) );

	m_iChannelInfo.Tokenize( tmp );
	m_iChannelInfo.PrintTargetString( m_szConstantString, MAX_PATH );

	m_flMinSizeY_VS = 0;
	if ( m_iChannelInfo.iActiveChannels >= 2 )
	{
		m_flMinSizeY_VS = 28;
		m_flMinSizeY_VS += 10 * (m_iChannelInfo.iActiveChannels - 2);
	}

	UpdateNode();
}

void CNodeConstant::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_Out( CBaseNode::BY_INDEX, m_hRestoreBridges );

	GenerateJacks_Output( 1 );

	if ( !m_iChannelInfo.IsActive() )
		m_iChannelInfo.Tokenize( "0" );

	Assert(m_iChannelInfo.IsActive());

	SetJackFlags_Output( 0, HLSLJACKFLAGS_F1 );
	LockJackOutput_Flags( 0, GetTypeFlagFromEnum(m_iChannelInfo.iActiveChannels) );

	// HLSLVAR_FLOAT1 - HLSLVAR_FLOAT4
	RestoreBridgesFromList_Out( m_hRestoreBridges );
}

bool CNodeConstant::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_Constant *solver = new CHLSL_Solver_Constant( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	solver->SetComponentInfo( m_iChannelInfo );
	AddSolver( solver );

	return true;
}

bool CNodeConstant::VguiDraw( bool bShadow )
{
	if ( !BaseClass::VguiDraw( bShadow ) )
		return false;

	if ( !m_iChannelInfo.IsActive() || bShadow )
		return false;

	bool bVis = false;
	HFont _font = pNodeView->GetFontScaled( 10.0f, bVis );
	if ( !bVis )
		return false;

	char szraw[32];
	wchar_t szconverted[ 32 ];
	
	const Vector2D bmin = GetBoundsBoxMin();
	const Vector2D bmax = GetBoundsBoxMax();
	const float _offset = PREVIEWINSET + 0.5f;
	const Vector2D text_pos( bmin.x + _offset, bmax.y + _offset );

	for ( int i = 0; i < m_iChannelInfo.iActiveChannels; i++ )
	{
		Assert( m_iChannelInfo.bChannelAsValue[i] );

		Vector2D curpos = text_pos;
		curpos.y += 10.0f * (m_iChannelInfo.iActiveChannels - i);
		pNodeView->ToPanelSpace( curpos );

		Q_snprintf( szraw, 32, "%f", m_iChannelInfo.flValue[i] );
		g_pVGuiLocalize->ConvertANSIToUnicode( szraw, szconverted, sizeof(szconverted)  );

		surface()->DrawSetTextFont( _font );
		surface()->DrawSetTextPos( curpos.x, curpos.y );

		surface()->DrawSetTextColor( NODE_DRAW_COLOR_TEXT_OP );
		surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
	}

	return true;
}





CNodeRandom::CNodeRandom( CNodeView *p ) : BaseClass( "Random", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Output( 1 );
	m_iNumComponents = 3;
	m_flValue_Min = 0;
	m_flValue_Max = 1;
	UpdateNode();
}
CNodeRandom::~CNodeRandom()
{
}
KeyValues *CNodeRandom::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_num_comps", m_iNumComponents );
	pKV->SetFloat( "fl_value_min", m_flValue_Min );
	pKV->SetFloat( "fl_value_max", m_flValue_Max );

	return pKV;
}
void CNodeRandom::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iNumComponents = pKV->GetInt( "i_num_comps", m_iNumComponents );
	m_flValue_Min = pKV->GetFloat( "fl_value_min", m_flValue_Min );
	m_flValue_Max = pKV->GetFloat( "fl_value_max", m_flValue_Max );
	UpdateNode();
}
void CNodeRandom::UpdateNode()
{
	LockJackOutput_Flags( 0, ::GetVarTypeFlag( m_iNumComponents ) );
}
bool CNodeRandom::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_Random *solver = new CHLSL_Solver_Random( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	solver->SetState( m_iNumComponents, m_flValue_Min, m_flValue_Max );
	AddSolver( solver );

	return true;
}


CNodeBumpBasis::CNodeBumpBasis( CNodeView *p ) : BaseClass( "Bump basis", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Output( 3 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "1" );
	LockJackOutput_Flags( 1, HLSLVAR_FLOAT3, "2" );
	LockJackOutput_Flags( 2, HLSLVAR_FLOAT3, "3" );
}
CNodeBumpBasis::~CNodeBumpBasis()
{
}
bool CNodeBumpBasis::CreateSolvers(GenericShaderData *ShaderData)
{
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *pJCur = GetJack_Out(i);
		if ( !pJCur->GetNumBridgesConnected() && i != 0 )
			continue;

		const int res = pJCur->GetResourceType();

		CHLSL_Var *tg = pJCur->AllocateVarFromSmartType();
		tg->MakeConstantOnly();

		CHLSL_Solver_BumpBasis *solver = new CHLSL_Solver_BumpBasis( GetUniqueIndex() );
		solver->SetResourceType( res );
		solver->AddTargetVar( tg );
		solver->SetState( i );
		AddSolver( solver );
	}

	return !!GetNumSolvers();
}
