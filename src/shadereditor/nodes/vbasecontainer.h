#ifndef CCONTAINER_NODE_H
#define CCONTAINER_NODE_H

#include "vBaseNode.h"

#define CBORDER_SIZE 10.0f
#define CBORDER_SIZE_COMMENT 5.0f

#define GRABICO_OFFSET 1.5f

enum
{
	CBORDER_INVALID = 0,
	CBORDER_TOP_LEFT,
	CBORDER_TOP,
	CBORDER_TOP_RIGHT,
	CBORDER_RIGHT,
	CBORDER_BOTTOM_RIGHT,
	CBORDER_BOTTOM,
	CBORDER_BOTTOM_LEFT,
	CBORDER_LEFT,

	CBORDER_MAX,
};

#define CBORDER_FIRST CBORDER_TOP_LEFT
#define CBORDER_LAST CBORDER_LEFT

class CBaseContainerNode : public CBaseNode
{
	DECLARE_CLASS( CBaseContainerNode, CBaseNode );

public:
	CBaseContainerNode( const char *opName, CNodeView *p );
	//CBaseContainerNode( CNodeView *p );
	virtual ~CBaseContainerNode();
	virtual void Spawn();

	virtual int GetNodeType(){ return HLSLNODE_INVALID; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual void UpdateSize();
	virtual void UpdateContainerBounds();
	virtual void SetPosition( Vector2D vec, bool bCenter = false );
	virtual bool VguiDraw( bool bShadow = false );
	virtual void FullHierachyUpdate();
	virtual bool TestFullHierachyUpdate();

	virtual void OnLeftClick( Vector2D &pos );
	virtual void OnDrag( Vector2D &delta );
	virtual void OnDragEnd();
	virtual bool ShouldSelectChildrenOnClick(){ return false; };
	void SelectAllInBounds( bool bSelected = true, CUtlVector<CBaseNode*>*hNodes = NULL );


	virtual CBaseContainerNode *GetAsContainer();
	virtual bool HasAnyChildren();
	//virtual bool IsSolvable( bool bHierachyUp, CBaseNode *pCaller, CUtlVector< CBaseNode* > *hTested = NULL );
	virtual bool IsSolvable( bool bHierachyUp,
							CUtlVector< CBaseNode* > *hCallerList = NULL,
							CUtlVector< CBaseNode* > *hTested = NULL );

	virtual bool IsWithinBounds_Base( const Vector2D &pos );
	virtual Vector2D GetContainerMins();
	virtual Vector2D GetContainerMaxs();
	virtual bool IsInContainerBounds( const Vector2D &pos );
	virtual Vector2D GetContainerBorderMin( int mode );
	virtual Vector2D GetContainerBorderMax( int mode );
	virtual int IsInBorderBounds( const Vector2D &pos );

	virtual bool HasChild( CBaseNode *child );
	virtual void RemoveAllChildren();
	virtual void RemoveChild( CBaseNode *child );
	virtual void AddChild( CBaseNode *child );
	virtual int GetNumChildren(){ return m_hChildren.Count(); };
	virtual CBaseNode *GetChild( int i ){ Assert(m_hChildren.IsValidIndex(i)); return m_hChildren[i]; };
	virtual void UpdateOnMove();

	virtual bool MustDragAlone();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	virtual void Solve_ContainerEntered();
	virtual void Solve_ContainerCreateVariables();
	virtual void Solve_ContainerLeft();

protected:
	virtual float GetBorderSize(){ return CBORDER_SIZE; };

	Vector m_vecContainerExtents;
	bool m_bAffectSolver;

	void DrawGrabIcon();

private:
	bool IsAffectingSolver();

	bool bLastHierachyErrorTest;
	CUtlVector< CBaseNode* > m_hChildren;

	int m_iActiveScalingMode;
	int m_iGrabIcon;
	Vector2D m_vecMouseStartPos;
	float m_vecAccumulatedExtent[5];

};


#endif