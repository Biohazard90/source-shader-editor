#ifndef SHADERDUMPS_H
#define SHADERDUMPS_H

#include "cbase.h"

KeyValues *df_LoadDump_List(); // alloc
void df_SaveDump_List( KeyValues *pKV ); // dealloc

BasicShaderCfg_t *BuildShaderData( const char *dumpFileName );
void df_SaveDump_File( const char *canvasname, const BasicShaderCfg_t &shader );

void ReadGameShaders( CUtlVector<BasicShaderCfg_t*> &hList );
void LoadGameShaders( bool bFirstRun = false );
void UnloadGameShaders();

#endif