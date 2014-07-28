
#include <vgui_controls\Utilities.h>

#include "vgui_editor_platform.h"

static char hexLookup[16] =
{
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
};

void IntToHex( int value, char *out, int bufSize )
{
	value = abs(value);

	char tmpString[9];
	int curpos = 8;

	int value_cur = value;
	while (value_cur && curpos >= 0)
	{
		int tmp = value_cur % 16;
		value_cur /= 16;

		tmpString[curpos] = hexLookup[ tmp ];
		curpos--;
	}
	curpos++;

	int i = 0;
	for ( ; i < bufSize && curpos <= 8; i++ )
	{
		out[i] = tmpString[curpos];
		curpos++;
	}
	out[i] = '\0';
}
void HexToInt( const char *hexString, int &out, int bufSize )
{
	out = 0;
	int multiplier = 1;

	int num_min = (int)'0';
	int text_low_min = (int)'a';
	int text_high_min = (int)'A';

	const char *walker = hexString;
	int steps = 0;
	while ( *walker && (steps < bufSize || bufSize < 0))
	{
		walker++;
		steps++;
	}
	walker--;

	while (walker && *walker && (steps > 0 || bufSize < 0))
	{
		int value = 0;
		if (*walker >= num_min && *walker <= num_min + 9)
			value = *walker - num_min;
		else if (*walker >= text_low_min && *walker <= text_low_min + 5)
			value = *walker - text_low_min + 10;
		else if (*walker >= text_high_min && *walker <= text_high_min + 5)
			value = *walker - text_high_min + 10;

		out += value * multiplier;

		multiplier *= 16;
		walker--;
		steps--;
	}
}



void HSV2RGB( float H, float s, float v, Vector &normalizedRGB )
{
	int Hr = floor( H / 60.0f );
	float f = H / 60.0f - Hr;

	float p = v * ( 1.0f - s );
	float q = v * ( 1.0f - s * f );
	float t = v * ( 1.0f - s * ( 1.0f - f ) );

	switch ( Hr )
	{
	default:
		normalizedRGB.Init( v, t, p );
		break;
	case 1:
		normalizedRGB.Init( q, v, p );
		break;
	case 2:
		normalizedRGB.Init( p, v, t );
		break;
	case 3:
		normalizedRGB.Init( p, q, v );
		break;
	case 4:
		normalizedRGB.Init( t, p, v );
		break;
	case 5:
		normalizedRGB.Init( v, p, q );
		break;
	}
}
void HSV2RGB( float H, float s, float v, Vector4D &normalizedRGBA )
{
	Vector tmp;
	HSV2RGB( H, s, v, tmp );
	normalizedRGBA.Init( tmp.x, tmp.y, tmp.z, 1 );
}
void HSV2RGB( const Vector &hsv, Vector4D &normalizedRGBA )
{
	HSV2RGB( hsv.x, hsv.y, hsv.z, normalizedRGBA );
}
void HSV2RGB( const Vector &hsv, Vector &normalizedRGBA )
{
	HSV2RGB( hsv.x, hsv.y, hsv.z, normalizedRGBA );
}
void RGB2HSV( const Vector &normalizedRGB, float &H, float &s, float &v )
{
	float fmax = max( normalizedRGB.x, max( normalizedRGB.y, normalizedRGB.z ) );
	float fmin = min( normalizedRGB.x, min( normalizedRGB.y, normalizedRGB.z ) );

	v = fmax;

	if ( fmax <= 0.0f || normalizedRGB.LengthSqr() <= 0.0f )
		s = 0.0f;
	else
		s = (fmax-fmin) / fmax;

	if ( fmax == fmin || ( normalizedRGB.x == normalizedRGB.y && normalizedRGB.x == normalizedRGB.z ) )
	{
		H = 0; //-1.0f;
	}
	else if ( normalizedRGB.x >= fmax )
		H = 60.0f * ((normalizedRGB.y - normalizedRGB.z)/(fmax-fmin));
	else if ( normalizedRGB.y >= fmax )
		H = 60.0f * (2+(normalizedRGB.z - normalizedRGB.x)/(fmax-fmin));
	else if ( normalizedRGB.z >= fmax )
		H = 60.0f * (4+(normalizedRGB.x - normalizedRGB.y)/(fmax-fmin));
	if ( H < 0 )
		H += 360.0f;
}
void RGB2HSV( const Vector &normalizedRGB, Vector &hsv )
{
	RGB2HSV( normalizedRGB, hsv.x, hsv.y, hsv.z );
}


void SetupVguiTex( int &var, const char *tex )
{
	var = vgui::surface()->DrawGetTextureId(tex);
	if ( var <= 0 )
	{
		var = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( var, tex , true, false);
	}
}



inline int GetChannelNumFromChar( const char *c )
{
	switch ( *c )
	{
	case 'r':
	case 'R':
	case 'x':
	case 'X':
			return 0;
		break;
	case 'g':
	case 'G':
	case 'y':
	case 'Y':
			return 1;
		break;
	case 'b':
	case 'B':
	case 'z':
	case 'Z':
			return 2;
		break;
	case 'a':
	case 'A':
	case 'w':
	case 'W':
			return 3;
		break;
	}
	return -1;
}
inline char GetCharFromChannelNum( const int i )
{
	switch (i)
	{
	case 0:
		return 'x';
	case 1:
		return 'y';
	case 2:
		return 'z';
	case 3:
		return 'w';
	}
	return 'x';
}