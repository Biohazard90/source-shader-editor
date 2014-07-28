
#include "cbase.h"
#include "editorCommon.h"
//#include "vgui_int.h"

#include "vgui_controls/colorpicker.h"

#define PREVBROWSER_CONTEXT_MODEL "context_model"
#define PREVBROWSER_CONTEXT_BG "context_background"

CPreview::CPreview( vgui::Panel *parent, CNodeView *nodeview ) : BaseClass( parent, nodeview, "preview", true )
{
	//m_szEnvmap[0] = '\0';
	InitColors();

	pRenderPanel = new CRenderPanel( this, "renderpanel" );

	m_pCBox_RenderMode = new ComboBox( this, "cbox_rendermode", 10, false );
	m_pCBox_RenderMode->AddItem( "Sphere", NULL );
	m_pCBox_RenderMode->AddItem( "Cube", NULL );
	m_pCBox_RenderMode->AddItem( "Cylinder", NULL );
	m_pCBox_RenderMode->AddItem( "Plane", NULL );
	m_pCBox_RenderMode->AddItem( "Model", NULL );
	m_pCBox_RenderMode->AddItem( "Post process", NULL );
	m_pCBox_RenderMode->AddItem( "Pingpong RT 0", NULL );

	m_pCBox_Model_Sequence = new ComboBox( this, "cbox_modelseq", 20, false );
	m_pCheck_MatOverride = new CheckButton( this, "check_moverride", "" );
	m_pCheck_DoPPE = new CheckButton( this, "check_ppe", "" );

	LoadControlSettings("shadereditorui/vgui/shadereditor_preview.res");

	m_pCheck_MatOverride->SetSelected( true );
	m_pCheck_DoPPE->SetSelected( true );

	SetTitle("Preview",true);
	SetMinimumSize( 440, 320 );

	SetAutoDelete( true );
	SetDeleteSelfOnClose( false );

	m_pCBox_RenderMode->ActivateItem( 0 );
	DoLayout_Empty();

	psz_SequenceList = NULL;
	m_iNumSequences = 0;
}
CPreview::~CPreview()
{
	KillBrowser();

	if ( psz_SequenceList && sEditMRender )
	{
		sEditMRender->DestroyModel();
		sEditMRender->DestroyCharPtrList( &psz_SequenceList );
		psz_SequenceList = NULL;
		m_iNumSequences = 0;
	}
}

void CPreview::OnTextChanged( KeyValues *pKV )
{
	Panel *pCaller = ((Panel*)pKV->GetPtr( "panel" ));
	if ( pCaller == m_pCBox_RenderMode )
	{
		UpdateLayout();
	}
	else if ( pCaller == m_pCBox_Model_Sequence )
	{
		if ( sEditMRender )
		{
			char tmp[ MAX_PATH ];
			m_pCBox_Model_Sequence->GetItemText( m_pCBox_Model_Sequence->GetActiveItem(), tmp, MAX_PATH );
			sEditMRender->SetSequence( tmp );
		}
	}
	else
		Assert( 0 );

	return;
}
void CPreview::OnCheckButtonChecked( KeyValues *pKV )
{
	Panel *pCaller = ((Panel*)pKV->GetPtr( "panel" ));
	const bool bState = !!pKV->GetInt( "state" );

	if ( pCaller == m_pCheck_MatOverride )
	{
		pRenderPanel->SetMatOverrideEnabled( bState );
	}
	else if ( pCaller == m_pCheck_DoPPE )
	{
		pRenderPanel->SetPPEPreviewEnabled( bState );
	}
}
void CPreview::DoLayout_Empty()
{
	m_pCBox_RenderMode->SetVisible( true );
	Panel *p = FindChildByName( "button_modelload" ); if ( p ) p->SetVisible( false );
	p = FindChildByName( "label_sequence" ); if ( p ) p->SetVisible( false );
	for ( int i = 1; i < 4; i++ ){
		p = FindChildByName( VarArgs("Button%i",i) ); if ( p ) p->SetVisible( true );}
	
	m_pCBox_Model_Sequence->SetVisible( false );
	m_pCheck_MatOverride->SetVisible( false );
	m_pCheck_DoPPE->SetVisible( false );
}
void CPreview::DoLayout_Model()
{
	m_pCBox_RenderMode->SetVisible( true );
	Panel *p = FindChildByName( "button_modelload" ); if ( p ) p->SetVisible( true );
	p = FindChildByName( "label_sequence" ); if ( p ) p->SetVisible( true );
	for ( int i = 1; i < 4; i++ ){
		p = FindChildByName( VarArgs("Button%i",i) ); if ( p ) p->SetVisible( true );}

	m_pCBox_Model_Sequence->SetVisible( true );
	m_pCheck_MatOverride->SetVisible( true );
	m_pCheck_DoPPE->SetVisible( false );
}
void CPreview::DoLayout_PPE()
{
	m_pCBox_RenderMode->SetVisible( false );
	Panel *p = FindChildByName( "button_modelload" ); if ( p ) p->SetVisible( false );
	p = FindChildByName( "label_sequence" ); if ( p ) p->SetVisible( false );
	for ( int i = 1; i < 4; i++ ){
		p = FindChildByName( VarArgs("Button%i",i) ); if ( p ) p->SetVisible( false );}

	m_pCBox_Model_Sequence->SetVisible( false );
	m_pCheck_MatOverride->SetVisible( false );
	m_pCheck_DoPPE->SetVisible( true );
}

void CPreview::UpdateLayout()
{
	int var = m_pCBox_RenderMode->GetActiveItem();
	if ( var == PRENDER_MODEL && !sEditMRender )
		var = PRENDER_SPHERE;

	if ( pEditorRoot->GetSafeFlowgraph()->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC )
		var = PRENDER_PPECHAIN;

	switch ( var )
	{
	case PRENDER_SPHERE:
	case PRENDER_CUBE:
	case PRENDER_CYLINDER:
	case PRENDER_PLANE:
		pRenderPanel->CreateMesh( var );
	default:
		DoLayout_Empty();
		break;
	case PRENDER_MODEL:
		DoLayout_Model();
		break;
	case PRENDER_PPECHAIN:
		DoLayout_PPE();
		break;
	}

	pRenderPanel->SetRenderMode( var );
}

void CPreview::OnSceneRender()
{
	pRenderPanel->DrawScene();
}

void CPreview::OnPostRender()
{
	pRenderPanel->DrawPostProc();
}

void CPreview::Activate()
{
	BaseClass::Activate();
}
void CPreview::OnFinishedClose()
{
	if ( GetParent() )
		GetParent()->OnCommand( "pv_closed" );

	BaseClass::OnFinishedClose();
}

void CPreview::InitColors()
{
	_col_BorderBright = Color( 255,255,255,255 );
	_col_BorderDark = Color( 0,0,0,255 );
}

void CPreview::Paint()
{
	BaseClass::Paint();

	if ( pRenderPanel )
	{
		int x,y,sx,sy;
		pRenderPanel->GetBounds( x, y, sx, sy );
		x -= 1;
		y -= 1;
		sx += 1;
		sy += 1;

		surface()->DrawSetColor( _col_BorderBright );
		surface()->DrawLine( x, y+sy, x+sx, y+sy );
		surface()->DrawLine( x+sx, y, x+sx, y+sy );
		surface()->DrawSetColor( _col_BorderDark );
		surface()->DrawLine( x, y, x+sx, y );
		surface()->DrawLine( x, y, x, y+sy );
	}
}

void CPreview::OnCommand(const char* pcCommand)
{
	if( !Q_stricmp(pcCommand, "loadmodel") )
	{
		OpenBrowser( false );
		return;
	}
	if( !Q_stricmp(pcCommand, "loadbg") )
	{
		OpenBrowser( true );
		return;
	}
	if( !Q_stricmp(pcCommand, "reset_view") )
	{
		pRenderPanel->ResetView();
		return;
	}
	if( !Q_stricmp(pcCommand, "bgcolor") )
	{
		ColorPicker *pPicker = new ColorPicker( pEditorRoot, "cpicker", this );
		Color bgRR = pRenderPanel->GetBgColor();
		pPicker->SetPickerColor( Vector( bgRR.r() / 255.0f, bgRR.g() / 255.0f, bgRR.b() / 255.0f ) );
		return;
	}

	BaseClass::OnCommand( pcCommand );
}

void CPreview::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	InitColors();
	_col_BorderBright = pScheme->GetColor( "Border.Bright", _col_BorderBright );
	_col_BorderDark = pScheme->GetColor( "Border.Dark", _col_BorderDark );
}

void CPreview::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CPreview::RememberPosition()
{
	GetBounds( _px, _py, _sx, _sy );
}
void CPreview::RestorePosition()
{
	SetBounds( _px, _py, _sx, _sy );
}

void CPreview::KillBrowser()
{
	if ( m_hBrowser.Get() )
		m_hBrowser->MarkForDeletion();
}
void CPreview::OpenBrowser( bool bPreviewMaterial )
{
	KillBrowser();

	const char *context = PREVBROWSER_CONTEXT_MODEL;
	const char *dirFormat = "%s/models";
	const char *pExt = "*.mdl";
	const char *pDescription = "Models";
	const char *pFODPCContext = GetFODPathContext( FODPC_MDL );

	if ( bPreviewMaterial )
	{
		context = PREVBROWSER_CONTEXT_BG;
		dirFormat = "%s/materials";
		pExt = "*.vtf";
		pDescription = "Materials";
		pFODPCContext = GetFODPathContext( FODPC_VTF );
	}

	m_hBrowser = new FileOpenDialog( this, "Load .mdl", FOD_OPEN, new KeyValues("FileOpenContext", "context", context) );

	if ( m_hBrowser.Get() )
	{
		char finalPath[MAX_PATH];
		Q_snprintf( finalPath, sizeof( finalPath ), dirFormat, GetGamePath() );

		m_hBrowser->SetStartDirectoryContext( pFODPCContext, finalPath );
		m_hBrowser->AddFilter( pExt, pDescription, true );
		m_hBrowser->DoModal( true );
	}
}
void CPreview::OnFileSelected( KeyValues *pKV )
{
	KeyValues *pContext = pKV->FindKey( "FileOpenContext" );
	if ( pContext )
	{
		const char *context = pContext->GetString("context");
		const char *pathIn = pKV->GetString( "fullpath" );

		const bool bLoadBG = !Q_stricmp( context, PREVBROWSER_CONTEXT_BG );
		if ( bLoadBG )
			pEditorRoot->GetSafeFlowgraph()->SetUsingBackgroundMaterial( false );
		//	pRenderPanel->bRenderBackground = false;

		if ( Q_strlen( pathIn ) <= 1 )
			return;

		char tmp[MAX_PATH];
		if ( !g_pFullFileSystem->FullPathToRelativePath( pathIn, tmp, MAX_PATH ) )
			Q_snprintf( tmp, sizeof(tmp), "%s", pathIn );

		if ( !bLoadBG )
			LoadModel( tmp );
		else
		{
			CHLSL_Image img;
			bool bSuccess = img.LoadFromVTF( tmp );
			Assert( bSuccess );
			if ( !bSuccess )
				return;

			const char *matPath = Q_StripFirstDir( tmp );
			const bool bCubemap = img.IsEnvmap();

			IMaterial *pMat = pEditorRoot->GetBGPreviewMat();
			KeyValues *pKV = pEditorRoot->GetBGPreviewKV();
			if ( !pMat || !pKV )
				return;

			char szNoExtension[MAX_PATH];
			Q_StripExtension( matPath, szNoExtension, MAX_PATH );

			if ( bCubemap )
				SetEnvmap( szNoExtension );
				//Q_strcpy( m_szEnvmap, szNoExtension );
			else
				SetEnvmap( NULL );
				//m_szEnvmap[0] = '\0';

			//SetMaterialVar_Int( pMat, "$ISCUBEMAP", bCubemap ? 1 : 0 );
			//SetMaterialVar_String( pMat, "$BASETEXTURE", szNoExtension );
			//pMat->RecomputeStateSnapshots();

			pKV->SetInt( "$ISCUBEMAP", bCubemap ? 1 : 0 );
			pKV->SetString( "$BASETEXTURE", szNoExtension );
			pMat->Refresh();

			CNodeView *pView = pEditorRoot->GetSafeFlowgraph();
			pView->SetUsingBackgroundMaterial( true );
			pView->MakeSolversDirty();
			//pNodeView->MakeSolversDirty();
		}
	}
}

const char *CPreview::GetEnvmap()
{
	CNodeView *pView = pEditorRoot->GetActiveFlowGraph();
	Assert( pView );
	return pView->GetEnvmap();
}

void CPreview::SetEnvmap( const char *pszPath )
{
	CNodeView *pView = pEditorRoot->GetActiveFlowGraph();
	Assert( pView );
	pView->SetEnvmap( pszPath );
}

void CPreview::OnColorSelected( KeyValues *pKV )
{
	pEditorRoot->GetSafeFlowgraph()->SetUsingBackgroundMaterial( false );
	pRenderPanel->SetBgColor( pKV->GetColor( "color" ) );
}

int CPreview::FindSequenceByName( const char *name, SeqFindMode_t mode )
{
	for ( int i = 0; i < m_iNumSequences; i++ )
	{
		switch ( mode )
		{
		case SEQFIND_EXACT:
			if ( !Q_stricmp( psz_SequenceList[i], name ) )
				return i;
			break;
		case SEQFIND_STARTSWITH:
			if ( Q_stristr( psz_SequenceList[i], name ) == psz_SequenceList[i] )
				return i;
			break;
		case SEQFIND_CONTAINS:
			if ( Q_stristr( psz_SequenceList[i], name ) )
				return i;
			break;
		}
	}
	return -1;
}
void CPreview::LoadModel( const char *localPath )
{
	if ( !sEditMRender )
		return;

	sEditMRender->LoadModel( localPath );

	sEditMRender->DestroyCharPtrList( &psz_SequenceList );
	m_iNumSequences = sEditMRender->QuerySequences( &psz_SequenceList );
	sEditMRender->GetModelCenter( pRenderPanel->render_offset_modelBase.Base() );

	m_pCBox_Model_Sequence->RemoveAll();

	for ( int i = 0; i < m_iNumSequences; i++ )
		m_pCBox_Model_Sequence->AddItem( psz_SequenceList[i], NULL );

	int iIdealSequence = FindSequenceByName( "idle_unarmed", SEQFIND_STARTSWITH );
	if ( iIdealSequence < 0 )
		iIdealSequence = FindSequenceByName( "idle_subtle", SEQFIND_STARTSWITH );
	if ( iIdealSequence < 0 )
		iIdealSequence = FindSequenceByName( "idle", SEQFIND_STARTSWITH );
	if ( iIdealSequence < 0 )
		iIdealSequence = FindSequenceByName( "idle" );

	m_pCBox_Model_Sequence->ActivateItem( max( 0, iIdealSequence ) );
}
