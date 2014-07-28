#ifndef CNODE_COMMENT_H
#define CNODE_COMMENT_H

#include "vBaseContainer.h"

class CNodeComment : public CBaseContainerNode
{
	DECLARE_CLASS( CNodeComment, CBaseContainerNode );

public:

	CNodeComment( CNodeView *p );
	~CNodeComment();

	virtual Vector2D GetContainerSensitiveCenter();

	virtual const int GetAllowedFlowgraphTypes();
	virtual int GetNodeType(){ return HLSLNODE_OTHER_COMMENT; };

	virtual bool ShouldSelectChildrenOnClick(){ return true; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	virtual Vector2D GetSelectionBoundsMinNodeSpace();
	virtual Vector2D GetSelectionBoundsMaxNodeSpace();

	virtual int IsInBorderBounds( const Vector2D &pos );
	virtual bool IsWithinBounds_Base( const Vector2D &pos );
	virtual void UpdateSize();

	bool VguiDraw( bool bShadow = false );

	int GetErrorLevel(){ return ERRORLEVEL_NONE; };

protected:
	virtual float GetBorderSize(){ return CBORDER_SIZE_COMMENT; };

	virtual bool IsInBounds_DragMove( const Vector2D &pos );

private:

	TokenChannels_t m_iChannelInfo;
	char m_szConstantString[MAX_PATH];
};


#endif