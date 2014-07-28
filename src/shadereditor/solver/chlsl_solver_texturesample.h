#ifndef CHLSL_SOLVER_TEXSAMPLE_H
#define CHLSL_SOLVER_TEXSAMPLE_H

#include "editorcommon.h"

enum
{
	TEXSAMPLER_LOOKUPMODE_RGBA = 0,
	TEXSAMPLER_LOOKUPMODE_RGB,
	TEXSAMPLER_LOOKUPMODE_R,
	TEXSAMPLER_LOOKUPMODE_G,
	TEXSAMPLER_LOOKUPMODE_B,
	TEXSAMPLER_LOOKUPMODE_A,
	TEXSAMPLER_LOOKUPMODE_LAST,
};

class CHLSL_Solver_TextureSample : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_TextureSample( HNODE nodeidx );
	~CHLSL_Solver_TextureSample();
	CHLSL_Solver_TextureSample( const CHLSL_Solver_TextureSample& o );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_TextureSample( *this ); };

	void SetLookupMode( int i ){ iLookupMode = i; };
	void SetTextureMode( int i ){ iTextureMode = i; };
	void SetCubemap( bool c ){ bCubemap = c; };
	void SetSrgb( bool s ){ bSrgb = s; };
	void SetFunction( int f ){ iFunctionOverride = f; };

	void SetTextureName( const char *n );
	void SetParamName( const char *n );
	void SetFallbackName( const char *n );
	void SetRenderMaterial( IMaterial *pM );

	int GetTextureMode(){ return iTextureMode; };

	void MakeSamplerObject();
	//virtual bool IsRenderable(){ return m_bIsSamplerObject == false; };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnRenderInit();
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	bool m_bIsSamplerObject;
	SimpleTexture *GetActiveTexture( const WriteContext_FXC &context,
		bool *bHasSolverSiblings = NULL, bool *bIsFirstSibling = NULL, int *lookupIndex = NULL );

	void CreateSolverMaterial();
	void DestroySolverMaterial();

	int iLookupMode;
	int iTextureMode;
	int iFunctionOverride;
	bool bCubemap;
	char *szCustomTextureName;
	char *szCustomParamName;
	char *szFallbackName;
	bool bSrgb;

	int lastSourceIndex;
	IMaterial *pRenderMaterial;
	KeyValues *pKV;
};

#endif