
#include "cbase.h"
#include "vSheets.h"

#define TEXPATH_WHITE "white" //"Lights\\White.vtf"
#define TEXPATH_BLACK "black" //"cable\\black.vtf"
#define TEXPATH_GREY "grey" //"Dev\\bump.vtf"
#define TEXPATH_NORMAL "dev\\flat_normal.vtf"

//int CSheet_TextureSample::m_iVguiPaint_Image= -1;

CSheet_TextureSample::CSheet_TextureSample(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	m_bReady = false;
	m_bSamplerOnly = false;
	m_pParameterName = new TextEntry(this, "parameter");
	m_pCheckSRGB_Read = new CheckButton(this, "srgb","");

	m_pComboBox_TextureType = new ComboBox( this, "textype", 7, false );
	for ( int i = 0; i <= HLSLTEXSAM_LAST_USABLE; i++ )
	{
		KeyValues *pKv = new KeyValues( GetTextureTypeName( i ) );
		pKv->SetInt( "type", i );
		m_pComboBox_TextureType->AddItem( GetTextureTypeName( i ), pKv );
		pKv->deleteThis();
	}
	m_pComboBox_TextureType->ActivateItem( HLSLTEX_BASETEXTURE );

	m_szDemoTexturePath[0] = '\0';
	m_pTextEntry_DemoTexture = new TextEntry( this, "demotexname" );
	m_pButton_LoadTexture = new Button( this, "loadtexture", "", this, "" );

	m_pTextEntry_FallbackTexture = new TextEntry( this, "fallbacktexname" );
	m_pComboBox_FallbackType = new ComboBox( this, "fallbacktype", 7, false );
	m_pComboBox_FallbackType->AddItem( "white", NULL );
	m_pComboBox_FallbackType->AddItem( "black", NULL );
	m_pComboBox_FallbackType->AddItem( "grey", NULL );
	m_pComboBox_FallbackType->AddItem( "normal", NULL );
	m_pComboBox_FallbackType->AddItem( "custom", NULL );
	m_pComboBox_FallbackType->ActivateItem( 0 );

	m_pComboBox_LookupOverride = new ComboBox( this, "lookupoverride", 10, false );
	for ( int i = 0; i < TEXSAMP_MAX; i++ )
		m_pComboBox_LookupOverride->AddItem( ::GetSamplerData_VisualName( i ), NULL );

	bFileIsEnvmap = false;
	SetupVguiTex( m_iVguiPaint_Texture, "shadereditor/vguiTexSample" );

	LoadControlSettings( "shadereditorui/vgui/sheet_texsample.res" );
}
CSheet_TextureSample::~CSheet_TextureSample()
{
	if ( m_hOpenTexture.Get() )
		m_hOpenTexture->MarkForDeletion();
}

void CSheet_TextureSample::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	Panel *pImgPos = FindChildByName( "imagepos" );
	if ( pImgPos )
		pImgPos->SetBorder(pScheme->GetBorder("DepressedButtonBorder"));
}

void CSheet_TextureSample::MakeSamplerOnly()
{
	m_bSamplerOnly = true;

	Panel *pP = FindChildByName( "Label_lookup" );
	if ( pP != NULL )
		pP->SetVisible( false );

	m_pComboBox_LookupOverride->SetVisible( false );
	m_pCheckSRGB_Read->SetVisible( false );
}

void CSheet_TextureSample::OnCommand( const char *cmd )
{
	if ( !m_bReady )
		return;
	bool bOpenBrowser = false;
	const char *pBContext = NULL;
	const char *pBPath = NULL;
	if ( !Q_stricmp( cmd, "openvtf" ) )
	{
		bOpenBrowser = true;
		pBContext = "open_demofile";
		pBPath = "%s/materials/texture_samples";
	}
	else if ( !Q_stricmp( cmd, "openfallback" ) )
	{
		bOpenBrowser = true;
		pBContext = "open_fallback";
		pBPath = "%s/materials";
	}

	if ( bOpenBrowser )
	{
		Assert( pBContext );
		Assert( pBPath );

		if ( m_hOpenTexture.Get() )
			m_hOpenTexture.Get()->MarkForDeletion();

		m_hOpenTexture = new FileOpenDialog( this, "Load .vtf", FOD_OPEN, new KeyValues("FileOpenContext", "context", pBContext) );

		if ( m_hOpenTexture.Get() )
		{
			char finalPath[MAX_PATH];
			Q_snprintf( finalPath, sizeof( finalPath ), pBPath, GetGamePath() );

			m_hOpenTexture->SetStartDirectoryContext( GetFODPathContext( FODPC_VTF ), finalPath );
			m_hOpenTexture->AddFilter( "*.vtf", "Textures", true );
			m_hOpenTexture->DoModal( true );
		}
	}
}
void CSheet_TextureSample::OnFileSelected( KeyValues *pKV )
{
	KeyValues *pContext = pKV->FindKey( "FileOpenContext" );
	if ( pContext )
	{
		const bool bMainMaterial = !Q_stricmp( pContext->GetString( "context" ), "open_demofile" );
		char tmparray[MAX_PATH*4];
		char *targetarray = m_szDemoTexturePath;
		if ( !bMainMaterial )
			targetarray = tmparray;

		const char *pathIn = pKV->GetString( "fullpath" );
		if ( Q_strlen( pathIn ) <= 1 )
			return;

		char tmp[MAX_PATH*4];
		char tmp2[MAX_PATH*4];
		Q_snprintf( tmp, sizeof( tmp ), "%s", pathIn );

		bool bIsAbsPath = false;
		if ( g_pFullFileSystem->FullPathToRelativePath( tmp, tmp2, sizeof(tmp2) ) )
			Q_strcpy( tmp, tmp2 );
		else if ( !g_pFullFileSystem->RelativePathToFullPath( tmp, NULL, tmp2, sizeof(tmp2) ) )
			bIsAbsPath = true;

		char *go = bIsAbsPath ? tmp : Q_StripFirstDir( tmp );
		Q_snprintf( targetarray, MAX_PATH*4, "%s", go );

		char tmpNoExt[MAX_PATH*4];
		Q_StripExtension( targetarray, tmpNoExt, MAX_PATH*4 );
		Q_strcpy( targetarray, tmpNoExt );

		Q_FixSlashes( targetarray );

		if ( bMainMaterial )
			m_pTextEntry_DemoTexture->SetText( targetarray );
		else
			m_pTextEntry_FallbackTexture->SetText( targetarray );

		if ( bMainMaterial )
			LoadFile();
	}
}
void CSheet_TextureSample::LoadFile()
{
	if ( Q_strlen( m_szDemoTexturePath ) < 1 )
		return;

	bFileIsEnvmap = false;

	//{
	//	unsigned char *pImgData = m_Image.GetVTF()->ImageData();
	//	if ( pImgData )
	//	{
	//		//m_iVguiPaint_Image = surface()->CreateNewTextureID( true );
	//		surface()->DrawSetTextureRGBA( m_iVguiPaint_Image, pImgData, m_Image.GetVTF()->Width(), m_Image.GetVTF()->Height(), 0, true );
	//	}
	//	m_pMat_Tex = NULL;
	//	m_Image.DestroyImage();
	//}
	//else

	{
		m_pMat_Tex = materials->FindMaterial( "shadereditor/vguiTexSample", TEXTURE_GROUP_OTHER );
		if ( !m_pMat_Tex )
			return;
		bool bFound = false;
		IMaterialVar *pBaseTex = m_pMat_Tex->FindVar( "$basetexture", &bFound );
		if ( !bFound )
			return;
		ITexture *tex = materials->FindTexture( m_szDemoTexturePath, TEXTURE_GROUP_OTHER );
		if ( IsErrorTexture(tex) )
			return;
		bFileIsEnvmap = tex->IsCubeMap();
		pBaseTex->SetTextureValue( tex );
		//tex->Download();
		//pBaseTex->SetStringValue( tmp2 );
#ifdef SHADER_EDITOR_DLL_2006
		m_pMat_Tex->Refresh();
#else
		m_pMat_Tex->RefreshPreservingMaterialVars();
#endif
		m_pMat_Tex->RecomputeStateSnapshots();
		//( "shadereditor/vguiTexSample" );
	}
}

void CSheet_TextureSample::OnTexturetypeSelect()
{
	int param = m_pComboBox_TextureType->GetActiveItem();
	bool bEnableCParram = param == HLSLTEX_CUSTOMPARAM;
	m_pParameterName->SetEnabled( bEnableCParram );
	m_pParameterName->SetEditable( bEnableCParram );

	const char *newTex = NULL;
	bool bAllowTextureLoad = false;
	bool bAllowSRGBSet = true;

	switch (param)
	{
	default:
	case HLSLTEX_CUSTOMPARAM:
	case HLSLTEX_BASETEXTURE:
	case HLSLTEX_BUMPMAP:
		bAllowTextureLoad = true;
		break;
	//case HLSLTEX_NORMAL:
	//	newTex = "dev\\flat_normal.vtf";
	//	break;
	case HLSLTEX_BLACK:
		newTex = TEXPATH_BLACK;
		break;
	case HLSLTEX_WHITE:
		newTex = TEXPATH_WHITE;
		break;
	case HLSLTEX_LIGHTMAP:
	case HLSLTEX_LIGHTMAP_BUMPMAPPED:
		bAllowSRGBSet = false;
	case HLSLTEX_GREY:
		newTex = TEXPATH_GREY;
		break;
	case HLSLTEX_FRAMEBUFFER:
		newTex = "_rt_FullFrameFB";
		break;
	}

	m_pButton_LoadTexture->SetEnabled( bAllowTextureLoad );
	m_pTextEntry_DemoTexture->SetEnabled( bAllowTextureLoad );

	m_pComboBox_FallbackType->SetEnabled( bAllowTextureLoad );
	m_pTextEntry_FallbackTexture->SetEnabled( bAllowTextureLoad );
	if ( !bAllowTextureLoad )
		m_pTextEntry_DemoTexture->SetText( "" );

	m_pCheckSRGB_Read->SetEnabled( bAllowSRGBSet );

	if ( newTex )
	{
		Q_snprintf( m_szDemoTexturePath, MAX_PATH, "%s", newTex );
		LoadFile();
	}
}

void CSheet_TextureSample::OnMenuClose( KeyValues *pKV )
{
	Panel *pCaller = ((Panel*)pKV->GetPtr( "panel" ));

	if ( pCaller == m_pComboBox_FallbackType )
	{
		int index = m_pComboBox_FallbackType->GetActiveItem();
		const char *tgName = NULL;
		switch ( index )
		{
		case 0:
			tgName = TEXPATH_WHITE;
			break;
		case 1:
			tgName = TEXPATH_BLACK;
			break;
		case 2:
			tgName = TEXPATH_GREY;
			break;
		case 3:
			tgName = TEXPATH_NORMAL;
			break;
		}
		if ( tgName )
			m_pTextEntry_FallbackTexture->SetText( tgName );
		else
			OnCommand( "openfallback" );
	}
}
void CSheet_TextureSample::OnTextChanged( KeyValues *pKV )
{
	Panel *pCaller = ((Panel*)pKV->GetPtr( "panel" ));

	if ( pCaller == m_pTextEntry_DemoTexture )
	{
		char text[MAX_PATH];
		m_pTextEntry_DemoTexture->GetText( text, MAX_PATH );
		if ( Q_strlen( text ) < 1 )
			return;
		ITexture *tex = materials->FindTexture( text, TEXTURE_GROUP_OTHER, false );
		if ( !tex )
			return;

		Q_snprintf( m_szDemoTexturePath, MAX_PATH, "%s", text );
		LoadFile();
	}
	else if ( pCaller == m_pComboBox_TextureType )
		OnTexturetypeSelect();
	else if ( pCaller == m_pTextEntry_FallbackTexture )
		m_pComboBox_FallbackType->ActivateItem( 4 );
}
void CSheet_TextureSample::Paint()
{
	BaseClass::Paint();

	Panel *pImgPos = FindChildByName( "imagepos" );
	if ( pImgPos )
	{
		int px,py,sx,sy;
		pImgPos->GetBounds( px, py, sx, sy );
		
		//if (m_pMat_Tex)
			surface()->DrawSetTexture( m_iVguiPaint_Texture );
		//else
		//	surface()->DrawSetTexture( m_iVguiPaint_Image );
		surface()->DrawSetColor( Color(255,255,255,255) );
		surface()->DrawTexturedRect( px, py, px+sx, py+sy );
	}
}

void CSheet_TextureSample::OnResetData()
{
	m_pParameterName->SetText( pData->GetString( "szParamname" ) );
	m_pCheckSRGB_Read->SetSelected( ( pData->GetInt( "iSRGB" ) != 0 ) );
	m_pComboBox_TextureType->ActivateItem( pData->GetInt( "iTextureType" ) );
	Q_snprintf( m_szDemoTexturePath, MAX_PATH, "%s", pData->GetString("szDemoTexturePath") );

	m_pTextEntry_DemoTexture->SetText( m_szDemoTexturePath );

	bool bEnableCParram = m_pComboBox_TextureType->GetActiveItem() == HLSLTEX_CUSTOMPARAM;
	m_pParameterName->SetEnabled( bEnableCParram );

	const char *fallbackname = pData->GetString("szFallbackTexturePath");
	int fallbackmode = pData->GetInt( "iFallbackMode");
	if ( !fallbackname || !*fallbackname )
	{
		fallbackname = TEXPATH_WHITE;
		fallbackmode = 0;
	}
	m_pTextEntry_FallbackTexture->SetText( fallbackname );
	m_pComboBox_FallbackType->ActivateItem( fallbackmode );

	m_pComboBox_LookupOverride->ActivateItem( pData->GetInt( "iLookupoverride" ) );

	OnTexturetypeSelect();
	LoadFile();

	m_bReady = true;
}
void CSheet_TextureSample::OnApplyChanges()
{
	char name[MAX_PATH];
	char nameModified[MAX_PATH];
	m_pParameterName->GetText( name, MAX_PATH );
	CleanupString( name, nameModified, MAX_PATH );

	int textype = m_pComboBox_TextureType->GetActiveItem();
	pData->SetString( "szParamname", nameModified );
	pData->SetInt( "iSRGB", m_pCheckSRGB_Read->IsSelected() ? 1 : 0 );
	pData->SetInt( "iTextureType", textype );
	pData->SetString( "szDemoTexturePath", m_szDemoTexturePath );

	bool bCustomTexIsCubemap = bFileIsEnvmap &&
		(textype == HLSLTEX_CUSTOMPARAM ||
		textype == HLSLTEX_BASETEXTURE);
	bool bCubemap = textype == HLSLTEX_ENVMAP || bCustomTexIsCubemap;
	pData->SetInt( "IsCubemap", bCubemap ? 1 : 0 );

	char tmp[MAX_PATH];
	m_pTextEntry_FallbackTexture->GetText( tmp, MAX_PATH );
	pData->SetString( "szFallbackTexturePath", tmp );
	pData->SetInt( "iFallbackMode", m_pComboBox_FallbackType->GetActiveItem() );

	pData->SetInt( "iLookupoverride", m_pComboBox_LookupOverride->GetActiveItem() );
}
