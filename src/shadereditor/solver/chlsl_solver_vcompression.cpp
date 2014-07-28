
#include "cbase.h"
#include "editorCommon.h"

void CHLSL_Solver_VCompression::SetCompressionType( int type )
{
	m_iType = type;
}

void CHLSL_Solver_VCompression::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	for ( int i = 0; i < this->GetNumTargetVars(); i++ )
		GetTargetVar(i)->DeclareMe( context, true );

	const bool bUseTangent = m_iType == CNodeVCompression::COMPRESSION_MODE_NORMAL_TANGENT;

	CHLSL_Var *pVar_Normal = GetSourceVar( 0 );
	CHLSL_Var *pVar_Tangent = bUseTangent ? GetSourceVar( 1 ) : NULL;

	switch ( m_iType )
	{
	default:
		AssertMsg( 0, "unimplemented compression type in solver" );
	case CNodeVCompression::COMPRESSION_MODE_NORMAL:
		Q_snprintf( tmp, MAXTARGC, "DecompressVertex_Normal( %s, %s );\n",
			pVar_Normal->GetName(), GetTargetVar(0)->GetName() );
		break;
	case CNodeVCompression::COMPRESSION_MODE_NORMAL_TANGENT:
		Q_snprintf( tmp, MAXTARGC, "DecompressVertex_NormalTangent( %s, %s, %s, %s );\n",
			pVar_Normal->GetName(), pVar_Tangent->GetName(),
			GetTargetVar(0)->GetName(), GetTargetVar(1)->GetName() );
		break;
	}

	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_VCompression::OnIdentifierAlloc( IdentifierLists_t &List )
{
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_VERTEXCOMPRESSION ) );
}