
#include "cbase.h"
#include "editorCommon.h"

void CHLSL_Solver_StdSkinning::SetState( int m )
{
	m_iState = m;
}
void CHLSL_Solver_StdSkinning::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	for ( int i = 0; i < this->GetNumTargetVars(); i++ )
		GetTargetVar(i)->DeclareMe( context, true );

	CHLSL_Var *pVar_ObjPos = GetSourceVar( 2 );
	CHLSL_Var *pVar_BoneIndices = GetSourceVar( 0 );
	CHLSL_Var *pVar_BoneWeights = GetSourceVar( 1 );
	CHLSL_Var *pVar_ObjNormal = ( m_iState >= STD_SKINNING_MODE_POS_NORMAL ) ? GetSourceVar( 3 ) : NULL;
	CHLSL_Var *pVar_ObjTangentS = ( m_iState >= STD_SKINNING_MODE_POS_NORMAL_TANGENT ) ? GetSourceVar( 4 ) : NULL;

	switch ( m_iState )
	{
	default:
		Assert(0);
	case STD_SKINNING_MODE_POS:
			Q_snprintf( tmp, MAXTARGC, "SkinPosition( SKINNING, float4( %s, 1 ),\n\t\t%s, %s, %s );\n",
				pVar_ObjPos->GetName(),
				pVar_BoneWeights->GetName(), pVar_BoneIndices->GetName(),
				GetTargetVar( 0 )->GetName() );
		break;
	case STD_SKINNING_MODE_POS_NORMAL:
			Q_snprintf( tmp, MAXTARGC, "SkinPositionAndNormal( SKINNING, float4( %s, 1 ), %s,\n\t\t%s, %s,\n\t\t%s, %s );\n",
				pVar_ObjPos->GetName(), pVar_ObjNormal->GetName(),
				pVar_BoneWeights->GetName(), pVar_BoneIndices->GetName(),
				GetTargetVar( 0 )->GetName(), GetTargetVar( 1 )->GetName() );
		break;
	case STD_SKINNING_MODE_POS_NORMAL_TANGENT:
			Q_snprintf( tmp, MAXTARGC, "SkinPositionNormalAndTangentSpace( SKINNING, float4( %s, 1 ), %s, %s,\n\t\t%s, %s,\n\t\t%s, %s, %s, %s );\n",
				pVar_ObjPos->GetName(), pVar_ObjNormal->GetName(), pVar_ObjTangentS->GetName(),
				pVar_BoneWeights->GetName(), pVar_BoneIndices->GetName(),
				GetTargetVar( 0 )->GetName(), GetTargetVar( 1 )->GetName(), GetTargetVar( 2 )->GetName(), GetTargetVar( 3 )->GetName() );
		break;
	}

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_StdSkinning::OnIdentifierAlloc( IdentifierLists_t &List )
{
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_SKINNING ) );
}



void CHLSL_Solver_StdMorph::SetState( int m )
{
	m_iState = m;
}
void CHLSL_Solver_StdMorph::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	for ( int i = 0; i < GetNumTargetVars(); i++ )
		GetTargetVar(i)->DeclareMe( context, true );

	CHLSL_Var *pVar_ObjPos_Read = GetSourceVar( 0 );
	CHLSL_Var *pVar_ObjPos_Write = GetTargetVar( 0 );
	CHLSL_Var *pVar_FlexPos = GetSourceVar( 1 );
	//CHLSL_Var *pVar_MorphCoords = GetSourceVar( 2 );

	CHLSL_Var *pVar_ObjNormal_Read = ( m_iState >= STD_SKINNING_MODE_POS_NORMAL ) ? GetSourceVar( 2 ) : NULL;
	CHLSL_Var *pVar_FlexNormal = ( m_iState >= STD_SKINNING_MODE_POS_NORMAL ) ? GetSourceVar( 3 ) : NULL;
	CHLSL_Var *pVar_ObjNormal_Write = ( m_iState >= STD_SKINNING_MODE_POS_NORMAL ) ? GetTargetVar( 1 ) : NULL;

	CHLSL_Var *pVar_ObjTangent_Read = ( m_iState >= STD_SKINNING_MODE_POS_NORMAL_TANGENT ) ? GetSourceVar( 4 ) : NULL;
	CHLSL_Var *pVar_ObjTangent_Write = ( m_iState >= STD_SKINNING_MODE_POS_NORMAL_TANGENT ) ? GetTargetVar( 2 ) : NULL;

	context.buf_code.PutString( "#if !defined( SHADER_MODEL_VS_3_0 ) || !MORPHING\n" );
	switch ( m_iState )
	{
	default:
		Assert(0);
	case STD_SKINNING_MODE_POS:
			Q_snprintf( tmp, MAXTARGC, "ApplyMorph( %s, %s, %s );\n",
				pVar_FlexPos->GetName(), pVar_ObjPos_Read->GetName(), pVar_ObjPos_Write->GetName() );
		break;
	case STD_SKINNING_MODE_POS_NORMAL:
			Q_snprintf( tmp, MAXTARGC, "ApplyMorph( %s, %s,\n\t\t%s, %s,\n\t\t%s, %s );\n",
				pVar_FlexPos->GetName(), pVar_FlexNormal->GetName(),
				pVar_ObjPos_Read->GetName(), pVar_ObjPos_Write->GetName(),
				pVar_ObjNormal_Read->GetName(), pVar_ObjNormal_Write->GetName() );
		break;
	case STD_SKINNING_MODE_POS_NORMAL_TANGENT:
			Q_snprintf( tmp, MAXTARGC, "ApplyMorph( %s, %s,\n\t\t%s, %s,\n\t\t%s, %s,\n\t\t%s, %s );\n",
				pVar_FlexPos->GetName(), pVar_FlexNormal->GetName(),
				pVar_ObjPos_Read->GetName(), pVar_ObjPos_Write->GetName(),
				pVar_ObjNormal_Read->GetName(), pVar_ObjNormal_Write->GetName(),
				pVar_ObjTangent_Read->GetName(), pVar_ObjTangent_Write->GetName() );
		break;
	}
	context.buf_code.PutString( tmp );
	context.buf_code.PutString( "#else\n" );
	char _texcoordName[64];
	//Q_snprintf( _texcoordName, sizeof(_texcoordName), "float3( %s, 0 )", pVar_MorphCoords->GetName() );
	Q_snprintf( _texcoordName, sizeof(_texcoordName), "float3( 0, 0, 0 )" );
	switch ( m_iState )
	{
	default:
		Assert(0);
	case STD_SKINNING_MODE_POS:
			Q_snprintf( tmp, MAXTARGC, "ApplyMorph( morphSampler, g_cMorphTargetTextureDim, g_cMorphSubrect,\n\t\tIn.vVertexID, %s,\n\t\t%s, %s );\n",
				_texcoordName,
				pVar_ObjPos_Read->GetName(), pVar_ObjPos_Write->GetName() );
		break;
	case STD_SKINNING_MODE_POS_NORMAL:
			Q_snprintf( tmp, MAXTARGC, "ApplyMorph( morphSampler, g_cMorphTargetTextureDim, g_cMorphSubrect,\n\t\tIn.vVertexID, %s,\n\t\t%s, %s,\n\t\t%s, %s );\n",
				_texcoordName,
				pVar_ObjPos_Read->GetName(), pVar_ObjPos_Write->GetName(),
				pVar_ObjNormal_Read->GetName(), pVar_ObjNormal_Write->GetName() );
		break;
	case STD_SKINNING_MODE_POS_NORMAL_TANGENT:
		Q_snprintf( tmp, MAXTARGC, "ApplyMorph( morphSampler, g_cMorphTargetTextureDim, g_cMorphSubrect,\n\t\tIn.vVertexID, %s,\n\t\t%s, %s,\n\t\t%s, %s,\n\t\t%s, %s );\n",
				_texcoordName,
				pVar_ObjPos_Read->GetName(), pVar_ObjPos_Write->GetName(),
				pVar_ObjNormal_Read->GetName(), pVar_ObjNormal_Write->GetName(),
				pVar_ObjTangent_Read->GetName(), pVar_ObjTangent_Write->GetName() );
		break;
	}
	context.buf_code.PutString( tmp );
	context.buf_code.PutString( "#endif\n" );
}
void CHLSL_Solver_StdMorph::OnIdentifierAlloc( IdentifierLists_t &List )
{
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_MORPHING ) );

	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_STUDIO_MORPHING;
	ec->iHLSLRegister = -1;
	List.hList_EConstants.AddToTail( ec );

	SimpleTexture *sampler = new SimpleTexture();
	sampler->iTextureMode = HLSLTEX_MORPH;
	sampler->m_hTargetNodes.AddToTail( new HNODE( GetData().iNodeIndex ) );
	List.hList_Textures.AddToTail( sampler );
}