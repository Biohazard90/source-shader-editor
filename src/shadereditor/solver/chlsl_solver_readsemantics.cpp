
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_ReadSemantics::CHLSL_Solver_ReadSemantics( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
}

void CHLSL_Solver_ReadSemantics::SendVSSetup( SETUP_HLSL_VS_Input s )
{
	vs_setup = s;
}

int CHLSL_Solver_ReadSemantics::GetTexcoordFlag( int numCoord )
{
	Assert( numCoord >= 0 && numCoord < 3 );
	return vs_setup.iDataTypeFlag_TexCoords[ numCoord ];
}

void CHLSL_Solver_ReadSemantics::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	const ResourceType_t &type = GetResourceType();
	const char *varname = GetResourceTypeName( type );
	const char *semanticsname = GetSemanticTypeName( type );
	const char *semanticdatatype = GetSemanticType( type );

	const bool bIsTexCoord = ( type >= RESOURCETYPE_TEXCOORD_0 && type <= RESOURCETYPE_TEXCOORD_7 );
	const bool bIsColor = ( type >= RESOURCETYPE_COLOR_0 && type <= RESOURCETYPE_COLOR_3 );

	char tmp[MAXTARGC];
	if ( bIsPixelShader && ( bIsTexCoord || bIsColor ) )
		//semanticdatatype = GetVarCodeNameFromFlag( pInfo->_IntValue_0 );
		semanticdatatype = GetVarCodeNameFromFlag( GetData()._IntValue_0 );
	else if ( !bIsPixelShader )
	{
		if ( bIsTexCoord )
		{
			int typenum = type - RESOURCETYPE_TEXCOORD_0;
			Assert( typenum >= 0 && typenum < 3 );
			semanticdatatype = GetVarCodeNameFromFlag( vs_setup.iDataTypeFlag_TexCoords[typenum] );
		}
		else if ( bIsColor )
		{
			int typenum = type - RESOURCETYPE_COLOR_0;
			Assert( typenum >= 0 && typenum < 3 );
			semanticdatatype = GetVarCodeNameFromFlag( vs_setup.iDataTypeFlag_Color[typenum] );
		}
	}

	Q_snprintf( tmp, MAXTARGC, "%s %s", semanticdatatype, varname );
	int tabsWorth = StringTabsWorth( tmp );
	Q_snprintf( tmp, MAXTARGC, "\n%s %s", semanticdatatype, varname );
	context.buf_semantics_In.PutString( tmp );

	for ( int i = 6 - tabsWorth; i >= 0; i-- )
		context.buf_semantics_In.PutString( "\t" );

	Q_snprintf( tmp, MAXTARGC, ":\t%s;", semanticsname  );
	context.buf_semantics_In.PutString( tmp );
}
void CHLSL_Solver_ReadSemantics::OnVarInit( const WriteContext_FXC &context )
{
	const ResourceType_t &type = GetResourceType();
	const char *varname = GetResourceTypeName( type );
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "In.%s", varname );
	target->SetName( tmp, true );
}

void CHLSL_Solver_ReadSemantics::Render( Preview2DContext &c )
{
	int opNum = 0;
	const ResourceType_t &type = GetResourceType();
	switch ( type )
	{
		case RESOURCETYPE_TEXCOORD_0:
		case RESOURCETYPE_TEXCOORD_1:
		case RESOURCETYPE_TEXCOORD_2:
		case RESOURCETYPE_TEXCOORD_3:
		case RESOURCETYPE_TEXCOORD_4:
		case RESOURCETYPE_TEXCOORD_5:
		case RESOURCETYPE_TEXCOORD_6:
		case RESOURCETYPE_TEXCOORD_7:
			opNum = type - RESOURCETYPE_TEXCOORD_0;
			break;
		case RESOURCETYPE_COLOR_0:
		case RESOURCETYPE_COLOR_1:
			opNum = type - RESOURCETYPE_COLOR_0 + NPSOP_WRITE_COLOR_0;
			break;
	}

	Assert( opNum >= NPSOP_WRITE_TEXCOORD_0 && opNum < NPSOP_WRITE_MAX );
	c.pMat_PsInOP->SetIntValue( opNum );
	CNodeView::RenderSingleSolver( c, c.pMat_PsIn );

	UpdateTargetVarToReflectMapIndex( 0 );
}