#ifndef VNODELOOP_H
#define VNODELOOP_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasecontainer.h"

class CNodeLoop : public CBaseContainerNode
{
	DECLARE_CLASS( CNodeLoop, CBaseContainerNode );

public:

	CNodeLoop( CNodeView *p );
	~CNodeLoop();

	virtual int GetNodeType(){ return HLSLNODE_CONTROLFLOW_LOOP; };

	virtual void Solve_ContainerEntered();
	virtual void Solve_ContainerLeft();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:

	int m_iCondition;
	int m_iValue_Start;
	int m_iValue_End;
	bool bDecrement;
};

#endif
