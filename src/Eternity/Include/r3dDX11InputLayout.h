#ifndef __R3D_DX11_INPUT_LAYOUT_H
#define __R3D_DX11_INPUT_LAYOUT_H

#ifndef WO_SERVER

struct ID3D11InputLayout;
class r3dDX11Shader;

class r3dDX11InputLayoutManager
{
public:
    r3dDX11InputLayoutManager();
    ~r3dDX11InputLayoutManager();

    void Shutdown();
    void InvalidateCache();

    ID3D11InputLayout* GetOrCreate(const void* d3d9Declaration, r3dDX11Shader* shader);
    bool Set(const void* d3d9Declaration, r3dDX11Shader* shader);

private:
    enum
    {
        R3D_DX11_MAX_INPUT_LAYOUTS = 256
    };

    struct LayoutEntry
    {
        unsigned int Hash;
        unsigned int ElementCount;
        ID3D11InputLayout* Layout;
    };

    int FindLayout(unsigned int hash, unsigned int elementCount) const;
    int AllocLayoutSlot();

    LayoutEntry Layouts[R3D_DX11_MAX_INPUT_LAYOUTS];
    int NumLayouts;
    ID3D11InputLayout* CurrentLayout;
};

extern r3dDX11InputLayoutManager g_r3dDX11InputLayouts;

#endif

#endif