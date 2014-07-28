#ifndef CNODE_COMBO_H
#define CNODE_COMBO_H

#include "vBaseContainer.h"

class CNodeCombo : public CBaseContainerNode
{
	DECLARE_CLASS( CNodeCombo, CBaseContainerNode );

public:

	CNodeCombo( CNodeView *p );
	~CNodeCombo();

	virtual int GetNodeType(){ return HLSLNODE_CONTROLFLOW_COMBO; };

	virtual void Solve_ContainerEntered();
	virtual void Solve_ContainerLeft();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	bool VguiDraw( bool bShadow = false );

private:

	int m_iCondition;
	int m_iValue_Compare;
	bool m_bStatic;
	char *m_szComboName;
};


#endif