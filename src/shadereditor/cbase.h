//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CBASE_H
#define CBASE_H
#ifdef _WIN32
#pragma once
#endif

struct studiohdr_t;

#define CUSTOM_MEM_DBG 0

#if CUSTOM_MEM_DBG
#define NO_MALLOC_OVERRIDE
#endif // CUSTOM_MEM_DBG

#ifndef swap
template < typename T >
inline void swap( T& x, T& y )
{
	T temp = x;
	x = y;
	y = temp;
}
#endif

#include <stdio.h>
#include <stdlib.h>

#include <tier0/platform.h>
#include <tier0/dbg.h>

#include <tier1/strtools.h>
#include <vstdlib/random.h>
#include <utlvector.h>

#include <const.h>

#include "string_t.h"
#include "globalvars_base.h"

#include <icvar.h>

#include "dll_init.h"

#include "engine/ivmodelinfo.h"
#include "engine/ivmodelrender.h"

#include "igamesystem.h"

#include "editorCommon.h"

#endif // CBASE_H
