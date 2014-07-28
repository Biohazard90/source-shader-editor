#ifndef CNODE_CONDITION_H
#define CNODE_CONDITION_H

#include "vBaseContainer.h"

class CNodeCondition : public CBaseContainerNode
{
	DECLARE_CLASS( CNodeCondition, CBaseContainerNode );

public:

	CNodeCondition( CNodeView *p );
	~CNodeCondition();

	virtual int GetNodeType(){ return HLSLNODE_CONTROLFLOW_CONDITION; };

	virtual int UpdateInputsValid();

	virtual void Solve_ContainerEntered();
	virtual void Solve_ContainerLeft();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	//void VguiDraw( bool bShadow = false );

private:

	int m_iCondition;
};


#endif