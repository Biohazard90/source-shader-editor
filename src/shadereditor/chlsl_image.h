#ifndef CHLSL_IMAGE_H
#define CHLSL_IMAGE_H

#include "vtf/vtf.h"
#include "editorCommon.h"

class CHLSL_Image
{
public:

	CHLSL_Image();
	~CHLSL_Image();

	void DestroyImage();
	bool LoadFromVTF( const char *path );

	bool IsEnvmap(){ return bEnvmap; };
	IVTFTexture *GetVTF(){return pVtf;};

	void InitProceduralMaterial();
	void DestroyProceduralMaterial();

	static void CreateScreenshot( CNodeView *n, const char *filepath );

private:
	IVTFTexture *pVtf;
	bool bEnvmap;

	IMaterial *pProcMat;
	KeyValues *pKVM;
};



#endif