#ifndef CBRIDGE_H
#define CBRIDGE_H

#include <vgui/VGUI.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include "ienginevgui.h"
#include "vgui_controls/Controls.h"

class CJack;
class CBaseNode;
class CNodeView;

class CBridge
{
public:
	CBridge( CNodeView *p );
	~CBridge();

	void ConnectSource( CJack *j, CBaseNode *n );
	void ConnectDestination( CJack *j, CBaseNode *n );

	void DisconnectSource();
	void DisconnectDestination();

	void SetTemporaryTarget( Vector2D tg );

	virtual void VguiDraw( const bool bShadow = false );

	CBaseNode *GetEndNode( CBaseNode *n );
	CBaseNode *GetEndNode( CJack *j );
	CJack *GetEndJack( CBaseNode *n );
	CJack *GetEndJack( CJack *j );

	CJack *GetInputJack(){ return pJ_Src; };
	CJack *GetDestinationJack(){ return pJ_Dst; };
	CBaseNode *GetInputNode(){ return pN_Src; };
	CBaseNode *GetDestinationNode(){ return pN_Dst; };

	const int GetErrorLevel(){ return iErrorLevel; };
	void SetErrorLevel(const int e){ iErrorLevel = e; };

	//void MakeBridgeVolatile(){ bDrawOrange = true; };
	enum
	{
		TMPCOL_NONE = 0,
		TMPCOL_TRANS,
		TMPCOL_ORANGE,
	};
	void SetTemporaryColor( int mode );

private:
	Vector2D tmp;
	CNodeView *pNodeView;
	int iErrorLevel;
	int iTempColor;
	//bool bDrawOrange;

	CJack *pJ_Src;
	CJack *pJ_Dst;

	CBaseNode *pN_Src;
	CBaseNode *pN_Dst;

	int m_iTex_Arrow;

	float m_flLength;
	Vector2D start_old;
	Vector2D end_old;
};


#endif