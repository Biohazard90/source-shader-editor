
#include "cbase.h"
#include "editorCommon.h"

#include "view_shared.h"
#include "materialsystem/imesh.h"


CRenderPanel::CRenderPanel( Panel *parent, const char *pElementName ) : BaseClass( parent, pElementName )
{
	render_ang.Init();
	render_pos.Init();
	render_offset.Init();
	render_offset_modelBase.Init();

	m_bDoMatOverride = true;
	m_bDoPPE = true;
	ResetView();

	m_iDragMode = RDRAG_NONE;
	m_iCachedMpos_x = 0;
	m_iCachedMpos_y = 0;

	CreateMesh( PRENDER_SPHERE );

	__view.Identity();
	__proj.Identity();
	__ViewProj.Identity();
	__ViewProjNDC.Identity();

	m_iRendermode = 0;
	
	SetupVguiTex( m_iVguitex_pp0, "shadereditor/_rtdebug_pp0" );
	SetupVguiTex( m_iVguitex_postprocPrev, "shadereditor/_rt_framebuffer_prev" );
}
CRenderPanel::~CRenderPanel()
{
}

void CRenderPanel::CreateMesh( int mode )
{
	switch ( mode )
	{
	case PRENDER_SPHERE:
		_mesh.CreateSphere( sedit_mesh_size.GetFloat(), sedit_mesh_subdiv.GetInt() );
		break;
	case PRENDER_CUBE:
		_mesh.CreateCube( sedit_mesh_size.GetFloat(), sedit_mesh_subdiv.GetInt() );
		break;
	case PRENDER_PLANE:
		_mesh.CreatePlane( sedit_mesh_size.GetFloat(), sedit_mesh_subdiv.GetInt() );
		break;
	case PRENDER_CYLINDER:
		_mesh.CreateCylinder( sedit_mesh_size.GetFloat(), sedit_mesh_subdiv.GetInt() );
		break;
	}
}

void CRenderPanel::SetRenderMode( int mode )
{
	m_iRendermode = mode;
	if ( m_iRendermode == PRENDER_MODEL && sEditMRender )
		sEditMRender->GetModelCenter( render_offset_modelBase.Base() );
	else
		render_offset_modelBase.Init();
}

void CRenderPanel::UpdateRenderPosition()
{
	QAngle out( m_flPitch, m_flYaw, 0 );
	Vector fwd;
	AngleVectors( out, &fwd );

	render_pos = vec3_origin - fwd * m_flDist;
	VectorAngles( fwd, render_ang );

	render_pos += render_offset;
	if ( m_iRendermode == PRENDER_MODEL )
		render_pos += render_offset_modelBase;
}

void CRenderPanel::OnThink()
{
	const bool bIsRenderingModel = m_iRendermode == PRENDER_MODEL;

	int iInset_base = 15;
	int iInset_Top = 70;
	if ( bIsRenderingModel )
		iInset_Top = 110;

	int parentSx, parentSy;
	if ( GetParent() )
	{
		GetParent()->GetSize( parentSx, parentSy );
		parentSx -= iInset_base * 2;
		parentSy -= iInset_base + iInset_Top;

		SetBounds( iInset_base, iInset_Top, parentSx, parentSy );
	}

	//if ( bIsRenderingModel && sEditMRender )
	//	sEditMRender->GetModelCenter( render_offset.Base() );

	if ( m_iDragMode )
	{
		int mdelta_x, mdelta_y;
		input()->GetCursorPosition( mdelta_x, mdelta_y );
		mdelta_x -= m_iCachedMpos_x;
		mdelta_y -= m_iCachedMpos_y;
		input()->SetCursorPos( m_iCachedMpos_x, m_iCachedMpos_y );
		switch ( m_iDragMode )
		{
		case RDRAG_LIGHT:
			{
				VMatrix viewInv, viewInvT;
				MatrixInverseGeneral( __view, viewInv );
				matrix3x4_t rot_x, rot_y, rot_comb;
				Vector fwd, right, up;

				viewInv.GetBasisVectors( right, up, fwd );

#ifdef SHADER_EDITOR_DLL_2006
				VMatrix rot_tmp;
				MatrixBuildRotationAboutAxis( rot_tmp, right, mdelta_y );
				rot_x = rot_tmp.As3x4();
				MatrixBuildRotationAboutAxis( rot_tmp, up, mdelta_x );
				rot_y = rot_tmp.As3x4();
#else
				MatrixBuildRotationAboutAxis( right, mdelta_y, rot_x );
				MatrixBuildRotationAboutAxis( up, mdelta_x, rot_y );
				ConcatTransforms( rot_x, rot_y, rot_comb );
#endif
				QAngle rot( -mdelta_y, mdelta_x,0 );
				Vector tmp,tmp2;
				AngleVectors( lightAng, &tmp );
				//VectorRotate(tmp,rot,tmp2);
				VectorRotate(tmp,rot_comb,tmp2);
				VectorAngles(tmp2,lightAng);
			}
			break;
		case RDRAG_ROTATE:
			{
				m_flPitch -= mdelta_y * 0.5f;
				m_flYaw += mdelta_x * 0.5f;

				m_flPitch = clamp( m_flPitch, -89, 89 );
				if ( m_flYaw > 180.0f )
					m_flYaw -= 360.0f;
				if ( m_flYaw < -180.0f )
					m_flYaw += 360.0f;
			}
			break;
		case RDRAG_POS:
			{
#define RENDER_DRAGPOS_MOVESCALE 0.2f
				Vector viewRight, viewUp;
				AngleVectors( render_ang, NULL, &viewRight, &viewUp );
				render_offset += mdelta_x * -viewRight * RENDER_DRAGPOS_MOVESCALE +
									mdelta_y * viewUp * RENDER_DRAGPOS_MOVESCALE;

#ifdef SHADER_EDITOR_DLL_SWARM
				if ( !input()->IsMouseDown( MOUSE_MIDDLE ) )
					OnMouseReleased( MOUSE_MIDDLE );
#endif
			}
			break;
		}
	}
}
void CRenderPanel::ResetView()
{
	const bool bIsRenderingModel = m_iRendermode == PRENDER_MODEL;

	m_flDist = 128;
	m_flPitch = 45;
	m_flYaw = 180;
	lightAng.Init( 45, -135, 0 );

	if ( bIsRenderingModel && sEditMRender )
		sEditMRender->GetModelCenter( render_offset_modelBase.Base() );
	else
		render_offset_modelBase.Init();
	render_offset.Init();

	UpdateRenderPosition();
}

void CRenderPanel::OnMouseWheeled( int delta )
{
	BaseClass::OnMouseWheeled( delta );

	float amt = RemapVal( m_flDist, 0, 100, 2, 25 );
	m_flDist -= delta * amt;
	m_flDist = clamp( m_flDist, 5, 16000 );
}

void CRenderPanel::OnMousePressed( MouseCode code )
{
	BaseClass::OnMousePressed( code );
	if ( m_iDragMode )
		return;

	if ( code == MOUSE_LEFT )
		m_iDragMode = RDRAG_ROTATE;
	else if ( code == MOUSE_RIGHT )
		m_iDragMode = RDRAG_LIGHT;
	else
		m_iDragMode = RDRAG_POS;

	input()->GetCursorPosition( m_iCachedMpos_x, m_iCachedMpos_y );
	input()->SetCursorOveride( dc_none );
	input()->SetMouseCapture( GetVPanel() );
}
void CRenderPanel::OnMouseReleased( MouseCode code )
{
	BaseClass::OnMouseReleased( code );
	if ( !m_iDragMode )
		return;

	input()->SetCursorOveride( NULL );
	input()->SetMouseCapture( NULL );
	m_iDragMode = RDRAG_NONE;
}
void CRenderPanel::OnCursorMoved( int x, int y )
{
}

void CRenderPanel::SetupView( CViewSetup &setup )
{
	UpdateRenderPosition();

	int x,y,w,t;
	GetSize( w, t );
	x = y = 0;
	LocalToScreen( x, y );
	if ( x < 0 )
		w += x;
	if ( y < 0 )
		t += y;
	x = max( 0, x );
	y = max( 0, y );

	setup.angles = render_ang;
	setup.origin = render_pos;

	setup.fov = 70;
	setup.m_bOrtho = false;

	setup.x = x;
	setup.y = y;

	setup.width = max(1, w);
	setup.height = max(1, t);

	setup.zNear = 1;
	setup.zFar = 10000;
}

void CRenderPanel::DrawBackground()
{
	VMatrix viewProjInv;
	MatrixInverseGeneral( __ViewProj, viewProjInv );

	const Vector vec_pos_start[4] = { 
		Vector(-1.01f, 1.01f, 1 ), // 00
		Vector( 1, 1.01f, 1 ), // 10
		Vector( 1,-1, 1 ), // 11
		Vector(-1.01f,-1, 1 )  // 01
	};
	Vector vec_pos[4];

	float _UVs[4][2] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};

	for ( int i = 0; i < 4; i++ )
	{
		Vector3DMultiplyPositionProjective( viewProjInv, vec_pos_start[i], vec_pos[i] );
		vec_pos[i] -= render_pos;
		//vec_pos[i] *= 0.1f;
		//vec_pos[i] += render_pos;
	}

	if ( !pEditorRoot->GetBGPreviewMat() )
		return;

	IMaterial *pMat = pEditorRoot->GetBGPreviewMat();
	CMatRenderContextPtr pRenderContext( materials );
	CMeshBuilder pMeshBuilder;

	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, 0, 0, pMat );
	pMeshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
#ifndef SHADER_EDITOR_DLL_2006
	pMeshBuilder.SetCompressionType( VERTEX_COMPRESSION_NONE );
#endif

	for ( int i = 0; i < 4; i++ )
	{
		//Vector _normal = render_pos - vec_pos[i];
		Vector _normal = -vec_pos[i];
		_normal.NormalizeInPlace();

		pMeshBuilder.Position3fv( vec_pos_start[i].Base() );
		pMeshBuilder.Normal3fv( _normal.Base() );
		pMeshBuilder.TexCoord2fv( 0, _UVs[i] );
		pMeshBuilder.AdvanceVertex();
	}

	pMeshBuilder.End();
	pMesh->Draw();
}

void CRenderPanel::SetupVert( CMeshBuilder &builder, CHLSL_Vertex &vert, float alpha )
{
	builder.Position3fv( vert.pos );
	builder.Normal3fv( vert.normal );
	builder.TangentS3fv( vert.tangent_s );
	builder.TangentT3f( -vert.tangent_t[0], -vert.tangent_t[1], -vert.tangent_t[2] );
	Vector4D tang_s( vert.tangent_s[0], vert.tangent_s[1], vert.tangent_s[2], vert.tangent_s[3] );
	builder.UserData( tang_s.Base() );

	GenericShaderData *data = pEditorRoot->GetSafeFlowgraph()->GetPreviewData();

	if ( !data )
	{
		builder.TexCoord2f( 0, vert.uv[0][0], vert.uv[0][1] );
	}
	else
	{
		for ( int i = 0; i < data->shader->iVFMT_numTexcoords && i < 3; i++ )
		{
			switch ( data->shader->iVFMT_texDim[i] )
			{
			default:
				Assert(0);
			case 1:
				builder.TexCoord1f( i, vert.uv[0][0] );
				break;
			case 2:
				builder.TexCoord2f( i, vert.uv[0][0], vert.uv[0][1] );
				break;
			case 3:
				builder.TexCoord3f( i, vert.uv[0][0], vert.uv[0][1], 0 );
				break;
#ifndef SHADER_EDITOR_DLL_2006
			case 4:
				builder.TexCoord4f( i, vert.uv[0][0], vert.uv[0][1], 0, 0 );
				break;
#endif
			}
		}
	}

	builder.Color4f( 1, 1, 1, alpha );
	builder.Specular4f( 0.3f, 0.3f, 0.3f, 1 );

#ifndef SHADER_EDITOR_DLL_2006
	builder.m_NumBoneWeights = 0;
#endif

	builder.AdvanceVertex();
}



void CRenderPanel::Paint()
{
	BaseClass::Paint();

#ifndef SHADER_EDITOR_DLL_2006
	modelrender->SuppressEngineLighting( true );
#endif

	CViewSetup setup;
	SetupView( setup );

	gShaderEditorSystem->UpdateConstants( &setup );

#ifdef SHADER_EDITOR_DLL_2006
	render->Push3DView( setup, 0, false, NULL, frustum );
	GetMatricesForView( setup, __view, __proj, __ViewProj );
#else
	render->Push3DView( setup, 0, NULL, frustum );
	render->GetMatricesForView( setup, &__view, &__proj, &__ViewProj, &__ViewProjNDC );
#endif
	CMatRenderContextPtr pRenderContext( materials );

	//ITexture *pCubemap = pEditorRoot->GetSafeFlowgraph()->GetEnvmapTexture();
	//if ( pCubemap )
	//	pRenderContext->BindLocalCubemap( pCubemap );
	pRenderContext->ClearColor4ub( 0, 0, 0, 0 );
	pRenderContext->ClearBuffers( false, true );

#ifndef SHADER_EDITOR_DLL_2006
	pRenderContext->SetLightingOrigin( vec3_origin );
#endif
	g_pStudioRender->SetLocalLights( 0, NULL );
	LightDesc_t inf;
	Vector lightDir;
	AngleVectors( lightAng, &lightDir );
#ifdef SHADER_EDITOR_DLL_2006
	InitLightDirectional( inf, lightDir, Vector( 1, 1, 1 ) );
#else
	inf.InitDirectional( lightDir, Vector( 1, 1, 1 ) );
#endif
	g_pStudioRender->SetLocalLights( 1, &inf );
#define AMBIENT_ 0.05f
	static Vector white[6] = 
	{
		Vector( AMBIENT_, AMBIENT_, AMBIENT_ ),
		Vector( AMBIENT_, AMBIENT_, AMBIENT_ ),
		Vector( AMBIENT_, AMBIENT_, AMBIENT_ ),
		Vector( AMBIENT_, AMBIENT_, AMBIENT_ ),
		Vector( AMBIENT_, AMBIENT_, AMBIENT_ ),
		Vector( AMBIENT_, AMBIENT_, AMBIENT_ ),
	};
	g_pStudioRender->SetAmbientLightColors( white );
#ifndef SHADER_EDITOR_DLL_SWARM
	pRenderContext->SetAmbientLight( AMBIENT_, AMBIENT_, AMBIENT_ );
	pRenderContext->SetLight( 0, inf );
#else
	pRenderContext->SetLights( 1, &inf );
#endif

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	render->SetColorModulation( color );
	render->SetBlend( 1.0f );

	ITexture *oldCopyTex = pRenderContext->GetFrameBufferCopyTexture( 0 );

	const bool bShouldDrawBG = m_iRendermode == PRENDER_SPHERE ||
		m_iRendermode == PRENDER_CUBE ||
		m_iRendermode == PRENDER_CYLINDER ||
		m_iRendermode == PRENDER_PLANE ||
		m_iRendermode == PRENDER_MODEL;

	Rect_t rSrc;
	rSrc.x = setup.x;
	rSrc.y = setup.y;
	rSrc.width = setup.width;
	rSrc.height = setup.height;

	if ( bShouldDrawBG )
	{
		shaderEdit->SetFramebufferCopyTexOverride( GetFBCopyTex() );

		if ( pEditorRoot &&
			pEditorRoot->GetSafeFlowgraph()->IsUsingBackgroundMaterial() )
			DrawBackground();

		if ( sedit_prev_HOMFun.GetInt() == 0 )
			pRenderContext->CopyRenderTargetToTextureEx( GetFBCopyTex(), 0, &rSrc );
	}

	MaterialFogMode_t oldFog = pRenderContext->GetFogMode();
	pRenderContext->FogMode( MATERIAL_FOG_NONE );

	switch ( m_iRendermode )
	{
	case PRENDER_SPHERE:
	case PRENDER_CUBE:
	case PRENDER_CYLINDER:
	case PRENDER_PLANE:
			DrawHardware();
		break;
	case PRENDER_MODEL:
			DrawModel();
		break;
	}

	pRenderContext->FogMode( oldFog );

	if ( bShouldDrawBG )
		pRenderContext->CopyRenderTargetToTextureEx( GetFBCopyTex(), 0, &rSrc );

	//if ( sEditMRender )
	//	sEditMRender->DoPostProc( setup.x, setup.y, setup.width, setup.height );

	if ( bShouldDrawBG )
		pRenderContext->SetFrameBufferCopyTexture( oldCopyTex, 0 );

	render->PopView( frustum );
#ifndef SHADER_EDITOR_DLL_2006
	modelrender->SuppressEngineLighting( false );
#endif

	switch ( m_iRendermode )
	{
	case PRENDER_PINGPONG_0:
			DrawPingpongRTs();
		break;
	case PRENDER_POSTPROC:
	case PRENDER_PPECHAIN:
			DrawPostProcPreview();
		break;
	}

	gShaderEditorSystem->UpdateConstants();
}
void CRenderPanel::DrawPostProcPreview()
{
	surface()->DrawSetTexture( m_iVguitex_postprocPrev );
	surface()->DrawSetColor( Color(255,255,255,255) );

	int w,t;
	GetSize(w,t);
	surface()->DrawTexturedRect( 0, 0, w, t );
}

void CRenderPanel::DrawScene()
{
	Assert( pEditorRoot );
	const bool bShaderAsPP = m_iRendermode == PRENDER_POSTPROC;
	const bool bDoPPE = m_iRendermode == PRENDER_PPECHAIN;

	if ( !bShaderAsPP && bDoPPE && m_bDoPPE && pEditorRoot->GetSafeFlowgraph()->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC )
	{
		CUtlVector< CHLSL_SolverBase* > &hSolverStack = pEditorRoot->GetSafeFlowgraph()->AccessSolverStack_POSTPROC();
		GenericPPEData *pPPECfg = pEditorRoot->GetSafeFlowgraph()->AccessPPEConfig();

		if ( hSolverStack.Count() && pPPECfg )
		{
			EditorPostProcessingEffect effect( hSolverStack, *pPPECfg, false );
			GetPPCache()->RenderSinglePPE( &effect, true, true );
		}
	}
}
void CRenderPanel::DrawPostProc()
{
	Assert( pEditorRoot );
	const bool bShaderAsPP = m_iRendermode == PRENDER_POSTPROC;
	const bool bDoPPE = m_iRendermode == PRENDER_PPECHAIN;

	if ( !bShaderAsPP && !bDoPPE )
		return;

	if ( bShaderAsPP )
	{
		CMatRenderContextPtr pRenderContext( materials );
		pRenderContext->PushRenderTargetAndViewport();

		GeneralFramebufferUpdate( pRenderContext );

		//shaderEdit->SetFramebufferCopyTexOverride( NULL );

		int w,t;
		engine->GetScreenSize( w, t );
		pRenderContext->DrawScreenSpaceRectangle(
			pEditorRoot->GetMainPreviewMat(),
			0, 0, w, t,
			0, 0, w-1, t-1,
			w, t );

		pRenderContext->PopRenderTargetAndViewport();
	}
	else
	{
		if ( m_bDoPPE && pEditorRoot->GetSafeFlowgraph()->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC )
		{
			CUtlVector< CHLSL_SolverBase* > &hSolverStack = pEditorRoot->GetSafeFlowgraph()->AccessSolverStack_POSTPROC();
			GenericPPEData *pPPECfg = pEditorRoot->GetSafeFlowgraph()->AccessPPEConfig();

			if ( hSolverStack.Count() && pPPECfg )
			{
				EditorPostProcessingEffect effect( hSolverStack, *pPPECfg, false );
				GetPPCache()->RenderSinglePPE( &effect, true );
			}
		}
	}

	shaderEdit->UpdateFramebufferTexture( true, true );
}
void CRenderPanel::DrawModel()
{
	if ( !sEditMRender )
		return;
	if ( !pEditorRoot || !pEditorRoot->GetMainPreviewMat() )
		return;
	IMaterial *pMat = pEditorRoot->GetMainPreviewMat();

	if ( m_bDoMatOverride )
		modelrender->ForcedMaterialOverride( pMat );
	sEditMRender->ExecRender();
	modelrender->ForcedMaterialOverride( NULL );
}
void CRenderPanel::DrawPingpongRTs()
{
	surface()->DrawSetTexture( m_iVguitex_pp0 );
	surface()->DrawSetColor( Color(255,255,255,255) );

	int w,t;
	GetSize(w,t);
	surface()->DrawTexturedRect( 0, 0, w, t );
}

//static ConVar sedit_debug_tangentspace( "sedit_debug_tangentspace", "1" );
void CRenderPanel::DrawHardware()
{
	if ( !pEditorRoot || !pEditorRoot->GetMainPreviewMat() )
		return;

	IMaterial *pMat = pEditorRoot->GetMainPreviewMat();
	//pMat = materials->FindMaterial( "shadereditor/tex", TEXTURE_GROUP_OTHER );

	float alpha = surface()->DrawGetAlphaMultiplier();

	CMatRenderContextPtr pRenderContext( materials );
	int numtris = _mesh.GetNumTriangles();
	CMeshBuilder pMeshBuilder;
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, 0, 0, pMat );
#ifndef SHADER_EDITOR_DLL_2006
	pMeshBuilder.SetCompressionType( VERTEX_COMPRESSION_NONE );
#endif
	pMeshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, numtris );
	for ( int i = 0; i < numtris; i++ )
	{
		CHLSL_Triangle &tri = _mesh.GetTriangles()[i];
		
		for ( int t = 0; t < 3; t++ )
			SetupVert( pMeshBuilder, *tri.vertices[t], alpha );
	}
	pMeshBuilder.End();
	pMesh->Draw();

#if 0
	if ( true )
	{
		pMat = materials->FindMaterial( "shadereditor/blank", TEXTURE_GROUP_OTHER );
		pMesh = pRenderContext->GetDynamicMesh( true, 0, 0, pMat );
		pMeshBuilder.Begin( pMesh, MATERIAL_LINES, _mesh.GetNumVertices() * 3 );
		pMeshBuilder.SetCompressionType( VERTEX_COMPRESSION_NONE );
		for ( int i = 0; i < _mesh.GetNumVertices(); i++ )
		{
			CHLSL_Vertex &vert = _mesh.GetVertices()[i];
			Vector pos;
			VectorCopy( vert.pos, pos.Base() );
			Vector normal;
			VectorCopy( vert.normal, normal.Base() );
			Vector tangent_s;
			VectorCopy( vert.tangent_s, tangent_s.Base() );
			Vector tangent_t;
			VectorCopy( vert.tangent_t, tangent_t.Base() );

			pos += normal * 2;
			normal = normal * 3 + pos;
			tangent_s = tangent_s * 3 + pos;
			tangent_t = tangent_t * 3 + pos;

			pMeshBuilder.Position3fv( pos.Base() );
			pMeshBuilder.Color3f( 0, 0, 1 );
			pMeshBuilder.AdvanceVertex();
			pMeshBuilder.Position3fv( normal.Base() );
			pMeshBuilder.Color3f( 0, 0, 1 );
			pMeshBuilder.AdvanceVertex();

			pMeshBuilder.Position3fv( pos.Base() );
			pMeshBuilder.Color3f( 1, 0, 0 );
			pMeshBuilder.AdvanceVertex();
			pMeshBuilder.Position3fv( tangent_s.Base() );
			pMeshBuilder.Color3f( 1, 0, 0 );
			pMeshBuilder.AdvanceVertex();

			pMeshBuilder.Position3fv( pos.Base() );
			pMeshBuilder.Color3f( 0, 1, 0 );
			pMeshBuilder.AdvanceVertex();
			pMeshBuilder.Position3fv( tangent_t.Base() );
			pMeshBuilder.Color3f( 0, 1, 0 );
			pMeshBuilder.AdvanceVertex();
		}
		pMeshBuilder.End();
		pMesh->Draw();
	}
#endif
}
