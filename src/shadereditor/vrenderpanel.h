#ifndef CRENDERPANEL_H
#define CRENDERPANEL_H

#include "cbase.h"
using namespace vgui;

#include <vgui_controls/Frame.h>
#include <vgui_controls/label.h>
#include <vgui_controls/textentry.h>
#include "editorCommon.h"
#include <vgui_controls/controls.h>

#include "ivrenderview.h"

enum
{
	PRENDER_SPHERE = 0,
	PRENDER_CUBE,
	PRENDER_CYLINDER,
	PRENDER_PLANE,
	PRENDER_MODEL,
	PRENDER_POSTPROC,
	PRENDER_PINGPONG_0,
	PRENDER_PPECHAIN,
};

class CMeshBuilder;

enum
{
	RDRAG_NONE = 0,
	RDRAG_ROTATE,
	RDRAG_LIGHT,
	RDRAG_POS,
};

class CRenderPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CRenderPanel, vgui::Panel );

public:
	CRenderPanel( Panel *parent, const char *pElementName );
	~CRenderPanel();

	virtual void OnThink();

	virtual void Paint();

	void SetupView( CViewSetup &setup );

	void OnMousePressed( MouseCode code );
	void OnMouseReleased( MouseCode code );
	void OnCursorMoved( int x, int y );
	void OnMouseWheeled( int delta );

	//void ReceiveRGBA( CHLSL_Image *img );

	QAngle render_ang;
	Vector render_pos;
	Vector render_offset;
	Vector render_offset_modelBase;
	QAngle lightAng;

	void UpdateRenderPosition();
	void CreateMesh( int mode );

	float m_flDist;
	float m_flPitch;
	float m_flYaw;

	Frustum frustum;
	int m_iDragMode;
	int m_iCachedMpos_x;
	int m_iCachedMpos_y;

	void SetupVert( CMeshBuilder &builder, CHLSL_Vertex &vert, float alpha );

	VMatrix __view;
	VMatrix __proj;
	VMatrix __ViewProj;
	VMatrix __ViewProjNDC;

	void SetRenderMode( int mode );
	void SetMatOverrideEnabled( bool b ){
		m_bDoMatOverride = b;
	};

	void DrawScene();
	void DrawPostProc();
	void SetPPEPreviewEnabled( bool bEnabled ){
		m_bDoPPE = bEnabled;
	};

	void ResetView();

private:
	CHLSL_Mesh _mesh;

	int m_iRendermode;
	bool m_bDoMatOverride;
	bool m_bDoPPE;

	void DrawBackground();
	void DrawHardware();
	void DrawPingpongRTs();
	void DrawModel();
	void DrawPostProcPreview();

	int m_iVguitex_pp0;
	int m_iVguitex_postprocPrev;
};


#endif