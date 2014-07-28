
#include "cbase.h"
#include "vSheets.h"


CNodePropertySheet::CNodePropertySheet( CBaseNode *pNode, CNodeView *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	n = pNode;
	pNodeView = parent;

	SetSize( 480, 400 );
	MoveToCenterOfScreen();

	SetApplyButtonVisible( false );
	//DoModal();

	SetTitle( "Properties", false );

	pKV_NodeSettings = GetNode()->AllocateKeyValues(0);
	pKV_NodeSettings_Original = pKV_NodeSettings->MakeCopy();
}

CNodePropertySheet::~CNodePropertySheet()
{
	if ( pKV_NodeSettings )
		pKV_NodeSettings->deleteThis();

	if ( pKV_NodeSettings_Original )
		pKV_NodeSettings_Original->deleteThis();
}

CBaseNode *CNodePropertySheet::GetNode()
{
	return n;
}

KeyValues *CNodePropertySheet::GetPropertyContainer()
{
	return pKV_NodeSettings;
}

void CNodePropertySheet::ResetAllData()
{
	BaseClass::ResetAllData();
}

bool CNodePropertySheet::OnOK(bool applyOnly)
{
	bool b = BaseClass::OnOK(applyOnly);

	if ( GetPropertySheet() )
	{
		for ( int i = 0; i < GetPropertySheet()->GetNumPages(); i++ )
			PostMessage( GetPropertySheet()->GetPage(i), new KeyValues( "RestoreNode" ) );
	}

	GetNode()->RestoreFromKeyValues( pKV_NodeSettings );
	//GetNode()->RestoreFromKeyValues_CreateBridges( pKV_NodeSettings );

	pNodeView->MakeSolversDirty();

	return b;
}

void CNodePropertySheet::OnCancel()
{
	bool bDoReset = false;
	for ( int i = 0; i < GetPropertySheet()->GetNumPages(); i++ )
	{
		CSheet_Base *pSheet = assert_cast< CSheet_Base* >( GetPropertySheet()->GetPage( i ) );

		if ( pSheet->RequiresReset() )
			bDoReset = true;
	}

	if ( bDoReset )
	{
		GetNode()->RestoreFromKeyValues( pKV_NodeSettings_Original );

		pNodeView->MakeSolversDirty();
	}

	BaseClass::OnCancel();
}
