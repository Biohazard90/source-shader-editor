
#include "cbase.h"
#include "editorCommon.h"



void CHLSL_Solver_WriteSemantics::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	const ResourceType_t &type = GetResourceType();
	const char *varname = GetResourceTypeName( type );
	const char *semanticsname = GetSemanticTypeName( type );
	const char *semanticdatatype = GetSemanticType( type );
	//int customDataType = -1; //pInfo->_IntValue_0;
	char tmp[MAXTARGC];

	if ( !bIsPixelShader && 
		(
		( type >= RESOURCETYPE_TEXCOORD_0 && type <= RESOURCETYPE_TEXCOORD_7 ) ||
		( type >= RESOURCETYPE_COLOR_0 && type <= RESOURCETYPE_COLOR_3 )
		) )
		//semanticdatatype = GetVarCodeNameFromFlag( pInfo->_IntValue_0 );
		semanticdatatype = GetVarCodeNameFromFlag( GetData()._IntValue_0 );

	Q_snprintf( tmp, MAXTARGC, "%s %s", semanticdatatype, varname );
	int tabsWorth = StringTabsWorth( tmp );
	Q_snprintf( tmp, MAXTARGC, "\n%s %s", semanticdatatype, varname );
	context.buf_semantics_Out.PutString( tmp );

	for ( int i = 6 - tabsWorth; i >= 0; i-- )
		context.buf_semantics_Out.PutString( "\t" );

	Q_snprintf( tmp, MAXTARGC, ":\t%s;", semanticsname );
	context.buf_semantics_Out.PutString( tmp );

	CHLSL_Var *source = source = GetSourceVar( 0 );
	Q_snprintf( tmp, MAXTARGC, "Out.%s = %s;\n", varname, source->GetName() );
	context.buf_code.PutString( tmp );
}