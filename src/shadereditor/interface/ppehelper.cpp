
#include "cbase.h"
#include "editorCommon.h"


static CPPEHelper __ppeHelper_Instance;
CPPEHelper *gPPEHelper = &__ppeHelper_Instance;

CPPEHelper::CPPEHelper()
{
}

CPPEHelper::~CPPEHelper()
{
}

void CPPEHelper::DestroyKeyValues( KeyValues *pKV )
{
	Assert( pKV );

	pKV->deleteThis();
}

KeyValues *CPPEHelper::GetInlineMaterial( const char *szmatName )
{
	if ( !szmatName )
		return NULL;

	for ( int i = 0; i < GetPPCache()->GetNumPostProcessingEffects(); i++ )
	{
		CUtlVector<CHLSL_SolverBase*> &solver = GetPPCache()->GetPostProcessingEffect(i)->hSolverArray;

		KeyValues *pKV = GetInlineMaterial( szmatName, solver );
		if ( pKV != NULL )
			return pKV;
	}

	if ( IsInEditMode() )
	{
		for ( int i = 0; i < pEditorRoot->GetNumFlowGraphs(); i++ )
		{
			CNodeView *pView = pEditorRoot->GetFlowGraph( i );

			if ( pView->GetFlowgraphType() != CNodeView::FLOWGRAPH_POSTPROC )
				continue;

			CUtlVector<CHLSL_SolverBase*> &solver = pView->AccessSolverStack_POSTPROC();

			KeyValues *pKV = GetInlineMaterial( szmatName, solver );
			if ( pKV != NULL )
				return pKV;
		}
	}

	Assert(0);

	return NULL;
}

KeyValues *CPPEHelper::GetInlineMaterial( const char *szmatName, CUtlVector<CHLSL_SolverBase*> &hStack )
{
	for ( int s = 0; s < hStack.Count(); s++ )
	{
		CHLSL_Solver_PP_Mat *pMatSolver = dynamic_cast< CHLSL_Solver_PP_Mat* >( hStack[s] );
		if ( !pMatSolver )
			continue;

		if ( !pMatSolver->IsInline() )
			continue;

		if ( !pMatSolver->GetNumTargetVars() )
			continue;

		if ( !pMatSolver->GetMaterialName() || Q_strcmp( szmatName, pMatSolver->GetMaterialName() ) )
			continue;

		return pMatSolver->AccessKeyValues();
	}

	return NULL;
}