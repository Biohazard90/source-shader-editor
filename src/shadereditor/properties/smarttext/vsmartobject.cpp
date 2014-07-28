
#include "cbase.h"
#include "editorcommon.h"
#include "vSmartObject.h"


CSmartObject::CSmartObject( int type, const char *pszDef, const char *pszIdent, const char *pszDatatype )
{
	m_iType = type; //ACOMP_INVALID;

	AutoCopyStringPtr( pszDef, &m_pszDefinition );
	AutoCopyStringPtr( pszIdent, &m_pszIdentifier );
	AutoCopyStringPtr( pszDatatype, &m_pszDatatype );
	m_pszHelptext = NULL;
	m_pszSourceFile = NULL;

	m_iTextOffset = -1;
	m_bHelpLocked = false;
	m_bIntrinsic = false;
	m_iScopeDepth = 0;
	m_iScopeNum = 0;
}
CSmartObject::~CSmartObject()
{
	m_hChildren.PurgeAndDeleteElements();

	delete [] m_pszDefinition;
	delete [] m_pszIdentifier;
	delete [] m_pszDatatype;
	delete [] m_pszHelptext;
	delete [] m_pszSourceFile;
}
CSmartObject::CSmartObject( const CSmartObject &o )
{
	m_iType = o.m_iType;
	m_iTextOffset = o.m_iTextOffset;
	m_iScopeDepth = o.m_iScopeDepth;
	m_iScopeNum = o.m_iScopeNum;
	m_bHelpLocked = o.m_bHelpLocked;
	m_bIntrinsic = o.m_bIntrinsic;

	m_pszDefinition = NULL;
	m_pszIdentifier = NULL;
	m_pszDatatype = NULL;
	m_pszHelptext = NULL;
	m_pszSourceFile = NULL;

	if ( o.m_pszDefinition )
		AutoCopyStringPtr( o.m_pszDefinition, &m_pszDefinition );
	if ( o.m_pszIdentifier )
		AutoCopyStringPtr( o.m_pszIdentifier, &m_pszIdentifier );
	if ( o.m_pszDatatype )
		AutoCopyStringPtr( o.m_pszDatatype, &m_pszDatatype );
	if ( o.m_pszHelptext )
		AutoCopyStringPtr( o.m_pszHelptext, &m_pszHelptext );
	if ( o.m_pszSourceFile )
		AutoCopyStringPtr( o.m_pszSourceFile, &m_pszSourceFile );

	for ( int i = 0; i < o.m_hChildren.Count(); i++ )
		m_hChildren.AddToTail( new CSmartObject( *o.m_hChildren[i] ) );

	Assert( m_hChildren.Count() == o.m_hChildren.Count() );
}

void CSmartObject::DebugPrint( int stage )
{
	Assert( stage <= 15 );

	if ( stage > 15 )
		return;

	const char *pTypeNames[] =
	{
		"invalid",
		"def",
		"var",
		"func",
		"struct",
	};

	Assert( ARRAYSIZE( pTypeNames ) == ACOMP_ + 1 );

	Msg( "Autocompunit:: type:        %s\n", pTypeNames[ m_iType + 1 ] );
	if ( m_pszHelptext )
		Warning( "Help:\n%s", m_pszHelptext );
	if ( m_pszDefinition )
		Msg( "               definition:  %s\n", m_pszDefinition );
	if ( m_pszIdentifier )
		Msg( "               identifier:  %s\n", m_pszIdentifier );
	if ( m_pszDatatype )
		Msg( "               datatype:    %s\n", m_pszDatatype );
	if ( m_hChildren.Count() )
	{
		Msg( "CHILDREN START >> now at level %02i\n", stage );

		stage++;
		for ( int i = 0; i < m_hChildren.Count(); i++ )
		{
			m_hChildren[ i ]->DebugPrint( stage );
		}
		Msg( "CHILDREN END <<\n" );
	}
	Msg( "\n" );
}

bool CSmartObject::IsObjectVisible( int textPos, int iScopeDepth, int iScopeNum,
		const char *pSourceFile )
{
	bool bDifferentFile = true;

	const char *pszObjectSrc = m_pszSourceFile;
	if ( pszObjectSrc && pSourceFile && m_iTextOffset >= 0 )
	{
		if ( !Q_stricmp( pszObjectSrc, pSourceFile ) )
		{
			if ( textPos < m_iTextOffset )
				return false;
			bDifferentFile = false;
		}
	}

	if ( bDifferentFile )
	{
		if ( m_iScopeDepth > 0 )
			return false;
	}
	else if ( m_iScopeDepth > iScopeDepth ||
		m_iScopeDepth == iScopeDepth && m_iScopeNum != iScopeNum )
		return false;

	return true;
}

bool CSmartObject::IsObjectVisible( CSmartObject *pViewer )
{
	return IsObjectVisible( m_iTextOffset, m_iScopeDepth, m_iScopeNum, m_pszSourceFile );
}
