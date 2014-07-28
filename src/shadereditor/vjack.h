#ifndef CJACK_H
#define CJACK_H

#include <vgui/VGUI.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include "ienginevgui.h"
#include "vgui_controls/Controls.h"

class CBridge;
class CBaseNode;
class CNodeView;
class CHLSL_Var;

#define JACK_SIZE_Y 6
#define JACK_SIZE_X 8
#define JACK_DELTA_Y 5

class CJack
{
public:
	CJack( CBaseNode *p, int slot, bool input );
	~CJack();

	virtual void SetName( const char *name );
	const char *GetName(){ return szName; };
	CBaseNode *GetParentNode(){ return pNode; };

	const ResourceType_t &GetResourceType();
	void SetResourceType( const int &type );
	CHLSL_Var *AllocateVarFromSmartType( bool bAssigntoSelf = true );

	void ConnectBridge( CBridge *b );
	void OnBridgeRemoved( CBridge *b );
	void PurgeBridges();

	virtual void ClearVarFlags();
	virtual void AddVarFlags( int f );
	virtual void RemoveVarFlags( int f );
	virtual bool HasVarFlag( int f );

	virtual int GetSmartType(){ return m_iSmartType; };
	virtual void SetSmartType( const int t );
	//virtual void SetSmartTypeFromFlag( const int flag );
	virtual void SetSmartTypeLocked( const bool b );
	virtual const bool IsSmartTypeLocked(){ return bLockSmartType; };
	virtual void UpdateSmartType( CBridge *b );
	
	virtual void SetJackType( const int t ){ m_iJackType = t; };
	virtual const int GetJackType(){ return m_iJackType; };
	virtual void SetJackColorCode( int t ){ m_iJackColorCode = t; };
	virtual const int GetJackColorCode(){ return m_iJackColorCode; };

	virtual void VguiDraw( bool bShadow = false );
	virtual void VguiDrawName();

	virtual Vector2D GetBoundsMin();
	virtual Vector2D GetBoundsMax();
	virtual Vector2D GetCenter();
	virtual Vector2D GetBoundsMinPanelSpace();
	virtual Vector2D GetBoundsMaxPanelSpace();
	virtual Vector2D GetCenterPanelSpace();
	virtual bool IsWithinBounds_Base( Vector2D pos );

	virtual void UpdatePositionData();
	virtual void UpdatePosition();
	virtual void SetPosition( Vector2D vec, bool bCenter = false );
	virtual Vector2D GetPosition();

	virtual const bool IsInput(){ return m_bInput; };
	virtual const bool IsOutput(){ return !m_bInput; };
	virtual const int GetSlot(){ return m_iSlot; };

	virtual void OnParentMoved();

	virtual const bool HasFocus(){ return bHasFocus; };
	virtual void SetFocus( const bool b ){ bHasFocus = b; };

	virtual int GetFinalTextInset();

	const int GetNumBridges();
	int GetNumBridgesConnected();
	CBridge *GetBridge( const int idx ){ return m_hBridgeList[idx]; };
	CBridge *BridgeBeginBuild();
	void BridgeEndBuild( CBridge *pBridge );
	void DisconnectBridges();

	void SetTemporaryVarTarget( CHLSL_Var *var );
	CHLSL_Var *GetTemporaryVarTarget();
	//// for inputs!
	CHLSL_Var *GetTemporaryVarTarget_End();
	CHLSL_Var *GetTemporaryVarTarget_End_Smart( int varTypes, bool bAllowOverwrite = false );
	////
	void ClearTemporaryVarTarget();

protected:
	char szName[MAX_PATH];

	int m_iJackType;
	int m_iJackColorCode;
	int m_iSmartType;
	bool bLockSmartType;

	Vector2D m_vecPosition;
	Vector2D m_vecSize;
	
	CBaseNode *pNode;
	CNodeView *pNodeView;
	CUtlVector< CBridge* >m_hBridgeList;
	CHLSL_Var *m_pVolatileVar;

	bool bHasFocus;

	bool m_bInput;
	int m_iSlot;
	ResourceType_t m_iResourceType;
};



#endif