#ifndef VGUI_UTILITIES
#define VGUI_UTILITIES


#include <assert.h>
#include <math.h>
#define PROTECTED_THINGS_DISABLE

#include <mathlib/vector.h>
#include <mathlib/vector4d.h>

#include "vgui/ISurface.h"
#include "vgui/IVgui.h"

#include "vgui_controls/controls.h"
#include "vgui_controls/tokenchannels.h"

void IntToHex( int value, char *out, int bufSize );
void HexToInt( const char *hexString, int &out, int bufSize );


void HSV2RGB( float H, float s, float v, Vector &normalizedRGB );
void HSV2RGB( float H, float s, float v, Vector4D &normalizedRGBA );
void HSV2RGB( const Vector &hsv, Vector4D &normalizedRGBA );
void HSV2RGB( const Vector &hsv, Vector &normalizedRGBA );
void RGB2HSV( const Vector &normalizedRGB, float &H, float &s, float &v );
void RGB2HSV( const Vector &normalizedRGB, Vector &hsv );

void SetupVguiTex( int &var, const char *tex );

extern inline int GetChannelNumFromChar( const char *c );
extern inline char GetCharFromChannelNum( const int i );

//#include "memtrack.h"

#endif