#ifndef SMART_OBJECT_H
#define SMART_OBJECT_H

#include "cbase.h"

// #define A 1

// [const] [static] [datatype] blah [register];

// [datatype] func ([params])

// struct blah { [datatype] [varname]; };

enum
{
	ACOMP_INVALID = -1,
	ACOMP_DEFINE = 0,
	ACOMP_VAR,
	ACOMP_FUNC,
	ACOMP_STRUCT_DEF,
	ACOMP_,
};

class CSmartObject
{
public:

	CSmartObject( int type, const char *pszDef, const char *pszIdent = NULL, const char *pszDatatype = NULL );
	~CSmartObject();
	CSmartObject( const CSmartObject &o );

	inline bool operator == ( const CSmartObject &o );
	inline bool operator != ( const CSmartObject &o );

	void DebugPrint( int stage = 0 );

	int m_iType;

	char *m_pszDefinition;
	char *m_pszIdentifier;
	char *m_pszDatatype;
	char *m_pszHelptext;
	bool m_bHelpLocked;

	int m_iTextOffset;
	int m_iScopeDepth;
	int m_iScopeNum;

	char *m_pszSourceFile;

	CUtlVector< CSmartObject* >m_hChildren;

	bool IsObjectVisible( int textPos, int iScopeDepth, int iScopeNum,
		const char *pSourceFile );
	bool IsObjectVisible( CSmartObject *pViewer );

	bool m_bIntrinsic;
};


bool CSmartObject::operator == ( const CSmartObject &o )
{
	if ( !m_pszIdentifier || !o.m_pszIdentifier )
	{
		Assert( 0 );
		return false;
	}
	if ( !m_pszDatatype || !o.m_pszDatatype )
	{
		Assert( 0 );
		return false;
	}

	bool bSameName = !Q_stricmp(m_pszIdentifier,o.m_pszIdentifier);
	bool bSameDatatype = !Q_stricmp(m_pszDatatype,o.m_pszDatatype);

	if ( bSameName && bSameDatatype )
		return true;

	return false;
}
bool CSmartObject::operator != ( const CSmartObject &o )
{
	return !( *this == o );
}

#endif