#pragma once

void Init3D( IDirect3DDevice9 * D3DDev );

void Start3D( int imageWidth, int imageHeight );
void Trick3D( ID3DXEffect* eff, CBaseCamera* cam, bool test, bool usePPStereoscopy, float stereoStrength );
void Stop3D();

void CalcStereoViewProj( CBaseCamera* cam, D3DXMATRIX* leftView, D3DXMATRIX* leftProj, D3DXMATRIX* rightView, D3DXMATRIX* rightProj, float strength );

IDirect3DSurface9* GetCenterTarget();
IDirect3DSurface9* GetLeftTarget();
IDirect3DSurface9* GetRightTarget();

IDirect3DSurface9* GetDepth();

bool Is3DOn();

#define FAKE_STEREO 0