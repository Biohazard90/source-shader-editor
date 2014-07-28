
#include "cbase.h"

#include "vSmartObject.h"
#include "vSmartObjectList.h"
#include "vSmartAutocomplete.h"
#include "vSmartTooltip.h"
#include "cRegex.h"

#include "vgui/iinput.h"
#include <vgui_controls/menuitem.h>


CSmartAutocomplete::CSmartAutocomplete( Panel *parent, const char *pElementname ) : BaseClass( parent, pElementname )
{
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( false );

	SetVisible(true);
	MakePopup(false);

	m_pMatchStr = NULL;
	m_pInfoLabel = NULL;

	//SetSize( GetACSizeX(), GetACSizeY() );
	//SetMinimumSize( GetACSizeX(), GetACSizeY() );

	m_pList = NULL;

	SetupVguiTex( m_iImg_Var, "shadereditor/ac_var" );
	SetupVguiTex( m_iImg_Func, "shadereditor/ac_func" );
	SetupVguiTex( m_iImg_Struct, "shadereditor/ac_struct" );
	SetupVguiTex( m_iImg_Def, "shadereditor/ac_def" );
}

CSmartAutocomplete::~CSmartAutocomplete()
{
	if ( m_pInfoLabel )
		m_pInfoLabel->MarkForDeletion();

	delete m_pList;
	delete [] m_pMatchStr;
}

bool CSmartAutocomplete::InitList( CSmartObjectList *list,
	int textPos, int iScopeDepth, int iScopeNum,
	const char *pSourceFile, const char *pszVarName, bool bSort )
{
	if ( pszVarName && Q_strlen( pszVarName ) )
	{
		if ( !list ||
			!pszVarName ||
			!Q_strlen(pszVarName) )
			return false;

		CSmartObject *pVar = list->GetEntryByIdent( pszVarName );

		if ( !pVar || pVar->m_iType != ACOMP_VAR || !pVar->m_pszDatatype )
			return false;

		char *pszDatatype = pVar->m_pszDatatype;
		bool bSuccess = true;
		bool bSort = true;

		CSmartObjectList *pNewList = new CSmartObjectList();

		if ( CRegex::MatchSyntax_Datatypes( pszDatatype ) != __REGEX_INVALID )
		{
			const char *pszMatch = "(:?double|float|half|HALF|int|uint|bool)(?:[1-4]|[1-4]x[1-4])";
			bSuccess = CRegex::RegexMatch( pszDatatype, pszMatch );

			if ( bSuccess )
			{
				int iC = Q_strlen( pszDatatype ) - 1;

				bool bMatrix = pszDatatype[iC-1] == 'x' &&
					pszDatatype[iC] >= '1' && pszDatatype[iC] <= '4' &&
					pszDatatype[iC-2] >= '1' && pszDatatype[iC-2] <= '4';

				if ( bMatrix )
				{
					char matType[12];
					Q_snprintf( matType, sizeof(matType), "_m[%c][%c]", pszDatatype[iC-2], pszDatatype[iC] );
					pNewList->AddUnit( new CSmartObject( ACOMP_VAR, "", matType, pszDatatype ) );
				}
				else
				{
					int num = pszDatatype[iC] - '0';
					Assert( num >= 1 && num <= 4 );

					bSort = false;

					const char *lookUpIdents[] =
					{
						"r", "g", "b", "a", "x", "y", "z", "w",
					};

					for ( int i = 0; i < num; i++ )
						pNewList->AddUnit( new CSmartObject( ACOMP_VAR, "", lookUpIdents[i], pszDatatype ) );
					for ( int i = 0; i < num; i++ )
						pNewList->AddUnit( new CSmartObject( ACOMP_VAR, "", lookUpIdents[i+4], pszDatatype ) );
				}
			}
		}
		else
		{
			CSmartObject *pStruct = list->GetEntryByIdent( pszDatatype );

			if ( !pStruct || !pStruct->IsObjectVisible( textPos, iScopeDepth, iScopeNum, pSourceFile ) || !pStruct->m_hChildren.Count() )
				bSuccess = false;
			else
			{
				for ( int i = 0; i < pStruct->m_hChildren.Count(); i++ )
					pNewList->AddUnit( new CSmartObject( *pStruct->m_hChildren[i] ) );
			}
		}

		if ( bSuccess )
		{
			bSuccess = InitList( pNewList, 0, 0, 0, NULL, NULL, bSort );
		}

		delete pNewList;

		return bSuccess;
	}




	delete m_pList;
	m_pList = new CSmartObjectList( *list );

	if ( bSort )
		m_pList->DoSort();

	bool bValid = false;
	//for ( int i = 0; i < 30; i++ )
	//	m_pACMenu->AddMenuItem( VarArgs( "empty item %02i ................", i ), this );

	int numEntries = m_pList->GetNumEntries();

	KeyValues *pItemData = new KeyValues( "data" );

	for ( int i = 0; i < numEntries; i++ )
	{
		CSmartObject *pO = m_pList->GetEntry( i );

		if ( pO->m_bIntrinsic )
			continue;

		const char *pszIdent = pO->m_pszIdentifier;

		if ( !pszIdent )
			continue;

		if ( !pO->IsObjectVisible( textPos, iScopeDepth, iScopeNum, pSourceFile ) )
			continue;

		//if ( pszVarName && Q_strlen( pszVarName ) )
		//{
		//}

		bValid = true;

		pItemData->SetInt( "database_index", i );
		MenuItem *pItem = GetMenuItem( AddMenuItem( pszIdent, this, pItemData ) );


		Assert( pItem );

		if ( !pItem )
			continue;

		pItem->SetIgnoreFirstClick( true );

		int iImg = -1;

		switch ( pO->m_iType )
		{
		case ACOMP_DEFINE:
			iImg = m_iImg_Def;
			break;
		case ACOMP_FUNC:
			iImg = m_iImg_Func;
			break;
		case ACOMP_STRUCT_DEF:
			iImg = m_iImg_Struct;
			break;
		case ACOMP_VAR:
			iImg = m_iImg_Var;
			break;
		}

		if ( iImg < 0 )
			continue;

		pItem->SetImage( iImg );
	}

	pItemData->deleteThis();

	//ActivateItem( 0 );

	SelectItem( 0 );

	AddActionSignalTarget( this );

	return bValid;
}


void CSmartAutocomplete::MatchInput( const char *str )
{
	const bool bShowAll = str == NULL;
	bool bMatchSuccessful = false;

	int iFirst = MatchSetVisible( bShowAll, str );

	if ( iFirst < 0 )
	{
		char *pTest = m_pMatchStr;

		if ( str && *str && !(IS_CHAR(*str)||IS_NUMERICAL(*str)) )
			pTest = NULL;

		iFirst = MatchSetVisible( bShowAll, pTest );

		Assert( iFirst >= 0 );
	}
	else
		bMatchSuccessful = true;

	if ( bShowAll || bMatchSuccessful )
	{
		Assert( bMatchSuccessful && str && *str || bShowAll );

		delete [] m_pMatchStr;
		m_pMatchStr = NULL;

		if ( str && *str )
			AutoCopyStringPtr( str, &m_pMatchStr );
	}

	InvalidateLayout( true );

	SelectItem( iFirst );
}
void CSmartAutocomplete::MatchInput( const wchar_t *str )
{
	char *cStr = NULL;

	if ( str )
	{
		int size = Q_wcslen( str ) + 1;
		cStr = new char[size];
		Q_UnicodeToUTF8( str, cStr, size );
	}

	MatchInput( cStr );
	delete [] cStr;
}

int CSmartAutocomplete::IsExactMatch( const char *str, bool bPartial )
{
	if ( !str || !*str )
		return -1;

	char tmp[MAX_PATH];

	CUtlVector<int> hSuccess;

	for ( int i = 0; i < GetItemCount(); i++ )
	{
		int mID = GetMenuID(i);
		MenuItem *pItem = GetMenuItem(mID);
		Assert( pItem );

		if ( !pItem->IsVisible() )
			continue;

		pItem->GetText( tmp, sizeof(tmp) );

		if ( !Q_strlen( tmp ) )
			continue;

		if ( Q_stricmp( tmp, str ) )
		{
			if ( !bPartial || Q_stristr( tmp, str ) != tmp )
			{
				hSuccess.Purge();
				return -1;
			}
		}
		else
			hSuccess.AddToTail( mID );
	}

	int result = hSuccess.Count() ? hSuccess[0] : -1;

	if ( hSuccess.Count() > 1 )
	{
		for ( int i = 0; i < hSuccess.Count(); i++ )
		{
			MenuItem *pItem = GetMenuItem(hSuccess[i]);
			Assert( pItem );

			if ( !pItem->IsVisible() )
				continue;

			pItem->GetText( tmp, sizeof(tmp) );

			if ( !Q_strcmp( tmp, str ) )
				result = hSuccess[i];
		}
	}

	hSuccess.Purge();
	return result;
}

int CSmartAutocomplete::IsExactMatch( const wchar_t *str, bool bPartial )
{
	char *cStr = NULL;

	if ( str && *str )
	{
		int size = Q_wcslen( str ) + 1;
		cStr = new char[size];
		Q_UnicodeToUTF8( str, cStr, size );
	}

	int bResult = IsExactMatch( cStr, bPartial );
	delete [] cStr;

	return bResult;
}

int CSmartAutocomplete::MatchSetVisible( bool bAll, const char *str )
{
	if ( !str || !*str )
		bAll = true;

	char buf[MAX_PATH];
	char exp[MAX_PATH+2];

	int iFirstVisible = -1;

	for ( int i = 0; i < GetItemCount(); i++ )
	{
		MenuItem *pItem = GetMenuItem(GetMenuID(i));
		Assert( pItem );

		bool bShow = bAll;

		if ( !bAll )
		{
			pItem->GetText( buf, sizeof( buf ) );
			Q_strlower( buf );

			char *pSafe = CRegex::AllocSafeString( (char*)str );
			Q_snprintf( exp, sizeof(exp), "%s.*", pSafe );
			delete [] pSafe;
			Q_strlower( exp );

			bShow = CRegex::RegexMatch( buf, exp );
		}

		pItem->SetVisible( bShow );

		if ( bShow && iFirstVisible < 0 )
			iFirstVisible = i;
	}

	//if ( iFirstVisible < 0 )
	//	iFirstVisible = 0;

	return iFirstVisible;
}
void CSmartAutocomplete::SelectItem( int i )
{
	int last = GetCurrentlyHighlightedItem();

	if ( i == last )
		return;

	ActivateItem( i );
	SetCurrentlyHighlightedItem( i );
}

char *CSmartAutocomplete::AllocHighlightItemName()
{
	char buf[MAX_PATH];
	GetItemText( GetCurrentlyHighlightedItem(), buf, sizeof(buf) );

	if ( !*buf || Q_strlen(buf) < 1 )
		return NULL;

	char *pOut = new char[ Q_strlen(buf) + 1 ];
	Q_strcpy( pOut, buf );

	return pOut;
}

int CSmartAutocomplete::FindEntry( const char *pWord )
{
	char buf[MAX_PATH];

	for ( int i = 0; i < GetItemCount(); i++ )
	{
		int iMID = GetMenuID(i);
		Assert( GetMenuItem(iMID) );
		GetMenuItem(iMID)->GetText( buf, sizeof(buf) );

		if ( Q_stricmp( buf, pWord ) )
			continue;

		KeyValues *pKV = GetItemUserData( iMID );
		if ( !pKV )
		{
			Assert( 0 );
			break;
		}

		return pKV->GetInt( "database_index", -1 );
	}
	return 0;
}
void CSmartAutocomplete::ActivateEntry( int index )
{
	if ( !m_pList )
		return;

	Assert( index >= 0 && index < m_pList->GetNumEntries() );

	for ( int i = 0; i < GetItemCount(); i++ )
	{
		int iMID = GetMenuID(i);
		Assert( GetMenuItem(iMID) );
		KeyValues *pKV = GetItemUserData( iMID );
		if ( !pKV )
			continue;

		if ( pKV->GetInt( "database_index", -1 ) != index )
			continue;

		ActivateItem( iMID );
		break;
	}
}

void CSmartAutocomplete::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CSmartAutocomplete::OnKillFocus()
{
	BaseClass::OnKillFocus();

	//if (!input()->GetFocus() || !ipanel()->HasParent(input()->GetFocus(), GetVPanel()))
	//{
	//}
}

void CSmartAutocomplete::InternalMousePressed(int code)
{
}

void CSmartAutocomplete::MenuItemHighlight( int itemID )
{
	UpdateTooltip();
}

void CSmartAutocomplete::UpdateTooltip()
{
	if ( m_pInfoLabel )
		m_pInfoLabel->MarkForDeletion();
	m_pInfoLabel = NULL;

	int item = GetCurrentlyHighlightedItem();

	if ( item < 0 || item >= GetItemCount() )
		return;

	//int posy = GetVisibleItemIndex( item );

	int px, py, sx, sy;
	GetBounds( px, py, sx, sy );

	MenuItem *pItem = GetMenuItem( item );

	if ( !pItem )
		return;

	int ipx, ipy;
	pItem->GetPos( ipx, ipy );

	px += sx + 5;
	py += ipy;
	
	m_pInfoLabel = new CSmartTooltip( this, "ACtooltip" );

	Assert( pItem->GetUserData() );

	int iObj = pItem->GetUserData()->GetInt( "database_index" );
	
	Assert( m_pList );

	if ( !m_pInfoLabel->Init( m_pList->GetEntry( iObj ) ) )
	{
		m_pInfoLabel->MarkForDeletion();
		m_pInfoLabel = NULL;
	}
	else
	{
		int screenx, screeny;
		surface()->GetScreenSize( screenx, screeny );
		m_pInfoLabel->GetSize( sx, sy );

		if ( py + sy > screeny )
			py -= ( py + sy ) - screeny;
		m_pInfoLabel->SetPos( px, py );
	}
}