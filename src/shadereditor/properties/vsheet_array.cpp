
#include "cbase.h"
#include "vSheets.h"


CSheet_Array::CSheet_Array(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pTEntry_Size_X = new TextEntry( this, "asize_x" );
	m_pTEntry_Size_Y = new TextEntry( this, "asize_y" );
	m_pTEntry_Gauss_Bias = new TextEntry( this, "gauss_bias" );
	m_pTEntry_Random_Min = new TextEntry( this, "ran_min" );
	m_pTEntry_Random_Max = new TextEntry( this, "ran_max" );
	m_pCBox_Datatype = new ComboBox( this, "cbox_datatype", 10, false );
	for ( int i = 0; i < 4; i++ )
		m_pCBox_Datatype->AddItem( ::GetVarTypeName( i ), NULL );

	m_pArrayPanel = new PanelListPanel( this, "array_panel" );

	LoadControlSettings( "shadereditorui/vgui/sheet_array.res" );

	m_pTEntry_Random_Min->SetText( "0" );
	m_pTEntry_Random_Max->SetText( "1" );
}
CSheet_Array::~CSheet_Array()
{
	for ( int i = 0; i < m_hArray_Y_Major.Count(); i++ )
	{
		m_hArray_Y_Major[i]->Purge();
		delete m_hArray_Y_Major[i];
	}
	m_hArray_Y_Major.Purge();
}

void CSheet_Array::OnResetData()
{
	int num_x = pData->GetInt( "i_array_size_x", 1 );
	int num_y = pData->GetInt( "i_array_size_y", 1 );
	m_pTEntry_Size_X->SetText( VarArgs( "%i", num_x ) );
	m_pTEntry_Size_Y->SetText( VarArgs( "%i", num_y ) );
	m_pTEntry_Gauss_Bias->SetText( "0.0" );
	int iEnum = ::GetVarFlagsVarValue( pData->GetInt( "i_array_datatype" ) );
	m_pCBox_Datatype->ActivateItem( iEnum );

	UpdateArrayBlocks( num_x, num_y );

	::TokenChannels_t tokens;
	for ( int x = 0; x < num_x; x++ )
	{
		for ( int y = 0; y < num_y; y++ )
		{
			int element = num_y * x + y;

			char tmp[MAX_PATH];
			Q_snprintf( tmp, sizeof( tmp ), "%s", pData->GetString( VarArgs( "pfl_arraydata_%03i", element ) ) );

			tokens.bAllowChars = false;
			tokens.SetDefaultChannels();
			tokens.Tokenize( tmp );
			tokens.iActiveChannels = iEnum + 1;
			tokens.PrintTargetString( tmp, sizeof( tmp ) );

			m_hArray_Y_Major[ y ]->Element( x )->SetText( tmp );
		}
	}
}
void CSheet_Array::OnApplyChanges()
{
	int cur_x, cur_y;
	char ***entries = BuildStringSheet( cur_x, cur_y );
	int iEnum = clamp( m_pCBox_Datatype->GetActiveItem(), 0, 3 );

	pData->SetInt( "i_array_datatype", ::GetVarTypeFlag( iEnum ) );
	pData->SetInt( "i_array_size_x", cur_x );
	pData->SetInt( "i_array_size_y", cur_y );

	::TokenChannels_t tokens;

	for ( int x = 0; x < cur_x; x++ )
	{
		for ( int y = 0; y < cur_y; y++ )
		{
			int element = cur_y * x + y;
			//m_hArray_X_Major[ x ]->Element( y )->GetText( tmp, sizeof( tmp ) );
			char *str = entries[x][y];
			char tmp[MAX_PATH];

			tokens.bAllowChars = false;
			tokens.Tokenize( str );
			tokens.PrintTargetString( tmp, sizeof( tmp ) );
			pData->SetString( VarArgs( "pfl_arraydata_%03i", element ), tmp );
		}
	}

	DestroyStringSheet( entries, cur_x, cur_y );
}

void CSheet_Array::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "update_array" ) )
	{
		int new_x = 1;
		int new_y = 1;

		char tmp[MAX_PATH];
		m_pTEntry_Size_X->GetText( tmp, sizeof( tmp ) );
		new_x = max( new_x, atoi( tmp ) );
		m_pTEntry_Size_Y->GetText( tmp, sizeof( tmp ) );
		new_y = max( new_y, atoi( tmp ) );

		UpdateArrayBlocks( new_x, new_y );
	}
	else if ( !Q_stricmp( cmd, "fill_gauss" ) )
	{
		m_pCBox_Datatype->ActivateItem( 0 );

		char tmp[MAX_PATH];
		m_pTEntry_Gauss_Bias->GetText( tmp, sizeof( tmp ) );

		float gauss_bias = atof( tmp );
		int asize_y = m_hArray_Y_Major.Count();
		int asize_x = ( asize_y > 0 ) ? m_hArray_Y_Major[0]->Count() : 0;
		bool bIs2D = asize_y > 1;
		for ( int x = 0; x < asize_x; x++ )
		{
			for ( int y = 0; y < asize_y; y++ )
			{
				float flWeight = 0;
				if ( bIs2D )
					flWeight = ::GaussianWeight_2D( x, y, asize_x, asize_y, gauss_bias );
				else
					flWeight = ::GaussianWeight_1D( x, asize_x, gauss_bias );
				m_hArray_Y_Major[y]->Element(x)->SetText( VarArgs( "%f", flWeight ) );
			}
		}
	}
	else if ( !Q_stricmp( cmd, "fill_random" ) )
	{
		int numComps = m_pCBox_Datatype->GetActiveItem();
		float ran_min, ran_max;
		char tmp[MAX_PATH];

		m_pTEntry_Random_Min->GetText( tmp, sizeof( tmp ) );
		ran_min = atof( tmp );
		m_pTEntry_Random_Max->GetText( tmp, sizeof( tmp ) );
		ran_max = atof( tmp );

		int asize_y = m_hArray_Y_Major.Count();
		int asize_x = ( asize_y > 0 ) ? m_hArray_Y_Major[0]->Count() : 0;

		for ( int x = 0; x < asize_x; x++ )
		{
			for ( int y = 0; y < asize_y; y++ )
			{
				tmp[0] = '\0';

				char szComp[32];
				for ( int c = 0; c <= numComps; c++ )
				{
					Q_snprintf( szComp, sizeof( szComp ), "%f", RandomFloat( ran_min, ran_max ) );
					Q_strcat( tmp, szComp, sizeof( tmp ) );
					if ( c < numComps )
						Q_strcat( tmp, " ", sizeof( tmp ) );
				}

				m_hArray_Y_Major[y]->Element(x)->SetText( tmp );
			}
		}
	}
	else
		BaseClass::OnCommand( cmd );
}
void CSheet_Array::OnTextFocusLost( KeyValues *pKV )
{
	Panel *pCaller = (Panel*)pKV->GetPtr("panel");
	if ( pCaller == m_pTEntry_Size_X || pCaller == m_pTEntry_Size_Y )
	{
	}
}
void CSheet_Array::OnTextChanged( KeyValues *pKV )
{
	//Panel *pCaller = (Panel*)pKV->GetPtr("panel");
}

char ***CSheet_Array::BuildStringSheet( int &sizex, int &sizey )
{
	sizey = m_hArray_Y_Major.Count();
	sizex = ( sizey > 0 ) ? m_hArray_Y_Major[0]->Count() : 0;

	if ( sizex <= 0 || sizey <= 0 )
		return NULL;

	char ***strings = new char**[sizex];
	for ( int x = 0; x < sizex; x++ )
	{
		strings[ x ] = new char*[sizey];
		for ( int y = 0; y < sizey; y++ )
		{
			char szOld[MAX_PATH];
			m_hArray_Y_Major[y]->Element( x )->GetText( szOld, sizeof( szOld ) );
			int textlen = Q_strlen( szOld ) + 1;
			strings[x][y] = new char[ textlen ];
			Q_strcpy( strings[x][y], szOld );
		}
	}

	return strings;
}
void CSheet_Array::DestroyStringSheet( char ***strings, const int &sizex, const int &sizey )
{
	for ( int x = 0; x < sizex; x++ )
	{
		for ( int y = 0; y < sizey; y++ )
			delete [] strings[x][y];
		delete [] strings[x];
	}
	delete[] strings;
}
void CSheet_Array::UpdateArrayBlocks( const int &dest_x, const int &dest_y )
{
	int old_x, old_y;
	old_x = old_y = 0;
	char ***oldEntires = BuildStringSheet( old_x, old_y );

	for ( int y = 0; y < old_y; y++ )
	{
		//for ( int x = 0; x < old_x; x++ )
		m_hArray_Y_Major[y]->Purge();
		delete m_hArray_Y_Major[y];
	}
	m_hArray_Y_Major.Purge();

	int iCurItem = m_pArrayPanel->FirstItem();
	while ( iCurItem != m_pArrayPanel->InvalidItemID() )
	{
		Panel *pA = m_pArrayPanel->GetItemLabel( iCurItem );
		Panel *pB = m_pArrayPanel->GetItemPanel( iCurItem );
		if ( pA != NULL )
			pA->MarkForDeletion();
		if ( pB != NULL )
			pB->MarkForDeletion();
		iCurItem = m_pArrayPanel->NextItem( iCurItem );
	}
	m_pArrayPanel->DeleteAllItems();
	m_pArrayPanel->RemoveAll();

	for ( int y = 0; y < dest_y; y++ )
	{
		CUtlVector< TextEntry* > *cur = new CUtlVector< TextEntry* >;
		m_hArray_Y_Major.AddToTail( cur );
	}

	m_pArrayPanel->SetNumColumns( dest_y );
	for ( int x = 0; x < dest_x; x++ )
	{
		for ( int y = 0; y < dest_y; y++ )
		{
			CUtlVector< TextEntry* > *cur = m_hArray_Y_Major[y];

			TextEntry *pEntry = new TextEntry( m_pArrayPanel, "arrayslot" );
			pEntry->AddActionSignalTarget( this );
			cur->AddToTail( pEntry );

			if ( x < old_x &&
				y < old_y && oldEntires != NULL )
			{
				pEntry->SetText( oldEntires[x][y] );
			}
			else
				pEntry->SetText( "0" );

			Label *pHeader = NULL;
			if ( y == 0 )
				pHeader = new Label( m_pArrayPanel, "", VarArgs( "%i:", x ) );

			m_pArrayPanel->AddItem( pHeader, pEntry );
			pEntry->MakeReadyForUse();
			pEntry->InvalidateLayout( true, true );
			pEntry->SetBgColor( TOKENCHANNELS_SUPPORTED_COLOR );
		}
	}
	m_pArrayPanel->SetFirstColumnWidth( 20 );

	if ( oldEntires != NULL )
		DestroyStringSheet( oldEntires, old_x, old_y );
}
