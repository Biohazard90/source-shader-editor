#ifndef C_KV_PACKER_H
#define C_KV_PACKER_H

#include "cbase.h"
#include "editorCommon.h"


class CKVPacker
{
public:

	static char *ConvertKVSafeString( const char *pStr, bool bMakeEscaped );

	static bool KVPack( const char *pszStr, const char *pszKey, KeyValues *pKVTarget );
	static char *KVUnpack( KeyValues *pKVSource, const char *pszKey );
	static bool KVCopyPacked( KeyValues *pKVSource, KeyValues *pKVTarget, const char *pszKey );
	static void KVClearPacked( const char *pszKey, KeyValues *pKVTarget );
};



#endif