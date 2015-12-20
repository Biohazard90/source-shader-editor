
#include <assert.h>
#include <math.h>
#define PROTECTED_THINGS_DISABLE

#include "tier1/utlstring.h"
#include "vgui/Cursor.h"
#include "vgui/MouseCode.h"
#include "vgui/IBorder.h"
#include "vgui/IInput.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"
#include "vgui/KeyCode.h"

#include "vgui_controls/animationcontroller.h"
#include "vgui_controls/controls.h"
#include "vgui_controls/frame.h"
#include "vgui_controls/button.h"
#include "vgui_controls/menu.h"
#include "vgui_controls/menubutton.h"
#include "vgui_controls/textimage.h"
#include "vgui_controls/colorpicker.h"

#include "KeyValues.h"

#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

#include <stdio.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

HSV_Select_Base::HSV_Select_Base( vgui::Panel *parent, const char *pElementName ) : BaseClass( parent, pElementName )
{
	if ( parent )
		AddActionSignalTarget( parent->GetVPanel() );
	m_bIsReading = false;
	SetupVguiTex( m_iMat, "shadereditor/colorpicker" );
}
void HSV_Select_Base::OnMousePressed( MouseCode code )
{
	BaseClass::OnMousePressed( code );
	if ( code == MOUSE_LEFT )
	{
		m_bIsReading = true;
		input()->SetMouseCapture( GetVPanel() );
		ReadValues();
		return;
	}
}
void HSV_Select_Base::OnMouseReleased( MouseCode code )
{
	BaseClass::OnMouseReleased( code );
	if ( code == MOUSE_LEFT && m_bIsReading )
	{
		m_bIsReading = false;
		input()->SetMouseCapture( 0 );
		return;
	}
}
void HSV_Select_Base::OnMouseCaptureLost()
{
	BaseClass::OnMouseCaptureLost();
	m_bIsReading = false;
}
void HSV_Select_Base::OnCursorMoved( int x, int y )
{
	BaseClass::OnCursorMoved( x, y );
	if ( m_bIsReading )
		ReadValues();
}
void HSV_Select_Base::ReadValues()
{
	PostActionSignal( new KeyValues( "HSVUpdate" ) );
}



HSV_Select_SV::HSV_Select_SV( vgui::Panel *parent, const char *pElementName ) : BaseClass( parent, pElementName )
{
	m_flH = 0;
	m_flS = 0;
	m_flV = 0;
}
void HSV_Select_SV::Paint()
{
	surface()->DrawSetTexture( m_iMat );
	surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
	int x, y, sx, sy;
	GetBounds( x, y, sx, sy );

	Vertex_t points[4];
	points[0].m_TexCoord.Init( 0, 0 );
	points[1].m_TexCoord.Init( 1, 0 );
	points[2].m_TexCoord.Init( 1, 1 );
	points[3].m_TexCoord.Init( 0, 1 );


	IMaterial *pMatColorpicker = materials->FindMaterial( "shadereditor/colorpicker", TEXTURE_GROUP_OTHER );
	if ( IsErrorMaterial( pMatColorpicker ) )
		return;
	bool bFound = false;
	IMaterialVar *pVar_00 = pMatColorpicker->FindVar( "$COLOR_00", &bFound );
	IMaterialVar *pVar_10 = pMatColorpicker->FindVar( "$COLOR_10", &bFound );
	IMaterialVar *pVar_11 = pMatColorpicker->FindVar( "$COLOR_11", &bFound );
	IMaterialVar *pVar_01 = pMatColorpicker->FindVar( "$COLOR_01", &bFound );
	if ( !bFound )
		return;

	Vector col;
	HSV2RGB( m_flH, 1, 1, col );

	pVar_00->SetVecValue( 1, 1, 1 );
	pVar_10->SetVecValue( col.Base(), 3 );
	pVar_11->SetVecValue( 0, 0, 0 );
	pVar_01->SetVecValue( 0, 0, 0 );

	surface()->DrawTexturedRect( 0, 0, sx, sy );
}
void HSV_Select_SV::ReadValues()
{
	int mx, my;
	input()->GetCursorPosition( mx, my );
	ScreenToLocal( mx, my );

	int sx, sy;
	GetSize( sx, sy );

	int vpos = clamp( sy - my, 0, sy );
	m_flV = vpos / (float) sy;

	int spos = sx - clamp( sx - mx, 0, sx );
	m_flS = spos / (float) sx;

	BaseClass::ReadValues();
}





HSV_Select_Hue::HSV_Select_Hue( vgui::Panel *parent, const char *pElementName ) : BaseClass( parent, pElementName )
{
	m_flHue = 0;
}
void HSV_Select_Hue::Paint()
{
	surface()->DrawSetTexture( m_iMat );
	surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
	int x, y, sx, sy;
	GetBounds( x, y, sx, sy );

	Vector rgbs[7];
	for ( int i = 0; i < 7; i++ )
		HSV2RGB( i * 60.0f, 1, 1, rgbs[i] );

	Vertex_t points[4];
	points[0].m_TexCoord.Init( 0, 0 );
	points[1].m_TexCoord.Init( 1, 0 );
	points[2].m_TexCoord.Init( 1, 1 );
	points[3].m_TexCoord.Init( 0, 1 );


	IMaterial *pMatColorpicker = materials->FindMaterial( "shadereditor/colorpicker", TEXTURE_GROUP_OTHER );
	if ( IsErrorMaterial( pMatColorpicker ) )
		return;
	bool bFound = false;
	IMaterialVar *pVar_00 = pMatColorpicker->FindVar( "$COLOR_00", &bFound );
	IMaterialVar *pVar_10 = pMatColorpicker->FindVar( "$COLOR_10", &bFound );
	IMaterialVar *pVar_11 = pMatColorpicker->FindVar( "$COLOR_11", &bFound );
	IMaterialVar *pVar_01 = pMatColorpicker->FindVar( "$COLOR_01", &bFound );
	if ( !bFound )
		return;

	int delta_y = sy / 6;
	int cur_pos_y = sy;

	for ( int i = 0; i < 6; i++ )
	{
		if ( i == 5 )
			delta_y = cur_pos_y;

		pVar_00->SetVecValue( rgbs[ i + 1 ].Base(), 3 );
		pVar_10->SetVecValue( rgbs[ i + 1 ].Base(), 3 );
		pVar_11->SetVecValue( rgbs[ i ].Base(), 3 );
		pVar_01->SetVecValue( rgbs[ i ].Base(), 3 );

		points[0].m_Position.Init( 0, cur_pos_y - delta_y );
		points[1].m_Position.Init( sx, cur_pos_y - delta_y );
		points[2].m_Position.Init( sx, cur_pos_y );
		points[3].m_Position.Init( 0, cur_pos_y );

		surface()->DrawTexturedPolygon( 4, points );
		cur_pos_y -= delta_y;
	}
}
void HSV_Select_Hue::ReadValues()
{
	int mx, my;
	input()->GetCursorPosition( mx, my );
	ScreenToLocal( mx, my );

	int sx, sy;
	GetSize( sx, sy );

	int huepos = clamp( sy - my, 0, sy );
	m_flHue = (huepos / (float) sy) * 360.0f;
	m_flHue = clamp( m_flHue, 0, 360.0f );

	BaseClass::ReadValues();
}




class PickerHelper : public Panel
{
public:
	DECLARE_CLASS_SIMPLE( PickerHelper, Panel );
	PickerHelper( int img, Panel *parent ) : Panel(parent, "" )
	{
		m_iImage = img;
		this->SetMouseInputEnabled( false );
		this->SetKeyBoardInputEnabled( false );
		this->SetPaintBackgroundEnabled( false );
	};

	int m_iImage;

	void Paint()
	{
		surface()->DrawSetColor( Color( 255,255,255,255) );
		surface()->DrawSetTexture( m_iImage );
		int sx, sy;
		GetSize( sx, sy );
		surface()->DrawTexturedRect( 0, 0, sx, sy );
	};
	void OnThink()
	{
		SetZPos( 100 );
	};
	void SetCenter( int x, int y )
	{
		int sx, sy;
		GetSize( sx, sy );
		SetPos( x - sx / 2, y - sy / 2 );
	};
};


ColorPicker::ColorPicker( vgui::Panel *parent, const char *pElementName, Panel *pActionsignalTarget ) : BaseClass( parent, pElementName )
{
	pTarget = NULL;
	AddActionSignalTarget( pActionsignalTarget );

	Init();
}
ColorPicker::ColorPicker( vgui::Panel *parent, const char *pElementName, TextEntry *pTargetEntry ) : BaseClass( parent, pElementName )
{
	pTarget = pTargetEntry;

	Init();
}

ColorPicker::~ColorPicker()
{
}

void ColorPicker::Init()
{
	m_vecColor.Init();
	m_vecHSV.Init();

	if ( pTarget )
	{
		char buf[MAX_PATH];
		pTarget->GetText( buf, MAX_PATH );
		SimpleTokenize tokens;
		tokens.Tokenize( buf );
		for ( int i = 0; i < tokens.Count() && i < 3; i++ )
			m_vecColor[i] = clamp( atof( tokens[i] ), 0, 1 );
		RGB2HSV( m_vecColor, m_vecHSV );
	}

	m_pSelect_Hue = new HSV_Select_Hue( this, "pick_hue" );
	m_pSelect_SV = new HSV_Select_SV( this, "pick_sv" );

	for ( int i = 0; i < 3; i++ )
	{
		char tentry_name[64];
		Q_snprintf( tentry_name, 64, "col_%i", i );
		m_pText_RGB[ i ] = new TextEntry( this, tentry_name );
	}
	m_pText_HEX = new TextEntry( this, "col_hex" );

	Panel *pPrev = new Panel( this, "colorpreview" );
	pPrev->SetPaintEnabled( false );
	pPrev->SetPaintBackgroundEnabled( false );

	LoadControlSettings( "shadereditorui/vgui/colorpicker.res" );

	SetSizeable( false );
	SetSize( 440, 300 );
	SetTitle( "Color picker", false );

	MoveToCenterOfScreen();
	DoModal();

	SetupVguiTex( m_iVgui_Pick_Hue, "shadereditor/colorpicker_hue" );
	SetupVguiTex( m_iVgui_Pick_SV, "shadereditor/colorpicker_sv" );

	pDrawPicker_Hue = new PickerHelper( m_iVgui_Pick_Hue, this );
	pDrawPicker_Hue->SetSize( 48, 16 );
	pDrawPicker_SV = new PickerHelper( m_iVgui_Pick_SV, this );
	pDrawPicker_SV->SetSize( 16, 16 );

	UpdateAllVars();

	MakeReadyForUse();
	InvalidateLayout( true, true );
}

void ColorPicker::Paint()
{
	Color bg = GetBgColor();
	bg[3] = 160;
	SetBgColor( bg );
	SetOutOfFocusColor( bg );

	BaseClass::Paint();

	surface()->DrawSetColor( Color( 0, 0, 0, 255 ) );

#define __EXTRUDE_BORDER 3

	int x, y, sx, sy;
	m_pSelect_Hue->GetBounds( x, y, sx, sy );
	x -= __EXTRUDE_BORDER;
	y -= __EXTRUDE_BORDER;
	sx += __EXTRUDE_BORDER * 2;
	sy += __EXTRUDE_BORDER * 2;
	surface()->DrawFilledRect( x, y, x + sx, y + sy );
	m_pSelect_SV->GetBounds( x, y, sx, sy );
	x -= __EXTRUDE_BORDER;
	y -= __EXTRUDE_BORDER;
	sx += __EXTRUDE_BORDER * 2;
	sy += __EXTRUDE_BORDER * 2;
	surface()->DrawFilledRect( x, y, x + sx, y + sy );

	Panel *pColPrev = FindChildByName( "colorpreview" );
	if ( pColPrev )
	{
		pColPrev->GetBounds( x, y, sx, sy );
		surface()->DrawSetColor( Color( m_vecColor.x * 255, m_vecColor.y * 255, m_vecColor.z * 255, 255 ) );
		surface()->DrawFilledRect( x, y, x + sx, y + sy );

		x -= __EXTRUDE_BORDER;
		y -= __EXTRUDE_BORDER;
		sx += __EXTRUDE_BORDER * 2;
		sy += __EXTRUDE_BORDER * 2;
		surface()->DrawFilledRect( x, y, x + sx, y + sy );
	}
}

void ColorPicker::OnThink()
{
	int x, y, sx, sy;
	m_pSelect_Hue->GetBounds( x, y, sx, sy );
	float hue = clamp( (360.0f - m_pSelect_Hue->GetHue()) / 360.0f, 0, 360.0f );
	pDrawPicker_Hue->SetCenter( x + sx / 2, y + sy * hue );

	m_pSelect_SV->GetBounds( x, y, sx, sy );
	float _s = m_pSelect_SV->GetS();
	float _v = 1.0f - m_pSelect_SV->GetV();
	pDrawPicker_SV->SetCenter( x + sx * _s, y + sy * _v );
}

void ColorPicker::SetPickerColor( const Vector &col )
{
	for ( int i = 0; i < 3; i++ )
		m_vecColor[i] = clamp( col[i], 0, 1 );
	RGB2HSV( m_vecColor, m_vecHSV );

	UpdateAllVars();
}
Vector ColorPicker::GetPickerColor()
{
	return m_vecColor;
}
void ColorPicker::SetPickerColorHSV( const Vector &col )
{
	m_vecHSV = col;
	HSV2RGB( m_vecHSV, m_vecColor );
	UpdateAllVars();
}
Vector ColorPicker::GetPickerColorHSV()
{
	return m_vecHSV;
}
void ColorPicker::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
}

void ColorPicker::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "save" ) )
	{
		if ( pTarget != NULL )
		{
			char str[ MAX_PATH ];
			Q_snprintf( str, MAX_PATH, "%f %f %f", m_vecColor.x, m_vecColor.y, m_vecColor.z );
			pTarget->SetText( str );
		}
		else
		{
			KeyValues *pKV = new KeyValues( "ColorSelected" );
			pKV->SetColor( "color", Color( m_vecColor.x * 255, m_vecColor.y * 255, m_vecColor.z * 255, 255 ) );
			PostActionSignal( pKV );
		}
		Close();
		return;
	}
	BaseClass::OnCommand( cmd );
}

void ColorPicker::OnHSVUpdate( KeyValues *pKV )
{
	Panel *pCaller = ((Panel*)pKV->GetPtr( "panel" ));

	m_vecHSV.x = m_pSelect_Hue->GetHue();
	m_vecHSV.y = m_pSelect_SV->GetS();
	m_vecHSV.z = m_pSelect_SV->GetV();

	UpdateAllVars( pCaller );
}

void ColorPicker::OnTextChanged( KeyValues *pKV )
{
	Panel *pCaller = ((Panel*)pKV->GetPtr( "panel" ));

	Assert( dynamic_cast< TextEntry* >(pCaller) );
	TextEntry *pTEntry = (TextEntry*) pCaller;
	char tmpText[ 32 ];
	pTEntry->GetText( tmpText, 32 );

	for ( int i = 0; i < 3; i++ )
	{
		if ( pCaller != m_pText_RGB[i] )
			continue;
		m_vecColor[i] = atoi( tmpText ) / 255.0f;
	}
	if ( pCaller == m_pText_HEX )
	{
		for ( int i = 0; i < 3; i++ )
		{
			int iValue = 0;
			HexToInt( tmpText + i * 2, iValue, 2 );
			m_vecColor[i] = iValue / 255.0f;
		}
	}

	RGB2HSV( m_vecColor, m_vecHSV );
	UpdateAllVars( pCaller );
}

void ColorPicker::UpdateAllVars( Panel *pIgnore )
{
	HSV2RGB( m_vecHSV.x, m_vecHSV.y, m_vecHSV.z, m_vecColor );

	for ( int i = 0; i < 3; i++ )
		if ( pIgnore != m_pText_RGB[i] )
		{
			char tmp[64];
			Q_snprintf( tmp, 64, "%03i", ((int)clamp( m_vecColor[i] * 255.0f, 0, 255 )) );
			m_pText_RGB[i]->SetText( tmp );
		}

	if ( pIgnore != m_pText_HEX )
	{
		char hexString[ 7 ];
		hexString[ 0 ] = '\0';
		for ( int i = 0; i < 3; i++ )
		{
			char tmp[3];
			::IntToHex( m_vecColor[i] * 255.0, tmp, 2 );
			char tmp2[64];
			Q_snprintf( tmp2, 64, "%02s", tmp );
			Q_strcat( hexString, tmp2, sizeof(hexString) );
		}
		m_pText_HEX->SetText( hexString );
	}

	if ( pIgnore != m_pSelect_Hue )
		m_pSelect_Hue->SetHue( clamp( m_vecHSV.x, 0, 360.0f ) );
	if ( pIgnore != m_pSelect_SV )
	{
		m_pSelect_SV->SetSV( m_vecHSV.y, m_vecHSV.z );
		m_pSelect_SV->SetHue( m_vecHSV.x );
	}
}
