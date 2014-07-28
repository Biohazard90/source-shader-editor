#ifndef PPE_HELPER_H
#define PPE_HELPER_H

#include "cbase.h"


class CPPEHelper : public IVPPEHelper
{
public:

	CPPEHelper();
	~CPPEHelper();

	KeyValues *GetInlineMaterial( const char *szmatName );
	void DestroyKeyValues( KeyValues *pKV );

private:

	KeyValues *GetInlineMaterial( const char *szmatName, CUtlVector<CHLSL_SolverBase*> &hStack );

};



#endif