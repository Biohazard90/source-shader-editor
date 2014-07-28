
#include "cbase.h"
#include "vSheets.h"


CSheet_PSInVSOut::CSheet_PSInVSOut(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	m_pSlider_tex = new Slider(this, "tex" );
	m_pSlider_col = new Slider(this, "col" );

	m_pSlider_tex->SetRange( 0, 8 );
	m_pSlider_tex->SetNumTicks( 8 );
	m_pSlider_tex->SetThumbWidth( SHEET_SLIDER_THUMB_SIZE );

	m_pSlider_col->SetRange( 0, 2 );
	m_pSlider_col->SetNumTicks( 2 );
	m_pSlider_col->SetThumbWidth( SHEET_SLIDER_THUMB_SIZE );

	m_pListTexcoords = new PanelListPanel( this, "list_data" );
	m_pListTexcoords->SetNumColumns( 2 );

	for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
	{
		m_pLabel_List[ i ] = new Label(m_pListTexcoords,VarArgs("TexCoords %i",i),VarArgs("TexCoords %i",i));
		m_pComboBox_List[ i ] = new ComboBox(m_pListTexcoords, VarArgs("entry_%i",i), 7, false);
		FillComboBox( m_pComboBox_List[i], NUM_CBOX_VARTYPES );
		m_pComboBox_List[i]->ActivateItem( 0 );

		m_pTextEntry_List[ i ] = new TextEntry( m_pListTexcoords, VarArgs( "entry_%i", i ) );
		m_pTextEntry_List[i]->SetMaximumCharCount( 18 );

		m_pListTexcoords->AddItem( m_pLabel_List[i], m_pComboBox_List[i] );
		m_pListTexcoords->AddItem( NULL, m_pTextEntry_List[ i ] );
	}

	for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
	{
		m_pLabel_List_Colors[ i ] = new Label(m_pListTexcoords,VarArgs("Color %i",i),VarArgs("Color %i",i));
		m_pComboBox_List_Colors[ i ] = new ComboBox(m_pListTexcoords, VarArgs("entry_%i",i), 7, false);
		FillComboBox( m_pComboBox_List_Colors[i], NUM_CBOX_VARTYPES - 3 );
		m_pComboBox_List_Colors[i]->ActivateItem( 0 );

		m_pTextEntry_List_Colors[ i ] = new TextEntry( m_pListTexcoords, VarArgs( "entry_%i", i ) );
		m_pTextEntry_List_Colors[i]->SetMaximumCharCount( 18 );

		m_pListTexcoords->AddItem( m_pLabel_List_Colors[i], m_pComboBox_List_Colors[i] );
		m_pListTexcoords->AddItem( NULL, m_pTextEntry_List_Colors[ i ] );
	}
		
	m_pLabel_Info_Num_TexCoords = new Label( this, "info_tex", "" );
	m_pLabel_Info_Num_Colors = new Label( this, "info_col", "" );

	LoadControlSettings( "shadereditorui/vgui/sheet_vsoutput_psinput.res" );
}
CSheet_PSInVSOut::~CSheet_PSInVSOut()
{
}

void CSheet_PSInVSOut::FillComboBox( ComboBox *c, int numItems )
{
	c->AddActionSignalTarget( this );
	for ( int i = 0; i < numItems; i++ )
	{
		KeyValues *data = new KeyValues( VarArgs("vartype_%i",i), "vartype", i );
		c->AddItem( GetVarTypeName( i ), data );
		data->deleteThis();
	}
}
void CSheet_PSInVSOut::OnTextChanged( KeyValues *pKV )
{
	//Panel *pCaller = ((Panel*)pKV->GetPtr( "panel" ));
	int iDesiredJacks = m_pSlider_tex->GetValue();
	int iItemsToDisable = 0;

	for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
	{
		ComboBox *curBox = m_pComboBox_List[ i ];
		int curItem = curBox->GetActiveItem();

		bool bEnable = iItemsToDisable <= 0 && i < iDesiredJacks;
		if ( !bEnable )
		{
			curBox->ActivateItem( 0 );
			curItem = 0;
		}

		int rows = GetVarTypeRowsRequired( curItem );

		if ( rows + i > iDesiredJacks )
		{
			curBox->ActivateItem( 0 );
			curItem = 0;
			rows = GetVarTypeRowsRequired( curItem );
		}

		m_pLabel_List[ i ]->SetEnabled( bEnable );
		m_pComboBox_List[ i ]->SetEnabled( bEnable );
		m_pTextEntry_List[ i ]->SetEnabled( bEnable );

		if ( bEnable )
			iItemsToDisable = rows - 1;
		else
			iItemsToDisable--;
	}
}
void CSheet_PSInVSOut::OnSliderMoved( int position )
{
	int numTex = m_pSlider_tex->GetValue();
	int numCol = m_pSlider_col->GetValue();
	EnableItems_Textures( numTex );
	EnableItems_Colors( numCol );

	m_pLabel_Info_Num_TexCoords->SetText( VarArgs( "%1i", numTex ) );
	m_pLabel_Info_Num_Colors->SetText( VarArgs( "%1i", numCol ) );

	OnTextChanged( NULL );
}
void CSheet_PSInVSOut::EnableItems_Textures( int numTexcoords )
{
	for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
	{
		bool bEnable = numTexcoords > 0;
		m_pLabel_List[ i ]->SetEnabled( bEnable );
		m_pComboBox_List[ i ]->SetEnabled( bEnable );
		m_pTextEntry_List[ i ]->SetEnabled( bEnable );
		numTexcoords--;
	}
}
void CSheet_PSInVSOut::EnableItems_Colors( int numColors )
{
	for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
	{
		bool bEnable = numColors > 0;
		m_pLabel_List_Colors[ i ]->SetEnabled( bEnable );
		m_pComboBox_List_Colors[ i ]->SetEnabled( bEnable );
		m_pTextEntry_List_Colors[ i ]->SetEnabled( bEnable );
		numColors--;
	}
}

void CSheet_PSInVSOut::OnResetData()
{
	int numTexcoordJacks = pData->GetInt( "numTexcoords" );
	int numColorJacks = pData->GetInt( "numColor" );

	m_pSlider_tex->SetValue( numTexcoordJacks );
	m_pSlider_col->SetValue( numColorJacks );

	for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
	{
		int flag = pData->GetInt( VarArgs( "numTexcoord_Flag_%i", i ) );
		int value = GetVarFlagsVarValue( flag );
		m_pComboBox_List[ i ]->ActivateItem( value );
		m_pTextEntry_List[ i ]->SetText( pData->GetString(VarArgs( "numTexcoord_Name_%i", i )) );
	}
	for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
	{
		int flag = pData->GetInt( VarArgs( "numColor_Flag_%i", i ) );
		int value = GetVarFlagsVarValue( flag );
		m_pComboBox_List_Colors[ i ]->ActivateItem( value );
		m_pTextEntry_List_Colors[ i ]->SetText( pData->GetString(VarArgs( "numColor_Name_%i", i )) );
	}


	EnableItems_Textures( numTexcoordJacks );
	EnableItems_Colors( numColorJacks );
	OnTextChanged( NULL );
}
void CSheet_PSInVSOut::OnApplyChanges()
{
	pData->SetInt( "numTexcoords",m_pSlider_tex->GetValue() );
	pData->SetInt( "numColor",m_pSlider_col->GetValue() );

	for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
	{
		ComboBox *curBox = m_pComboBox_List[ i ];
		int curItem = curBox->GetActiveItem();
		int flag = GetVarTypeFlag( curItem );
		pData->SetInt( VarArgs( "numTexcoord_Flag_%i", i ), flag );
		char tmp[32];
		m_pTextEntry_List[i]->GetText( tmp, 32 );
		pData->SetString( VarArgs( "numTexcoord_Name_%i", i ), tmp );
	}
	for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
	{
		ComboBox *curBox = m_pComboBox_List_Colors[ i ];
		int curItem = curBox->GetActiveItem();
		int flag = GetVarTypeFlag( curItem );
		pData->SetInt( VarArgs( "numColor_Flag_%i", i ), flag );
		char tmp[32];
		m_pTextEntry_List_Colors[i]->GetText( tmp, 32 );
		pData->SetString( VarArgs( "numColor_Name_%i", i ), tmp );
	}
}
