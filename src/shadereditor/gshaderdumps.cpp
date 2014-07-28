
#include "cbase.h"
#include "editorCommon.h"

void GetDumpListFilePath( char *out, int maxlen )
{
	Q_snprintf( out, maxlen, "%s/shaderlist.txt", ::GetDumpDirectory() );
	Q_FixSlashes( out );
}

KeyValues *df_LoadDump_List()
{
	char path_dump_list[MAX_PATH];
	GetDumpListFilePath( path_dump_list, MAX_PATH );

	KeyValues *pKV = new KeyValues( "shaderlist" );
	pKV->LoadFromFile( g_pFullFileSystem, path_dump_list, "MOD" );
	return pKV;
}

void df_SaveDump_List( KeyValues *pKV )
{
	char path_dump_list[MAX_PATH];
	GetDumpListFilePath( path_dump_list, MAX_PATH );

	pKV->SaveToFile( g_pFullFileSystem, path_dump_list, "MOD" );
	pKV->deleteThis();
}

void ReadGameShaders( CUtlVector<BasicShaderCfg_t*> &hList )
{
	KeyValues *pKV = df_LoadDump_List();

	KeyValues *pN = pKV->GetFirstValue();
	while ( pN )
	{
		BasicShaderCfg_t *shader = BuildShaderData( pN->GetString() );
		hList.AddToTail( shader );

		pN = pN->GetNextValue();
	}

	pKV->deleteThis();
}

void LoadGameShaders( bool bFirstRun )
{
	CUtlVector< BasicShaderCfg_t* >hListShaders;

	ReadGameShaders( hListShaders );

	for ( int i = 0; i < hListShaders.Count(); i++ )
	{
		const int iIndex = bFirstRun ? -1 : gProcShaderCTRL->FindPreloadShader( hListShaders[i]->CanvasName );

		if ( iIndex < 0 )
		{
			gProcShaderCTRL->AddPreloadShader( hListShaders[i] );
		}
		else
		{
			BasicShaderCfg_t *trash = (BasicShaderCfg_t*)gProcShaderCTRL->SwapPreloadShader( iIndex, hListShaders[i] );
			delete trash;
		}
	}

	hListShaders.Purge();
}


void UnloadGameShaders()
{
	if ( !gProcShaderCTRL )
		return;

	while ( gProcShaderCTRL->GetNumPreloadShaders() )
	{
		BasicShaderCfg_t *shader = ( BasicShaderCfg_t* )gProcShaderCTRL->GetAndRemovePreloadShader( 0 );
		delete shader;
	}

	for ( int i = 0; i < 2; i++ )
	{
		BasicShaderCfg_t *old = ( BasicShaderCfg_t* )gProcShaderCTRL->SwapShaderSystem( NULL, i );
		delete old;
	}
}


void ReadKVIdents_Texture( CUtlVector< SimpleTexture* > &hList, KeyValues *pKV )
{
	int itr = 0;
	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "texsamp_%i", itr );
	KeyValues *c = pKV->FindKey( tmp );

	while( c )
	{
		SimpleTexture *tex = new SimpleTexture();

		const char *paramname = c->GetString( "sz_param" );
		int len = Q_strlen( paramname ) + 1;
		tex->szParamName = new char[ len ];
		Q_snprintf( tex->szParamName, MAX_PATH, "%s", paramname );

		const char *fallbackname = c->GetString( "sz_fallback" );
		len = Q_strlen( fallbackname ) + 1;
		tex->szFallbackName = new char[ len ];
		Q_snprintf( tex->szFallbackName, MAX_PATH, "%s", fallbackname );

		tex->iSamplerIndex = c->GetInt( "i_sampidx" );
		tex->iTextureMode = c->GetInt( "i_texmode" );
		tex->bCubeTexture = !!c->GetInt( "i_cubemap" );
		tex->bSRGB = !!c->GetInt( "i_srgb" );

		hList.AddToTail( tex );
		itr++;
		Q_snprintf( tmp, MAX_PATH, "texsamp_%i", itr );
		c = pKV->FindKey( tmp );
	}
}
void ReadKVIdents_EConst( CUtlVector< SimpleEnvConstant* > &hList, KeyValues *pKV )
{
	int itr = 0;
	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "econst_%i", itr );
	KeyValues *c = pKV->FindKey( tmp );

	while( c )
	{
		SimpleEnvConstant *econst = new SimpleEnvConstant();

		econst->iEnvC_ID = c->GetInt( "i_envconstidx" );
		econst->iHLSLRegister = c->GetInt( "i_normregister" );
		econst->iConstSize = c->GetInt( "i_econstsize" );
		econst->iSmartNumComps = c->GetInt( "i_smartcomps" );

		const char *name = c->GetString( "sz_smartname" );
		int len = Q_strlen( name ) + 1;
		econst->szSmartHelper = new char[ len ];
		Q_snprintf( econst->szSmartHelper, MAX_PATH, "%s", name );

		for ( int i = 0; i < 4; i++ )
		{
			char tmpdef[MAX_PATH];
			Q_snprintf( tmpdef, MAX_PATH, "fl_smartdefault_%02i", i );
			econst->flSmartDefaultValues[ i ] = c->GetFloat( tmpdef );
		}

		hList.AddToTail( econst );
		itr++;
		Q_snprintf( tmp, MAX_PATH, "econst_%i", itr );
		c = pKV->FindKey( tmp );
	}
}
void ReadKVIdents_Combos( CUtlVector< SimpleCombo* > &hList, KeyValues *pKV )
{
	int itr = 0;
	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "combo_%i", itr );
	KeyValues *c = pKV->FindKey( tmp );

	while( c )
	{
		SimpleCombo *combo = new SimpleCombo();

		const char *name = c->GetString( "sz_name" );
		int len = Q_strlen( name ) + 1;
		combo->name = new char[ len ];
		Q_snprintf( combo->name, MAX_PATH, "%s", name );

		combo->bStatic = !!c->GetInt( "i_static" );
		combo->min = c->GetInt( "i_min" );
		combo->max = c->GetInt( "i_max" );
		combo->iComboType = c->GetInt( "i_type" );

		hList.AddToTail( combo );
		itr++;
		Q_snprintf( tmp, MAX_PATH, "combo_%i", itr );
		c = pKV->FindKey( tmp );
	}
}
void ReadIdents( IdentifierLists_t &idents, KeyValues *pKV )
{
	ReadKVIdents_Combos( idents.hList_Combos, pKV );
	ReadKVIdents_EConst( idents.hList_EConstants, pKV );
	ReadKVIdents_Texture( idents.hList_Textures, pKV );

	idents.inum_DynamicCombos = pKV->GetInt( "i_numdcombos" );
}
BasicShaderCfg_t *BuildShaderData( const char *dumpFileName )
{
	KeyValues *pKV = new KeyValues( dumpFileName );
	char _path[MAX_PATH];
	Q_snprintf( _path, MAX_PATH, "%s/%s.dump", ::GetDumpDirectory(), dumpFileName );
	Q_FixSlashes( _path );
	pKV->LoadFromFile( g_pFullFileSystem, _path, "MOD" );

	BasicShaderCfg_t *data = new BasicShaderCfg_t();

	const char *szT = pKV->GetString( "vs_name" );
	int len = Q_strlen( szT ) + 1;
	data->ProcVSName = new char[ len ];
	Q_snprintf( data->ProcVSName, len, "%s", szT );

	szT = pKV->GetString( "ps_name" );
	len = Q_strlen( szT ) + 1;
	data->ProcPSName = new char[ len ];
	Q_snprintf( data->ProcPSName, len, "%s", szT );

	szT = pKV->GetString( "shader_filename" );
	len = Q_strlen( szT ) + 1;
	data->Filename = new char[ len ];
	Q_snprintf( data->Filename, len, "%s", szT );
	Q_FixSlashes( data->Filename );

	szT = pKV->GetName();
	len = Q_strlen( szT ) + 1;
	data->CanvasName = new char[ len ];
	Q_snprintf( data->CanvasName, len, "%s", szT );

	Q_snprintf( data->dumpversion, sizeof(data->dumpversion), "%s", pKV->GetString( GetDumpVersion_KeyName() ) );

	data->iShaderModel = pKV->GetInt( "i_sm" );
	data->iCullmode = pKV->GetInt( "i_cull" );
	data->iAlphablendmode = pKV->GetInt( "i_ablend" );
	data->flAlphaTestRef = pKV->GetFloat( "fl_atestref" );
	data->iDepthtestmode = pKV->GetInt( "i_dtest" );
	data->iDepthwritemode = pKV->GetInt( "i_dwrite" );
	data->bsRGBWrite = !!pKV->GetInt( "i_srgbw" );

	data->iVFMT_flags = pKV->GetInt( "i_vfmt_flags" );
	data->iVFMT_numTexcoords = pKV->GetInt( "i_vfmt_texcoords" );
	data->iVFMT_numUserData = pKV->GetInt( "i_vfmt_udata" );
	for ( int i = 0; i < 3; i++ )
	{
		char tmp[48];
		Q_snprintf( tmp, sizeof(tmp), "i_vfmt_texcoordDim_%i", i );
		data->iVFMT_texDim[i] = pKV->GetInt( tmp, GetVarFlagsVarValue( HLSLVAR_FLOAT2 ) + 1 );
	}

	data->bVertexLighting = !!pKV->GetInt( "i_vlit" );
	data->bRefractionSupport = !!pKV->GetInt( "i_vrefract" );

	KeyValues *pSubIdent_VS = pKV->FindKey( "identifiers_VS" );
	if ( pSubIdent_VS )
		ReadIdents( *data->pVS_Identifiers, pSubIdent_VS );

	KeyValues *pSubIdent_PS = pKV->FindKey( "identifiers_PS" );
	if ( pSubIdent_PS )
		ReadIdents( *data->pPS_Identifiers, pSubIdent_PS );

	pKV->deleteThis();
	return data;
}


KeyValues *__AllocKV_Texture( int i, SimpleTexture *c )
{
	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "texsamp_%i", i );
	KeyValues *pKV = new KeyValues( tmp );

	pKV->SetString( "sz_param", c->szParamName );
	pKV->SetString( "sz_fallback", c->szFallbackName );
	pKV->SetInt( "i_sampidx", c->iSamplerIndex );
	pKV->SetInt( "i_texmode", c->iTextureMode );
	pKV->SetInt( "i_cubemap", c->bCubeTexture ? 1 : 0 );
	pKV->SetInt( "i_srgb", c->bSRGB ? 1 : 0 );

	return pKV;
}
/*
	// custom parameter - uniquify this
	char *szParamName;
	// sampler index - defined on uniquify
	int iSamplerIndex;
	// texture mode, bind standard or from param?
	int iTextureMode;
	// do we need a cubemap lookup?
	bool bCubeTexture;
	bool bSRGB;
*/
KeyValues *__AllocKV_EConst( int i, SimpleEnvConstant *c )
{
	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "econst_%i", i );
	KeyValues *pKV = new KeyValues( tmp );

	pKV->SetInt( "i_envconstidx", c->iEnvC_ID );
	pKV->SetInt( "i_normregister", c->iHLSLRegister );
	pKV->SetInt( "i_econstsize", c->iConstSize );
	pKV->SetInt( "i_smartcomps", c->iSmartNumComps );
	pKV->SetString( "sz_smartname", c->szSmartHelper );
	for ( int i = 0; i < 4; i++ )
	{
		char tmpdef[MAX_PATH];
		Q_snprintf( tmpdef, MAX_PATH, "fl_smartdefault_%02i", i );
		pKV->SetFloat( tmpdef, c->flSmartDefaultValues[ i ] );
	}
	
	return pKV;
}
/*
	int iEnvC_ID;
	int iHLSLRegister;
*/
KeyValues *__AllocKV_Combo( int i, SimpleCombo *c )
{
	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "combo_%i", i );
	KeyValues *pKV = new KeyValues( tmp );

	pKV->SetString( "sz_name", c->name );
	pKV->SetInt( "i_static", c->bStatic ? 1 : 0 );
	pKV->SetInt( "i_min", c->min );
	pKV->SetInt( "i_max", c->max );
	pKV->SetInt( "i_type", c->iComboType );

	return pKV;
}
/*
	char *name;
	bool bStatic;
	int min;
	int max;
*/
KeyValues *__AllocKV_Identifiers( IdentifierLists_t *pList )
{
	KeyValues *pIdents = new KeyValues("");

	for ( int i = 0; i < pList->hList_Combos.Count(); i++ )
		pIdents->AddSubKey( __AllocKV_Combo( i, pList->hList_Combos[i] ) );
	for ( int i = 0; i < pList->hList_EConstants.Count(); i++ )
		pIdents->AddSubKey( __AllocKV_EConst( i, pList->hList_EConstants[i] ) );
	for ( int i = 0; i < pList->hList_Textures.Count(); i++ )
		pIdents->AddSubKey( __AllocKV_Texture( i, pList->hList_Textures[i] ) );

	pIdents->SetInt( "i_numdcombos", pList->inum_DynamicCombos );

	return pIdents;
}
/*
	CUtlVector< SimpleTexture* > hList_Textures;
	CUtlVector< SimpleCombo* > hList_Combos;
	CUtlVector< SimpleEnvConstant* > hList_EConstants;
*/
void df_SaveDump_File( const char *canvasname, const BasicShaderCfg_t &shader )
{
	KeyValues *pKV = new KeyValues( canvasname );
	char _path[MAX_PATH];
	Q_snprintf( _path, MAX_PATH, "%s/%s.dump", ::GetDumpDirectory(), canvasname );
	Q_FixSlashes( _path );

	pKV->SetString( "vs_name", shader.ProcVSName );
	pKV->SetString( "ps_name", shader.ProcPSName );
	pKV->SetString( "shader_filename", shader.Filename );
	pKV->SetString( GetDumpVersion_KeyName(), GetDumpVersion_Current() );

	pKV->SetInt( "i_sm", shader.iShaderModel );
	pKV->SetInt( "i_cull", shader.iCullmode );
	pKV->SetInt( "i_ablend", shader.iAlphablendmode );
	pKV->SetFloat( "fl_atestref", shader.flAlphaTestRef );
	pKV->SetInt( "i_dtest", shader.iDepthtestmode );
	pKV->SetInt( "i_dwrite", shader.iDepthwritemode );
	pKV->SetInt( "i_srgbw", shader.bsRGBWrite ? 1 : 0 );
	
	pKV->SetInt( "i_vfmt_flags", shader.iVFMT_flags );
	pKV->SetInt( "i_vfmt_texcoords", shader.iVFMT_numTexcoords );
	pKV->SetInt( "i_vfmt_udata", shader.iVFMT_numUserData );
	for ( int i = 0; i < 3; i++ )
	{
		char tmp[48];
		Q_snprintf( tmp, sizeof(tmp), "i_vfmt_texcoordDim_%i", i );
		pKV->SetInt( tmp, shader.iVFMT_texDim[i] );
	}

	pKV->SetInt( "i_vlit", shader.bVertexLighting );
	pKV->SetInt( "i_vrefract", shader.bRefractionSupport );

	KeyValues *pKVIdentVS = __AllocKV_Identifiers( shader.pVS_Identifiers );
	pKVIdentVS->SetName( "identifiers_VS" );
	pKV->AddSubKey( pKVIdentVS );

	KeyValues *pKVIdentPS = __AllocKV_Identifiers( shader.pPS_Identifiers );
	pKVIdentPS->SetName( "identifiers_PS" );
	pKV->AddSubKey( pKVIdentPS );

	pKV->SaveToFile( g_pFullFileSystem, _path, "MOD" );
	pKV->deleteThis();
}
/*
struct BasicShaderCfg_t
{
	char *ProcVSName;
	char *ProcPSName;

	int iShaderModel;
	int iCullmode;
	int iAlphablendmode;
	float flAlphaTestRef;
	int iDepthtestmode;
	int iDepthwritemode;

	int iVFMT_flags;
	int iVFMT_numTexcoords;
	int iVFMT_numUserData;

	//CUtlVector< SimpleCombo* > hList_Combos_VS;
	//CUtlVector< SimpleCombo* > hList_Combos_PS;

	IdentifierLists_t *pVS_Identifiers;
	IdentifierLists_t *pPS_Identifiers;
};
*/