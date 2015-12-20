
#include "cbase.h"
#include "editorcommon.h"
#include "view_shared.h"

#include "vtf/vtf.h"
//#include "magick++.h"
//#ifdef SHADER_EDITOR_DLL_2006
#include "setjmp.h"
//#endif

#define JPEGLIB_USE_STDIO
#include "jpeglib/jpeglib.h"
#undef JPEGLIB_USE_STDIO

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

CHLSL_Image::CHLSL_Image()
{
	pVtf = NULL;
	bEnvmap = false;
	pKVM = NULL;
}

CHLSL_Image::~CHLSL_Image()
{
	DestroyImage();
}

void CHLSL_Image::DestroyImage()
{
	if ( pVtf )
		DestroyVTFTexture( pVtf );
	pVtf = NULL;
}


bool CHLSL_Image::LoadFromVTF( const char *path )
{
	pVtf = CreateVTFTexture();
	if ( !pVtf )
		return false;

	CUtlBuffer vtf;
	bool bSuccess = g_pFullFileSystem->ReadFile( path, 0, vtf ) && pVtf->Unserialize( vtf );
	vtf.Purge();

	if ( !bSuccess )
	{
		DestroyImage();
		return false;
	}

	pVtf->ConvertImageFormat( IMAGE_FORMAT_DEFAULT, false );
	bEnvmap = ( pVtf->Flags() & TEXTUREFLAGS_ENVMAP ) != 0;
	return true;
}

void CHLSL_Image::InitProceduralMaterial()
{
}

void CHLSL_Image::DestroyProceduralMaterial()
{
}

#include "bitmap/tgawriter.h"


struct ValveJpegErrorHandler_t
{
	struct jpeg_error_mgr	m_Base;
	jmp_buf m_ErrorContext;
};

static void ValveJpegErrorHandler( j_common_ptr cinfo )
{
	ValveJpegErrorHandler_t *pError = reinterpret_cast< ValveJpegErrorHandler_t * >( cinfo->err );
	char buffer[ JMSG_LENGTH_MAX ];
	/* Create the message */
	( *cinfo->err->format_message )( cinfo, buffer );
	Warning( "%s\n", buffer );
	longjmp( pError->m_ErrorContext, 1 );
}

CUtlVector<JOCTET> jOut;
#define BLOCK_SIZE 16384

void jInit_destination(j_compress_ptr cinfo)
{
	jOut.Purge();
	jOut.SetSize(BLOCK_SIZE);
	cinfo->dest->next_output_byte = &jOut[0];
	cinfo->dest->free_in_buffer = jOut.Count();
}

boolean jEmpty_output_buffer(j_compress_ptr cinfo)
{
	size_t oldsize = jOut.Count();
	jOut.SetCountNonDestructively(oldsize + BLOCK_SIZE);

	cinfo->dest->next_output_byte = &jOut[oldsize];
	cinfo->dest->free_in_buffer = jOut.Count() - oldsize;
	return true;
}

void jTerm_destination(j_compress_ptr cinfo)
{
	jOut.SetCountNonDestructively(jOut.Count() - cinfo->dest->free_in_buffer);
}

void CHLSL_Image::CreateScreenshot( CNodeView *n, const char *filepath )
{
	Vector4D graphBounds;
	n->GetGraphBoundaries( graphBounds );

	int vsize_x, vsize_y;
	n->GetSize( vsize_x, vsize_y );
	const float zoomFactor = 1.0f / clamp( sedit_screenshot_zoom.GetFloat(), 0.01f, 100.0f );

	float nodespace_width = graphBounds.z - graphBounds.x;
	float nodespace_height = graphBounds.w - graphBounds.y;
	float nodespace_width_pershot = vsize_x * zoomFactor;
	float nodespace_height_pershot = vsize_y * zoomFactor;

	int numshots_x = ceil( nodespace_width / nodespace_width_pershot );
	int numshots_y = ceil( nodespace_height / nodespace_height_pershot );

	Vector2D extraSpace;
	extraSpace.x = (nodespace_width_pershot*numshots_x) - nodespace_width;
	extraSpace.y = (nodespace_height_pershot*numshots_y) - nodespace_height;
	extraSpace *= 0.5f;

	n->vecScreenshotBounds.Init( graphBounds.x - extraSpace.x,
								graphBounds.y - extraSpace.y,
								graphBounds.z + extraSpace.x,
								graphBounds.w + extraSpace.y );

	int tgapixels_x = numshots_x * vsize_x - numshots_x;
	int tgapixels_y = numshots_y * vsize_y - numshots_y;

	unsigned int targetSize = tgapixels_x * 3 * tgapixels_y;
	unsigned char *pTGA = ( unsigned char * )malloc( targetSize );
	if ( !pTGA )
	{
		Warning( "Not enough memory available (failed to malloc %u bytes).\n", targetSize );
	}

	Vector2D pos_old = n->AccessViewPos();
	float zoom_old = n->AccessViewZoom();
	n->bRenderingScreenShot = true;

	n->AccessViewZoom() = zoomFactor;

	int vp_x, vp_y;
	vp_x = vp_y = 0;
	n->LocalToScreen( vp_x, vp_y );
	int screen_sx, screen_sy;
	engine->GetScreenSize( screen_sx, screen_sy );

	vp_x++;
	vp_y++;
	vsize_x--;
	vsize_y--;

	n->AccessViewPos().Init();

	CViewSetup view2D;
	view2D.x = 0;
	view2D.y = 0;
	view2D.width = screen_sx;
	view2D.height = screen_sy;
	view2D.m_bRenderToSubrectOfLargerScreen = false;
	Frustum _fplanes;

	CMatRenderContextPtr pRenderContext( materials );

#ifdef SHADER_EDITOR_DLL_2006
	render->Push2DView( view2D, 0, false, NULL, _fplanes );
#else
	render->Push2DView( view2D, 0, NULL, _fplanes );
#endif
	pRenderContext->PushRenderTargetAndViewport( NULL, view2D.x, view2D.y, view2D.width, view2D.height );

//	bool bSuccess = TGAWriter::WriteDummyFileNoAlloc( filepath, tgapixels_x, tgapixels_y, IMAGE_FORMAT_RGBA8888 );
//	Assert( bSuccess );

	if ( pTGA )
	{
		for ( int sX = 0; sX < numshots_x; sX ++ )
		{
			for ( int sY = 0; sY < numshots_y; sY ++ )
			{
				Vector2D basepos( graphBounds.x + nodespace_width_pershot * 0.5f, graphBounds.w - nodespace_height_pershot * 0.5f );
				basepos.x += sX * nodespace_width_pershot;
				basepos.y -= sY * nodespace_height_pershot;
				basepos.x *= -1.0f;
				basepos += extraSpace;
				n->AccessViewPos() = basepos;

				pRenderContext->ClearColor4ub( 0, 0, 0, 0 );
				pRenderContext->ClearBuffers( true, true );
				vgui::ipanel()->PaintTraverse( n->GetVPanel(), true );

				unsigned int sizeimg = vsize_x * 3 * vsize_y;
				unsigned char *pImage = ( unsigned char * )malloc( sizeimg );
				if ( pImage )
				{
					pRenderContext->ReadPixels( vp_x, vp_y, vsize_x, vsize_y, pImage, IMAGE_FORMAT_RGB888 );

					for ( int pX = 0; pX < vsize_x; pX ++ )
					{
						for ( int pY = 0; pY < vsize_y; pY ++ )
						{
							int targetpixel = (sX * vsize_x + pX) * 3 +
								( sY * vsize_y + pY ) * tgapixels_x * 3;
							int srcpixel = pX * 3 + pY * vsize_x * 3;

							Q_memcpy( &pTGA[targetpixel], &pImage[srcpixel], 3 );
						}
					}
#if 0
#if DEBUG
					bool bSuccess =
#endif
					TGAWriter::WriteRectNoAlloc( pImage, filepath, sX * vsize_x, sY * vsize_y, vsize_x, vsize_y, 0, IMAGE_FORMAT_RGB888 );
#if DEBUG
					Assert( bSuccess );
#endif
#endif

					free( pImage );
				}
				else
					Warning( "Tiling error (failed to malloc %u bytes).\n", sizeimg );

			}
		}
	}


	if ( pTGA )
	{
		//unsigned int iMaxTGASize = 1024 + (tgapixels_x * tgapixels_y * 3);
		//void *pTGA_BUFFER = malloc( iMaxTGASize );
		//if ( pTGA_BUFFER )
		//{
		//	CUtlBuffer buffer( pTGA_BUFFER, iMaxTGASize );

		//	TGAWriter::WriteToBuffer( pTGA, buffer, tgapixels_x, tgapixels_y, IMAGE_FORMAT_RGB888, IMAGE_FORMAT_RGB888 );
		//	filesystem->AsyncWrite( filepath, buffer.Base(), buffer.TellPut(), true );
		//}
		//else
		//{
		//	Warning( "Not enough memory available (failed to malloc %u bytes).\n", iMaxTGASize );
		//}

		jpeg_compress_struct jInfo;
		ValveJpegErrorHandler_t jErr;
		jpeg_destination_mgr jDest;

		jInfo.err = jpeg_std_error( &jErr.m_Base );
		jInfo.err->error_exit = &ValveJpegErrorHandler;

		jpeg_create_compress( &jInfo );

		jInfo.dest = &jDest;
		jInfo.dest->init_destination = &jInit_destination;
		jInfo.dest->empty_output_buffer = &jEmpty_output_buffer;
		jInfo.dest->term_destination = &jTerm_destination;

		jInfo.image_width = tgapixels_x;
		jInfo.image_height = tgapixels_y;
		jInfo.input_components = 3;
		jInfo.in_color_space = JCS_RGB;
		jpeg_set_defaults( &jInfo );
		jpeg_set_quality( &jInfo, clamp( sedit_screenshot_quali.GetInt(), 1, 100 ), FALSE );

		jpeg_start_compress(&jInfo, TRUE);

		JSAMPROW row_pointer[1];
		int row_stride;
		row_stride = jInfo.image_width * 3;
		while (jInfo.next_scanline < jInfo.image_height)
		{
			row_pointer[0] = &pTGA[jInfo.next_scanline * row_stride];
			jpeg_write_scanlines(&jInfo, row_pointer, 1);
		}

		jpeg_finish_compress(&jInfo);

		jpeg_destroy_compress(&jInfo);

		void *pBUFFER = malloc( jOut.Count() );
		Q_memcpy( pBUFFER, jOut.Base(), jOut.Count() );
		filesystem->AsyncWrite( filepath, pBUFFER, jOut.Count(), true );

		jOut.Purge();
		free( pTGA );
	}

	pRenderContext->PopRenderTargetAndViewport();
	render->PopView( _fplanes );

	n->bRenderingScreenShot = false;
	n->AccessViewPos() = pos_old;
	n->AccessViewZoom() = zoom_old;
}
