#ifndef CHLSLVAR_H
#define CHLSLVAR_H

#include "editorCommon.h"

class CBaseNode;

class CHLSL_Var
{
public:
	CHLSL_Var( HLSLVariableTypes type );
	virtual ~CHLSL_Var();

	CHLSL_Var( const CHLSL_Var& other );

	const HLSLVariableTypes &GetType(){ return m_tType; };

	const bool &WasDeclared();
	const void OnDeclare();
	void DeclareMe( WriteContext_FXC &context, bool init = false );

	const bool &CanBeOverwritten();
	const void MakeConstantOnly();

	CHLSL_SolverBase *GetOwner();
	void SetOwner( CHLSL_SolverBase *n );

	const void ResetVarInfo();
	void Cleanup();

	void SetName( const char *name, bool Custom = false );
	const char *GetName();
	const bool HasCustomName();

	const int &GetMapIndex(){ return m_iRTMapIndex; };
	void ResetMapIndex(){ m_iRTMapIndex = 0; };
	void SetMapIndex( const int idx ){ m_iRTMapIndex = idx; };

public:

	IMaterial *GetMaterial();
	ITexture *GetTexture();
	void SetMaterial( IMaterial *pMat );
	void SetTexture( ITexture *pTex );

private:
	CHLSL_Var();
	void Init();

	HLSLVariableTypes m_tType;
	int m_iRTMapIndex;

	bool m_bCanBeDeclared;
	bool m_bGotDeclared;

	bool m_bCanBeOverwritten;
	char *_varName;

	bool bCustomName;
	CHLSL_SolverBase *pOwner;

	union
	{
		ITexture *m_pTex;
		IMaterial *m_pMat;
	};
};

//#if 1 //DEBUG
//extern void PrintError();
//#define ON_VAR_ERROR PrintError();
//#else
//#define ON_VAR_ERROR ((void)0);
//#endif
//
//inline CHLSL_Var& CHLSL_Var::operator=(const CHLSL_Var &o)
//{
//	Q_memcpy( m_Vars, o.m_Vars, varsSize );
//	return *this;
//}
//inline void CHLSL_Var::LoadType( const CHLSL_Var &o )
//{
//	m_iVarSize = o.m_iVarSize;
//	m_tType = o.m_tType;
//}
//
//inline CHLSL_Var& CHLSL_Var::operator+=(const CHLSL_Var &o)
//{
//	if ( m_tType == o.m_tType )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] += o.m_Vars[ i ];
//	}
//	else if ( m_tType != HLSLVAR_FLOAT1 && o.m_tType == HLSLVAR_FLOAT1 )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] += o.m_Vars[ 0 ];
//	}
//	else
//		ON_VAR_ERROR
//
//	return *this;
//}
//
//inline CHLSL_Var CHLSL_Var::operator+(const CHLSL_Var& o) const
//{
//	CHLSL_Var n;
//	if ( m_tType == HLSLVAR_FLOAT1 && o.m_tType != HLSLVAR_FLOAT1 )
//	{
//		n = o;
//		n.LoadType( o );
//		n += *this;
//	}
//	else
//	{
//		n = *this;
//		n.LoadType( *this );
//		n += o;
//	}
//	return n;
//}
//
//inline CHLSL_Var& CHLSL_Var::operator-=(const CHLSL_Var &o)
//{
//	if ( m_tType == o.m_tType )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] -= o.m_Vars[ i ];
//	}
//	else if ( m_tType != HLSLVAR_FLOAT1 && o.m_tType == HLSLVAR_FLOAT1 )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] -= o.m_Vars[ 0 ];
//	}
//	else
//		ON_VAR_ERROR
//
//	return *this;
//}
//
//inline CHLSL_Var CHLSL_Var::operator-(const CHLSL_Var& o) const
//{
//	CHLSL_Var n;
//
//	if ( m_tType == HLSLVAR_FLOAT1 && o.m_tType != HLSLVAR_FLOAT1 )
//	{
//		n = o;
//		n.LoadType( o );
//		n -= *this;
//	}
//	else
//	{
//		n = *this;
//		n.LoadType( *this );
//		n -= o;
//	}
//
//	return n;
//}
//
//inline CHLSL_Var& CHLSL_Var::operator*=(const CHLSL_Var &o)
//{
//	if ( o.m_tType >= HLSLVAR_MATRIX3X3 )
//	{
//		VMatrix other;
//		o.FillMatrix( &o, other );
//		if ( m_tType >= HLSLVAR_MATRIX3X3 )
//		{
//			VMatrix local = GetMatrix();
//			VMatrix dst;
//			//MatrixMultiply( local, other, dst );
//			MatrixMultiply( other, local, dst );
//		}
//		else if ( m_tType == HLSLVAR_FLOAT3 )
//		{
//			// 3 * 3x3
//			// Vector3DMultiply
//
//			// 3 * 4x3
//			// Vector3DMultiplyPosition
//
//			// 3 * 4x4
//			// Vector3DMultiplyPositionProjective
//
//			Vector out;
//			switch ( o.m_tType )
//			{
//			case HLSLVAR_MATRIX3X3:
//				Vector3DMultiply( other, GetVec3(), out );
//				SetVec3( out );
//				break;
//			case HLSLVAR_MATRIX4X3:
//				Vector3DMultiplyPosition( other, GetVec3(), out );
//				SetVec3( out );
//				break;
//			case HLSLVAR_MATRIX4X4:
//				// remove auto projection?
//#if 1
//				Vector4D tmp( m_Vars[0], m_Vars[1], m_Vars[2], 1 );
//				Vector4D outTmp;
//				Vector4DMultiply( other, tmp, outTmp );
//				//Vector lol( m_Vars[0], m_Vars[1], m_Vars[2] ), derp;
//				//Vector3DMultiplyPositionProjective( other, lol, derp );
//				//Msg("%f %f %f\n", derp.x, derp.y, derp.z);
//				//out.x = outTmp.x; out.y = outTmp.y; out.z = outTmp.z;
//				SetVec4( outTmp );
//#else
//				Vector3DMultiplyPositionProjective( other, GetVec3(), out );
//				SetVec3( out );
//#endif
//				break;
//			}
//		}
//		else if ( m_tType == HLSLVAR_FLOAT4 )
//		{
//			Vector4D out;
//			switch ( o.m_tType )
//			{
//			case HLSLVAR_MATRIX3X3:
//			case HLSLVAR_MATRIX4X3:
//			case HLSLVAR_MATRIX4X4:
//				Vector4DMultiply( other, GetVec4(), out );
//				break;
//			}
//			SetVec4( out );
//		}
//		else
//			ON_VAR_ERROR
//	}
//	else if ( m_tType == o.m_tType )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] *= o.m_Vars[ i ];
//	}
//	else if ( m_tType != HLSLVAR_FLOAT1 && o.m_tType == HLSLVAR_FLOAT1 )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] *= o.m_Vars[ 0 ];
//	}
//	else
//		ON_VAR_ERROR
//
//	return *this;
//}
//
//inline CHLSL_Var CHLSL_Var::operator*(const CHLSL_Var& o) const
//{
//	CHLSL_Var n;
//
//	if ( ( m_tType == HLSLVAR_FLOAT1 && o.m_tType != HLSLVAR_FLOAT1 ) ||
//		( m_tType >= HLSLVAR_MATRIX3X3 ) )
//	{
//		n = o;
//		n.LoadType( o );
//		n *= *this;
//	}
//	else
//	{
//		n = *this;
//		n.LoadType( *this );
//		n *= o;
//	}
//
//	return n;
//}
//
//inline CHLSL_Var& CHLSL_Var::operator/=(const CHLSL_Var &o)
//{
//	if ( m_tType == o.m_tType ) //&& m_tType <= HLSLVAR_FLOAT4 )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] /= o.m_Vars[ i ];
//	}
//	else if ( m_tType != HLSLVAR_FLOAT1 && o.m_tType == HLSLVAR_FLOAT1 )
//	{
//		for ( int i = 0; i < m_iVarSize; i++ )
//			m_Vars[ i ] /= o.m_Vars[ 0 ];
//	}
//	else
//		ON_VAR_ERROR
//
//	return *this;
//}
//
//inline CHLSL_Var CHLSL_Var::operator/(const CHLSL_Var& o) const
//{
//	CHLSL_Var n;
//
//	if ( ( m_tType == HLSLVAR_FLOAT1 && o.m_tType != HLSLVAR_FLOAT1 ) ||
//		( m_tType != o.m_tType && m_tType >= HLSLVAR_MATRIX3X3 ) )
//	{
//		n = o;
//		n.LoadType( o );
//		n *= *this;
//	}
//	else
//	{
//		n = *this;
//		n.LoadType( *this );
//		n *= o;
//	}
//
//	return n;
//}

#endif