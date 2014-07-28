
#include "cbase.h"
#include "vSheets.h"


CSheet_PSOutput::CSheet_PSOutput(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	m_pSlider_col = new Slider(this, "col" );
	m_pSlider_col->SetRange( 1, 4 );
	m_pSlider_col->SetNumTicks( 3 );
	m_pSlider_col->SetThumbWidth( SHEET_SLIDER_THUMB_SIZE );

	m_pCheck_Depth = new CheckButton(this, "depth","");

	m_pLabel_Info_Num_Colors = new Label( this, "info_col", "" );

	LoadControlSettings( "shadereditorui/vgui/sheet_psoutput.res" );
}
CSheet_PSOutput::~CSheet_PSOutput()
{
}

void CSheet_PSOutput::OnResetData()
{
	m_pSlider_col->SetValue( pData->GetInt( "numColors" ) );
	m_pCheck_Depth->SetSelected( ( pData->GetInt( "enable_Depth" ) != 0 ) );
}
void CSheet_PSOutput::OnApplyChanges()
{
	pData->SetInt( "numColors",m_pSlider_col->GetValue() );
	pData->SetInt( "enable_Depth", m_pCheck_Depth->IsSelected() ? 1 : 0 );
}


void CSheet_PSOutput::OnSliderMoved( int position )
{
	int numCol = m_pSlider_col->GetValue();
	m_pLabel_Info_Num_Colors->SetText( VarArgs( "%1i", numCol ) );
}