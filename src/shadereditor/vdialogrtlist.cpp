
#include "cbase.h"
#include "editorCommon.h"

CDialog_RendertargetList::CDialog_RendertargetList( Panel *parent ) : CBaseDiag( parent, NULL, "_rt_list" )
{
	SetSizeable( false );
	SetVisible( true );

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	Activate();



	m_pList_RT = new PanelListPanel( this, "rtlist" );
	m_pList_RT->SetNumColumns( 2 );
	m_pList_RT->SetFirstColumnWidth( 200 );

	FillList();


	LoadControlSettings( "shadereditorui/vgui/dialog_rendertarget_list.res" );

	SetTitle( "Rendertarget manager", true );

	DoModal();
	SetDeleteSelfOnClose( true ); 
	MoveToCenterOfScreen();
}

CDialog_RendertargetList::~CDialog_RendertargetList()
{
}

void CDialog_RendertargetList::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "new_rt" ) )
	{
		CDialog_RendertargetSettings *pEditDiag = new CDialog_RendertargetSettings( this );
		pEditDiag->AddActionSignalTarget( this );
		pEditDiag->SetDefaults();
	}
	else if ( Q_stristr( cmd, "del_rt_" ) == cmd )
	{
		const char *pszNum = cmd + 7;
		int index = atoi( pszNum );

		Assert( index >= 0 && index < GetRTManager()->GetNumRTs() );

		GetRTManager()->GetRT( index )->FlagForDeletion();

		FillList();
	}
	else if ( Q_stristr( cmd, "edit_rt_" ) == cmd )
	{
		const char *pszNum = cmd + 8;
		int index = atoi( pszNum );

		Assert( index >= 0 && index < GetRTManager()->GetNumRTs() );

		CDialog_RendertargetSettings *pEditDiag = new CDialog_RendertargetSettings( this );
		pEditDiag->AddActionSignalTarget( this );
		pEditDiag->InitFromRT( GetRTManager()->GetRT( index ) );
	}
	else if ( !Q_stricmp( cmd, "apply" ) )
	{
		GetRTManager()->UpdateRTsToMatSys();
		GetRTManager()->SaveRTsToFile();
	}
	else
	{
		GetRTManager()->UpdateRTsToMatSys();
		GetRTManager()->SaveRTsToFile();

		BaseClass::OnCommand( cmd );
	}
}

void CDialog_RendertargetList::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CDialog_RendertargetList::FillList()
{
	ScrollBar *pScroll = m_pList_RT->GetScrollbar();
	int scrollValue = pScroll ? pScroll->GetValue() : 0;

	m_pList_RT->DeleteAllItems();

	GetRTManager()->SortRTsByName();
	for ( int i = 0; i < GetRTManager()->GetNumRTs(); i++ )
	{
		RTDef *rt = GetRTManager()->GetRT( i );

		if ( rt->IsFlaggedForDeletion() )
			continue;

		Label *pL = new Label( m_pList_RT, "", rt->GetName() );
		Button *pBDel = new Button( m_pList_RT, "", "Delete", this, VarArgs("del_rt_%i",i) );
		Button *pBEdit = new Button( m_pList_RT, "", "Edit", this, VarArgs("edit_rt_%i",i) );

		pBDel->SetContentAlignment( Label::a_center );
		pBEdit->SetContentAlignment( Label::a_center );

		m_pList_RT->AddItem( pL, pBDel );
		m_pList_RT->AddItem( NULL, pBEdit );
	}

	if ( pScroll )
		pScroll->SetValue( scrollValue );
}
void CDialog_RendertargetList::OnUpdateList()
{
	FillList();
}


// quick access to files with all these constants
#include "bitmap/imageformat.h"
#include "vtf/vtf.h"
#include "materialsystem/imaterialsystem.h"

static const char *pszImgFormatNames[] = {
	"RGBA8888", // 0
	"ABGR8888",
	"RGB888",
	"BGR888",
	"RGB565",
	"I8",
	"IA88",
	"P8",
	"A8",
	"RGB888_BLUESCREEN",
	"BGR888_BLUESCREEN",
	"ARGB8888",
	"BGRA8888",
	"DXT1",
	"DXT3",
	"DXT5",
	"BGRX8888",
	"BGR565",
	"BGRX5551",
	"BGRA4444",
	"DXT1_ONEBITALPHA", // 20
	"BGRA5551",
	"UV88",
	"UVWQ8888",
	"RGBA16161616F",
	"RGBA16161616",
	"UVLX8888",
	"R32F",
	"RGB323232F",
	"RGBA32323232F", // 29

#ifdef SHADER_EDITOR_DLL_SWARM
	"RG1616F",
	"RG3232F",
	"RGBX8888",
#endif
};
static const int iNumImgFormatNames = ARRAYSIZE( pszImgFormatNames );


const char *pszRTSizeModes[] = {
	"NO_CHANGE", // 0
	"DEFAULT (<= FB)",
	"PICMIP",
	"HDR (QUARTER FB)",
	"FULL_FRAME_BUFFER (ROUND DOWN)",
	"OFFSCREEN",
	"FULL_FRAME_BUFFER (ROUND UP)",
#ifdef SHADER_EDITOR_DLL_2013
	"REPLAY SCREENSHOT",
#endif
	"HALF FRAME BUFFER",
};
static const int iNumRTSizeModes = ARRAYSIZE( pszRTSizeModes );


const char *pszRTDepthModes[] = {
	"SHARED",
	"SEPARATE",
	"NONE",
	"ONLY",
};
static const int iNumRTDepthModes = ARRAYSIZE( pszRTDepthModes );

const char *pszVTFFlags[] = {
	"POINTSAMPLE",
	"TRILINEAR",
	"CLAMPS",
	"CLAMPT",
	"ANISOTROPIC",
	"HINT_DXT5",
#ifdef SHADER_EDITOR_DLL_SWARM
	"PWL_CORRECTED",
#else
	"SRGB",
#endif
	"NORMAL",
	"NOMIP",
	"NOLOD",
	"ALL_MIPS",
	"PROCEDURAL",
	"ONEBITALPHA",
	"EIGHTBITALPHA",
	"ENVMAP",
	"RENDERTARGET",
	"DEPTHRENDERTARGET",
	"NODEBUGOVERRIDE",
	"SINGLECOPY",
#ifdef SHADER_EDITOR_DLL_SWARM
	"PRE_SRGB",
#else
	"UNUSED_00080000",
#endif
	"UNUSED_00100000",
	"UNUSED_00200000",
	"UNUSED_00400000",
	"NODEPTHBUFFER",
	"UNUSED_01000000",
	"CLAMPU",
	"VERTEXTEXTURE",
	"SSBUMP",
	"UNUSED_10000000",
	"BORDER",
	"UNUSED_40000000",
	"UNUSED_80000000",
};
static const int iNumVTFFlags = ARRAYSIZE( pszVTFFlags );


CDialog_RendertargetSettings::CDialog_RendertargetSettings( Panel *parent ) : CBaseDiag( parent, NULL, "rtsettings" ) //BaseClass( parent, "rtsettings" )
{
	m_pRTTarget = NULL;

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );

	m_pText_RTName = new TextEntry( this, "rt_name" );

	m_pText_SizeX = new TextEntry( this, "rt_size_x" );
	m_pText_SizeY = new TextEntry( this, "rt_size_y" );

	m_pList_Flags = new PanelListPanel( this, "rtlist" );
	m_pList_Flags->SetNumColumns( 1 );
	m_pList_Flags->SetFirstColumnWidth( 0 );
	m_pList_Flags->SetVerticalBufferPixels( 0 );

	for ( int i = 0; i < iNumVTFFlags; i++ )
	{
		CheckButton *pCBut = new CheckButton( m_pList_Flags, "", pszVTFFlags[i] );
		m_pList_Flags->AddItem( NULL, pCBut );
		m_hFlag_Buttons.AddToTail( pCBut );
	}

	m_pCBox_ImgFormat = new ComboBox( this, "cbox_format", 20, false );
	for ( int i = 0; i < iNumImgFormatNames; i++ )
		m_pCBox_ImgFormat->AddItem( pszImgFormatNames[i], NULL );

	m_pCBox_SizeMode = new ComboBox( this, "cbox_sizemode", 20, false );
	for ( int i = 0; i < iNumRTSizeModes; i++ )
		m_pCBox_SizeMode->AddItem( pszRTSizeModes[i], NULL );

	m_pCBox_DepthMode = new ComboBox( this, "cbox_depthmode", 20, false );
	for ( int i = 0; i < iNumRTDepthModes; i++ )
		m_pCBox_DepthMode->AddItem( pszRTDepthModes[i], NULL );

	LoadControlSettings( "shadereditorui/vgui/dialog_rendertarget_settings.res" );

	SetSizeable( false );
	SetTitle( "Rendertarget properties", true );

	SetCloseButtonVisible( false );

	DoModal();
	MoveToCenterOfScreen();
}

CDialog_RendertargetSettings::~CDialog_RendertargetSettings()
{
	m_hFlag_Buttons.Purge();
}

bool CDialog_RendertargetSettings::DoesRTExist( const char *name )
{
	if ( !name || !*name )
		return false;

	ITexture *pTex = materials->FindTexture( name, TEXTURE_GROUP_OTHER, false );
	RTDef *rt = GetRTManager()->FindRTByName( name, true );

	if ( rt && rt->IsFlaggedForDeletion() )
		return false;

	return rt != NULL ||
		!IsErrorTexture( pTex );
}

void CDialog_RendertargetSettings::ShowErrorDiag( const char *name )
{
	bool bEngineTex = GetRTManager()->FindRTByName( name, true ) == NULL;

	PromptSimple *prompt = new PromptSimple( this, "Error" );
	prompt->MoveToCenterOfScreen();
	if ( !bEngineTex )
	{
		prompt->SetText( "Rendertarget of specified name exists already, override?" );
		prompt->AddButton( "Ok", "onerror_override" );
		prompt->AddButton( "Cancel", "onerror_cancel" );
	}
	else
	{
		prompt->SetText( VarArgs( "'%s' is a texture defined elsewhere, you can't use this name!", name ) );
		prompt->AddButton( "Cancel", "onerror_cancel" );
	}
}

void CDialog_RendertargetSettings::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "Ok" ) )
	{
		if ( m_pText_RTName->GetTextLength() < 1 )
		{
			PromptSimple *prompt = new PromptSimple( this, "Error" );
			prompt->MoveToCenterOfScreen();
			prompt->SetText( "You must specify a name for the RT!" );
			prompt->AddButton( "Ok" );
			return;
		}

		if ( m_pRTTarget != NULL )
		{
			RTDef tmpRT = *m_pRTTarget;
			WriteToRT( &tmpRT );

			RTDef *pFound = GetRTManager()->FindRTByName( tmpRT.GetName(), true );
			if ( pFound != NULL && pFound != m_pRTTarget )
			{
				ShowErrorDiag( tmpRT.GetName() );
				return;
			}

			m_pRTTarget->Modify( tmpRT );
		}
		else
		{
			RTDef newRT;
			WriteToRT( &newRT );

			int oldIndex = GetRTManager()->FindRTIndexByName(newRT.GetName());

			if ( oldIndex >= 0 && GetRTManager()->GetRT( oldIndex )->IsFlaggedForDeletion() )
			{
				GetRTManager()->RemoveRT( oldIndex );
				GetRTManager()->AddRT( newRT );
			}
			else if ( DoesRTExist( newRT.GetName() ) )
			{
				ShowErrorDiag( newRT.GetName() );
				return;
			}
			else
				GetRTManager()->AddRT( newRT );
		}

		CloseModal();
	}
	else if ( !Q_stricmp( cmd, "Close" ) )
	{
		CloseModal();
	}
	else if ( !Q_stricmp( cmd, "onerror_override" ) )
	{
		RTDef newRT;
		WriteToRT( &newRT );

		for ( int i = 0; i < GetRTManager()->GetNumRTs(); i++ )
		{
			RTDef *pold = GetRTManager()->GetRT( i );
			if ( Q_stricmp( pold->GetName(), newRT.GetName() ) )
				continue;
			pold->FlagForDeletion();
		}

		if ( m_pRTTarget != NULL )
			m_pRTTarget->Modify( newRT );
		else
			GetRTManager()->AddRT( newRT );

		CloseModal();
	}
	else if ( !Q_stricmp( cmd, "onerror_cancel" ) )
	{
	}
}

void CDialog_RendertargetSettings::CloseModal()
{
	PostActionSignal( new KeyValues( "UpdateList" ) );
	BaseClass::CloseModal();
}

void CDialog_RendertargetSettings::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CDialog_RendertargetSettings::SetDefaults()
{
	m_pText_RTName->SetText( "" );
	m_pText_SizeX->SetText( VarArgs( "%i", RT_DEFAULT_SIZE ) );
	m_pText_SizeY->SetText( VarArgs( "%i", RT_DEFAULT_SIZE ) );

	m_pCBox_ImgFormat->ActivateItem( 0 );
	m_pCBox_SizeMode->ActivateItem( 0 );
	m_pCBox_DepthMode->ActivateItem( 2 );

	for ( int i = 0; i < m_hFlag_Buttons.Count(); i++ )
	{
		m_hFlag_Buttons[i]->SetSelected( (TEXTUREFLAGS_RENDERTARGET & ( 1 << i )) != 0 );
	}
}

void CDialog_RendertargetSettings::InitFromRT( RTDef *rt )
{
	m_pRTTarget = rt;

	m_pText_RTName->SetText( rt->GetName() );
	m_pText_SizeX->SetText( VarArgs( "%i", rt->GetSizeX() ) );
	m_pText_SizeY->SetText( VarArgs( "%i", rt->GetSizeY() ) );

	m_pCBox_ImgFormat->ActivateItem( rt->GetImgFormat() );
	m_pCBox_SizeMode->ActivateItem( rt->GetSizeMode() );
	m_pCBox_DepthMode->ActivateItem( rt->GetDepthMode() );

	unsigned int flags = rt->GetFlags();
	Assert( m_hFlag_Buttons.Count() == iNumVTFFlags );

	for ( int i = 0; i < m_hFlag_Buttons.Count(); i++ )
	{
		m_hFlag_Buttons[i]->SetSelected( !!(flags & ( 1 << i )) );
	}
}

void CDialog_RendertargetSettings::WriteToRT( RTDef *rt )
{
	char name_dirty[MAX_PATH];
	char name[MAX_PATH];
	char tmp[MAX_PATH];

	m_pText_RTName->GetText( name_dirty, sizeof(name_dirty) );
	CleanupString( name_dirty, name, sizeof( name ) );

	if ( Q_strlen( name ) < 1 )
		Q_snprintf( name, sizeof( name ), "_rt_unnamed" );

	m_pText_SizeX->GetText( tmp, sizeof(tmp) );
	int x = atoi( tmp );
	m_pText_SizeY->GetText( tmp, sizeof(tmp) );
	int y = atoi( tmp );

	if ( x < 1 )
		x = RT_DEFAULT_SIZE;
	if ( y < 1 )
		y = RT_DEFAULT_SIZE;

	x = min( RT_MAX_SIZE, x );
	y = min( RT_MAX_SIZE, y );

	unsigned int flags = 0;
	for ( int i = 0; i < m_hFlag_Buttons.Count(); i++ )
		if ( m_hFlag_Buttons[i]->IsSelected() )
			flags |= ( 1 << i );

	ImageFormat f = (ImageFormat)m_pCBox_ImgFormat->GetActiveItem();
	RenderTargetSizeMode_t sm = (RenderTargetSizeMode_t)m_pCBox_SizeMode->GetActiveItem();
	MaterialRenderTargetDepth_t dm = (MaterialRenderTargetDepth_t)m_pCBox_DepthMode->GetActiveItem();

	rt->Modify( (Q_strlen(name)) ? name : NULL,
		&f, &sm, &dm,
		&x, &y, &flags );
}

void CDialog_RendertargetSettings::OnTextChanged( KeyValues *pKV )
{
	Panel *p = (Panel*)pKV->GetPtr("panel");

	if ( p == m_pCBox_SizeMode )
	{
		const int sizemode = m_pCBox_SizeMode->GetActiveItem();
		const bool bEnableSizing = sizemode != RT_SIZE_HALF_FRAME_BUFFER &&
			sizemode != RT_SIZE_HDR &&
			sizemode != RT_SIZE_FULL_FRAME_BUFFER &&
			sizemode != RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP;

		m_pText_SizeX->SetEnabled( bEnableSizing );
		m_pText_SizeY->SetEnabled( bEnableSizing );
	}
}