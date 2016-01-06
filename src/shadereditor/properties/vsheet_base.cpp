
#include "cbase.h"
#include "vsheets.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CSheet_Base::CSheet_Base(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent ) : PropertyPage( parent, "" )
{
	n = pNode;
	pData = data;
	pView = view;
	szDynamicTitle = NULL;
}
CSheet_Base::~CSheet_Base()
{
	delete [] szDynamicTitle;
}

CBaseNode *CSheet_Base::GetNode()
{
	return n;
}
KeyValues *CSheet_Base::GetData()
{
	return pData;
}

void CSheet_Base::SetDynamicTitle( const char *name )
{
	delete [] szDynamicTitle;
	szDynamicTitle = new char[Q_strlen(name)+1];
	Q_strcpy( szDynamicTitle, name );
}
