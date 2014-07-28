
#include "cbase.h"
#include "vSheets.h"


CSheet_VSInput::CSheet_VSInput(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	m_pCheck_flex = new CheckButton(this, "flex","");
	m_pCheck_normal = new CheckButton(this, "normal","");
	m_pCheck_normal_compression = new CheckButton(this, "normal_compression","");
	m_pCheck_tangent = new CheckButton(this, "tangent","");
	m_pCheck_blend = new CheckButton(this, "blend","");
	m_pCheck_tangent_skinable = new CheckButton(this, "tangent_skinable","");

	m_pSlider_tex = new Slider(this, "tex" );
	m_pSlider_col = new Slider(this, "col" );

	m_pSlider_tex->SetRange( 0, 3 );
	m_pSlider_tex->SetNumTicks( 3 );
	m_pSlider_tex->SetThumbWidth( SHEET_SLIDER_THUMB_SIZE );

	m_pSlider_col->SetRange( 0, 2 );
	m_pSlider_col->SetNumTicks( 2 );
	m_pSlider_col->SetThumbWidth( SHEET_SLIDER_THUMB_SIZE );

	m_pLabel_Info_Num_TexCoords = new Label( this, "info_tex", "" );
	m_pLabel_Info_Num_Colors = new Label( this, "info_col", "" );

	for ( int i = 0; i < 3; i++ )
		m_pCBox_DType_TexCoords[i] = new ComboBox( this, VarArgs( "cbox_dtypes_texcoords_%i", i ), 10, false );
	for ( int i = 0; i < 2; i++ )
		m_pCBox_DType_Color[i] = new ComboBox( this, VarArgs( "cbox_dtypes_color_%i", i ), 10, false );

	for ( int i = 0; i < 4; i++ )
	{
		for ( int d = 0; d < 3; d++ )
			m_pCBox_DType_TexCoords[d]->AddItem( ::GetVarTypeName( i ), NULL );
		for ( int d = 0; d < 2; d++ )
			m_pCBox_DType_Color[d]->AddItem( ::GetVarTypeName( i ), NULL );
	}

	LoadControlSettings( "shadereditorui/vgui/sheet_vsinput.res" );
}
CSheet_VSInput::~CSheet_VSInput()
{
}

void CSheet_VSInput::OnResetData()
{
	m_pSlider_tex->SetValue( pData->GetInt( "numTexcoords" ) );
	m_pSlider_col->SetValue( pData->GetInt( "numColor" ) );

	m_pCheck_flex->SetSelected( ( pData->GetInt( "enable_flex" ) != 0 ) );
	m_pCheck_normal->SetSelected( ( pData->GetInt( "enable_normal" ) != 0 ) );
	m_pCheck_normal_compression->SetSelected( ( pData->GetInt( "enable_normal_compression" ) != 0 ) );
	m_pCheck_tangent->SetSelected( ( pData->GetInt( "enable_tangentspace" ) != 0 ) );
	m_pCheck_blend->SetSelected( ( pData->GetInt( "enable_blendweights" ) != 0 ) );
	m_pCheck_tangent_skinable->SetSelected( ( pData->GetInt( "enable_tangentspace_skinning" ) != 0 ) );

	for ( int d = 0; d < 3; d++ )
		m_pCBox_DType_TexCoords[d]->ActivateItem( ::GetVarFlagsVarValue( pData->GetInt( VarArgs( "dTFlag_Texcoords_%i", d ) ) ) );

	for ( int d = 0; d < 2; d++ )
		m_pCBox_DType_Color[d]->ActivateItem( ::GetVarFlagsVarValue( pData->GetInt( VarArgs( "dTFlag_Color_%i", d ) ) ) );
}
void CSheet_VSInput::OnApplyChanges()
{
	pData->SetInt( "numTexcoords",m_pSlider_tex->GetValue() );
	pData->SetInt( "numColor",m_pSlider_col->GetValue() );

	pData->SetInt( "enable_flex", m_pCheck_flex->IsSelected() ? 1 : 0 );
	pData->SetInt( "enable_normal", m_pCheck_normal->IsSelected() ? 1 : 0 );
	pData->SetInt( "enable_normal_compression", m_pCheck_normal_compression->IsSelected() ? 1 : 0 );
	pData->SetInt( "enable_tangentspace", m_pCheck_tangent->IsSelected() ? 1 : 0 );
	pData->SetInt( "enable_blendweights", m_pCheck_blend->IsSelected() ? 1 : 0 );
	pData->SetInt( "enable_tangentspace_skinning", m_pCheck_tangent_skinable->IsSelected() ? 1 : 0 );

	for ( int d = 0; d < 3; d++ )
		pData->SetInt( VarArgs( "dTFlag_Texcoords_%i", d ), ::GetVarTypeFlag( m_pCBox_DType_TexCoords[d]->GetActiveItem() ) );
	for ( int d = 0; d < 2; d++ )
		pData->SetInt( VarArgs( "dTFlag_Color_%i", d ), ::GetVarTypeFlag( m_pCBox_DType_Color[d]->GetActiveItem() ) );
}

void CSheet_VSInput::OnSliderMoved( int position )
{
	int numTex = m_pSlider_tex->GetValue();
	int numCol = m_pSlider_col->GetValue();

	m_pLabel_Info_Num_TexCoords->SetText( VarArgs( "%1i", numTex ) );
	m_pLabel_Info_Num_Colors->SetText( VarArgs( "%1i", numCol ) );

	for ( int d = 0; d < 3; d++ )
		m_pCBox_DType_TexCoords[d]->SetEnabled( numTex - 1 >= d );
	for ( int d = 0; d < 2; d++ )
		m_pCBox_DType_Color[d]->SetEnabled( numCol - 1 >= d );
}

void CSheet_VSInput::OnCheckButtonChecked( KeyValues *pData )
{
	Panel *pCaller = ((Panel*)pData->GetPtr( "panel" ));
	bool bState = ( pData->GetInt( "state" ) != 0 );

	if ( !pCaller )
		return;

	if ( pCaller == m_pCheck_tangent )
	{
		m_pCheck_tangent_skinable->SetEnabled( bState );
		if ( !bState )
			m_pCheck_tangent_skinable->SetSelected( false );
	}
	else if ( pCaller == m_pCheck_normal )
	{
		m_pCheck_normal_compression->SetEnabled( bState );
		if ( !bState )
			m_pCheck_normal_compression->SetSelected( false );
	}
}