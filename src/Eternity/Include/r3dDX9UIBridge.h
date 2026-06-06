#pragma once

#ifndef WO_SERVER

bool r3dDX9UIBridge_Init();
void r3dDX9UIBridge_Close();

bool r3dDX9UIBridge_Begin();
void r3dDX9UIBridge_End(bool compositeToDX11);

bool r3dDX9UIBridge_CompositeToDX11();
bool r3dDX9UIBridge_IsCapturing();

#endif