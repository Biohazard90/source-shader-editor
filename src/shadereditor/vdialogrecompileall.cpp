
#include "cbase.h"
#include "editorCommon.h"

enum
{
	_CSTEP_INITIAL = 0,
	_CSTEP_SOLVE_GRAPH,
	_CSTEP_COMPILE_SHADER_WAIT,
	_CSTEP_FINISHED_SUCCESS,
	_CSTEP_FINISHED_ERROR,
	_CSTEP_,
};

DumpCompileHelper_t::DumpCompileHelper_t()
{
	shaderDump = NULL;
	iCompileStep = _CSTEP_INITIAL;
}
DumpCompileHelper_t::~DumpCompileHelper_t()
{
	delete shaderDump;
}

#ifdef COLOR_GREY
#undef COLOR_GREY
#endif

#define DEFAULT_LOG_COLOR Color( 255,255,255,255 )
#define ERROR_LOG_COLOR Color( 255,64,64,255 )
#define ERROR_LOG_COLOR_LIGHT Color( 255,96,96,255 )
#define ERROR_LOG_COLOR_DARK Color( 255,32,32,255 )
#define SUCCESS_LOG_COLOR Color( 64,255,64,255 )
#define COLOR_WHITE Color(255,255,255,255)
#define COLOR_GREY Color(164,164,164,255)
#define COLOR_INVIS Color(0,0,0,0)

CDialogRecompileAll::CDialogRecompileAll( vgui::Panel *parent ) : BaseClass( parent, NULL, "diag_recompileall" )
{
	m_pRichText_Log = new RichText( this, "rtext_log" );
	m_pClose = new Button( this, "button_close", "", this, "close" );
	m_pProgressPos = new Panel( this, "progress_pos" );

	LoadControlSettings("shadereditorui/vgui/shadereditor_diag_recompileall.res");

	SetButtonState( false );

	SetTitle("Recompile all",true);

	//SetSizeable(false);

	int sx, sy;
	surface()->GetScreenSize( sx, sy );
	SetSize( 560, sy * 0.7f );

	CUtlVector<BasicShaderCfg_t*> hDumpList;
	ReadGameShaders( hDumpList );

	for ( int i = 0; i < hDumpList.Count(); i++ )
	{
		DumpCompileHelper_t *pC = new DumpCompileHelper_t();
		pC->shaderDump = hDumpList[i];
		hCompileList.AddToTail( pC );
	}

	hDumpList.Purge();

	m_pView = new CNodeView( NULL, pEditorRoot, "compileallhelper" );

	LogMsg( "Found %i shaders marked for precaching:\n", hCompileList.Count() );

	m_bCompilerRunning = true;

	char tmp[MAX_PATH*4];
	for ( int i = 0; i < hCompileList.Count(); i++ )
	{
		BasicShaderCfg_t *p = hCompileList[i]->shaderDump;
		if ( !p->Filename || !p->Filename )
			continue;

		for ( int n = 0; n < pEditorRoot->GetNumFlowGraphs(); n++ )
		{
			CNodeView *pV = pEditorRoot->GetFlowGraph(n);

			if ( pV->GetFlowgraphType() != CNodeView::FLOWGRAPH_HLSL )
				continue;

			if ( !pV->GetShadername() || !*pV->GetShadername() )
				continue;

			if ( !g_pFullFileSystem->FullPathToRelativePath( pV->GetShadername(), tmp, sizeof( tmp ) ) )
				Q_snprintf( tmp, sizeof( tmp ), "%s", pV->GetShadername() );
			Q_FixSlashes( tmp );

			if ( Q_stricmp( tmp, p->Filename ) )
				continue;

			if ( !pV->IsSaveDirty() )
				continue;

			m_bCompilerRunning = false;
			break;
		}
	}

	DoModal();

	if ( !m_bCompilerRunning )
	{
		PromptSimple *prompt = new PromptSimple( this, "Warning" );
		prompt->MoveToCenterOfScreen();
		prompt->SetText( "It appears that you have unsaved changes in some shaders which are selected for precaching. Save them now?" );
		prompt->AddButton( "Yes", "continuecompile_dosave" );
		prompt->AddButton( "No", "continuecompile" );
		prompt->InvalidateLayout( true, true );
	}

	m_bAutoInjectOld = pEditorRoot->m_bAutoShaderPublish;
	pEditorRoot->m_bAutoShaderPublish = true;

	SetDeleteSelfOnClose( true );
}

CDialogRecompileAll::~CDialogRecompileAll()
{
	pEditorRoot->m_bAutoShaderPublish = m_bAutoInjectOld;
	delete m_pView;
	hCompileList.PurgeAndDeleteElements();
}

void CDialogRecompileAll::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( "continuecompile_dosave", cmd ) )
	{
		pEditorRoot->SaveAllGraphs();
		m_bCompilerRunning = true;

		InvalidateLayout();
	}
	else if ( !Q_stricmp( "continuecompile", cmd ) )
	{
		m_bCompilerRunning = true;

		InvalidateLayout();
	}
	else if ( !Q_stricmp( "close", cmd ) )
	{
		if ( m_bCompilerRunning )
		{
			m_bCompilerRunning = false;

			InvalidateLayout();

			ForceTerminateCompilers();
			LogMsg( ERROR_LOG_COLOR, "\n\nCOMPILER TERMINATED\n" );
		}
		else
		{
			DeletePanel();
		}
	}
}

void CDialogRecompileAll::SetButtonState( bool bShowClose )
{
	if ( bShowClose )
	{
		m_pClose->SetText( "Close" );
		m_pClose->InvalidateLayout( true, true );
	}
	else
	{
		m_pClose->SetText( "Terminate" );
		m_pClose->SetDefaultColor( COLOR_WHITE, ERROR_LOG_COLOR );
		m_pClose->SetArmedColor( COLOR_WHITE, ERROR_LOG_COLOR_LIGHT );
		m_pClose->SetDepressedColor( COLOR_WHITE, ERROR_LOG_COLOR_DARK );
		//m_pClose->SetPaintBackgroundEnabled( true );
	}
}

void CDialogRecompileAll::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pRichText_Log->SetBorder( pScheme->GetBorder("DepressedButtonBorder") );
	m_pRichText_Log->SetVerticalScrollbar( true );
	m_pRichText_Log->SetUnusedScrollbarInvisible( false );
	m_pRichText_Log->SetPanelInteractive( true );
	m_pRichText_Log->InsertColorChange( DEFAULT_LOG_COLOR );
}

void CDialogRecompileAll::PerformLayout()
{
	BaseClass::PerformLayout();

	SetButtonState( !m_bCompilerRunning );
	m_pProgressPos->SetPaintBackgroundEnabled( false );

	SetDeleteSelfOnClose( true );
}

void CDialogRecompileAll::Paint()
{
	BaseClass::Paint();

	int x,y,sx,sy;
	m_pProgressPos->GetBounds( x, y, sx, sy );
	const int iExtrude = 2;
	const int iThick = 3;

	surface()->DrawSetColor( COLOR_GREY );
	surface()->DrawFilledRect( x - iExtrude - iThick,
		y - iExtrude - iThick,
		x + sx + iExtrude + iThick,
		y - iExtrude );
	surface()->DrawFilledRect( x - iExtrude - iThick,
		y + sy + iExtrude,
		x + sx + iExtrude + iThick,
		y + sy + iExtrude + iThick );
	surface()->DrawFilledRect( x - iExtrude - iThick,
		y - iExtrude,
		x - iExtrude,
		y + sy + iExtrude );
	surface()->DrawFilledRect( x + sx + iExtrude,
		y - iExtrude,
		x + sx + iExtrude + iThick,
		y + sy + iExtrude );

	if ( hCompileList.Count() > 0 )
	{
		//const int iStep = sx / (float)hCompileList.Count();

		const float flStep = sx / (float)hCompileList.Count();
		int iLastX1 = x;

		for ( int i = 0; i < hCompileList.Count(); i++ )
		{
			const int cur = hCompileList[i]->iCompileStep;
			bool bStop = false;

			switch ( cur )
			{
			case _CSTEP_FINISHED_ERROR:
				surface()->DrawSetColor( ERROR_LOG_COLOR_DARK );
				break;
			case _CSTEP_FINISHED_SUCCESS:
				surface()->DrawSetColor( COLOR_GREY );
				break;
			default:
				bStop = true;
				break;
			}

			if ( bStop )
				break;

			//const int lx0 = x + iStep * i;
			//const int lx1 = (i < hCompileList.Count() - 1) ? lx0 + iStep : x + sx;

			const int lx0 = iLastX1;
			const int lx1 = (i < hCompileList.Count() - 1) ? lx0 + flStep : x + sx;
			iLastX1 = lx1;

			surface()->DrawFilledRect( lx0, y, lx1, y + sy );
		}
	}
}

void CDialogRecompileAll::OnThink()
{
	if ( m_bCompilerRunning )
	{
		int cur = -1;
		for ( int i = 0; i < hCompileList.Count() && cur < 0; i++ )
		{
			if ( hCompileList[i]->iCompileStep < _CSTEP_FINISHED_SUCCESS )
				cur = i;
		}

		if ( cur < 0 )
		{
			m_bCompilerRunning = false;
			LogMsg( "\nFinished. Processed %i dumps", hCompileList.Count() );

			int success = 0;
			int error = 0;

			for ( int i = 0; i < hCompileList.Count(); i++ )
			{
				if ( hCompileList[i]->iCompileStep == _CSTEP_FINISHED_SUCCESS )
					success++;
				else
					error++;
			}

			if ( success != 0 || error != 0 )
			{
				LogMsg( " - %i successfully compiled, ", success );
				LogMsg( ((error>0) ? ERROR_LOG_COLOR : DEFAULT_LOG_COLOR), "%i errored", error );
				LogMsg( "." );
			}

			LogMsg( "\n" );

			InvalidateLayout();
			return;
		}

		Assert( cur >= 0 && cur < hCompileList.Count() );
		DumpCompileHelper_t *cmd = hCompileList[cur];

		switch ( cmd->iCompileStep )
		{
		case _CSTEP_INITIAL:
			{
				LogMsg( "\nProcessing %s:\n", ( cmd->shaderDump->CanvasName ? cmd->shaderDump->CanvasName : "MISSING_CANVAS_NAME" ) );

				LogMsg( "Loading file... " );
				if ( !cmd->shaderDump->Filename || !*cmd->shaderDump->Filename )
				{
					LogMsg( ERROR_LOG_COLOR, "ERROR (no filepath in dump -> try manual full compile!)" );
					LogMsg( ".\n" );
					cmd->iCompileStep = _CSTEP_FINISHED_ERROR;
				}
				else if ( !m_pView->LoadFromFile( cmd->shaderDump->Filename, true ) )
				{
					LogMsg( ERROR_LOG_COLOR, "ERROR (unable to open file)" );
					LogMsg( ".\n" );
					cmd->iCompileStep = _CSTEP_FINISHED_ERROR;
				}
				else
				{
					m_pView->SetShadername( cmd->shaderDump->Filename );

					LogMsg( SUCCESS_LOG_COLOR, "finished" );
					LogMsg( ".\n" );
					cmd->iCompileStep = _CSTEP_SOLVE_GRAPH;
				}
			}
			break;
		case _CSTEP_SOLVE_GRAPH:
			{
				LogMsg( "Solving graph... " );

				if ( m_pView->GetFlowgraphType() != CNodeView::FLOWGRAPH_HLSL )
				{
					cmd->iCompileStep = _CSTEP_FINISHED_ERROR;
					LogMsg( ERROR_LOG_COLOR, "ERROR (graph is not a compilable shader!)" );
					LogMsg( ".\n" );
				}
				else
				{
					m_pView->UpdateSolvers();

					CBaseNode *pVSOut = m_pView->GetNodeFromType( HLSLNODE_VS_OUT );
					CBaseNode *pPSOut = m_pView->GetNodeFromType( HLSLNODE_PS_OUT );

					if ( !pVSOut || !pPSOut ||
						pVSOut->GetErrorLevel() != ERRORLEVEL_NONE || pPSOut->GetErrorLevel() != ERRORLEVEL_NONE )
					{
						cmd->iCompileStep = _CSTEP_FINISHED_ERROR;
						LogMsg( ERROR_LOG_COLOR, "ERROR (graph not solvable, go check for errors)" );
						LogMsg( ".\n" );
					}
					else
					{
						LogMsg( SUCCESS_LOG_COLOR, "finished" );
						LogMsg( ".\n" );

						LogMsg( "Running compiler... " );
						cmd->iCompileStep = _CSTEP_COMPILE_SHADER_WAIT;
					}
				}
			}
			break;
		case _CSTEP_COMPILE_SHADER_WAIT:
			{
				if ( m_pView->GetCompiledStackIndex() == m_pView->GetStackIndex() )
				{
					if ( m_pView->IsLastCompileSuccessful() )
					{
						LogMsg( SUCCESS_LOG_COLOR, "finished" );
						LogMsg( ".\n" );
						cmd->iCompileStep = _CSTEP_FINISHED_SUCCESS;
					}
					else
					{
						LogMsg( ERROR_LOG_COLOR, "ERROR" );
						LogMsg( ".\n" );
						cmd->iCompileStep = _CSTEP_FINISHED_ERROR;
					}
				}
			}
			break;
		}
	}
}

static char __string[1024];

void CDialogRecompileAll::LogMsg( const char *msg, ... )
{
	va_list		argptr;
	va_start (argptr, msg);
	Q_vsnprintf (__string, sizeof( __string ), msg, argptr);
	va_end (argptr);

	m_pRichText_Log->InsertString( __string );
}
void CDialogRecompileAll::LogMsg( Color textCol, const char *msg, ... )
{
	m_pRichText_Log->InsertColorChange( textCol );

	va_list		argptr;
	va_start (argptr, msg);
	Q_vsnprintf (__string, sizeof( __string ), msg, argptr);
	va_end (argptr);

	m_pRichText_Log->InsertString( __string );

	m_pRichText_Log->InsertColorChange( DEFAULT_LOG_COLOR );
}