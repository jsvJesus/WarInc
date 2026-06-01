#ifndef __R3D_VERTEX_SHADER_H
#define __R3D_VERTEX_SHADER_H

class r3dDX11Shader;

#ifndef WO_SERVER
struct IDirect3DVertexDeclaration9;

r3dDX11Shader* r3dDX11_GetCurrentVertexShader();
bool r3dDX11_ApplyCurrentVertexShaderInputLayout(IDirect3DVertexDeclaration9* decl);
#endif

class   r3dVertexShader
{
public:
    struct ShaderMacro
    {
        r3dString Name;
        r3dString Definition;
    };

    typedef r3dTL::TArray< ShaderMacro > MacroArr;

    IDirect3DVertexShader9  *m_pShader;
    r3dDX11Shader*			m_dx11Shader;

    char    Name[64];
    char    FileName[MAX_PATH];
    int     bSystem;

    MacroArr Macros;

    r3dVertexShader();
    ~r3dVertexShader() { Unload(); }

    void    Unload();
    void    Init(const char* Shader, int Type = 0);
    int     Load(const char* FName, int Type = 0);
    int     Load(const char* FName, int Type, const r3dTL::TArray <D3DXMACRO> & defines );

    int     LoadBinaryCache( const char* FName, const char* Path, const char* defines );
    void    SaveBinaryCache( const char* FName, ID3DXBuffer*  pCode, const char* defines, const r3dTL::TArray< r3dString >& Includes );

    int		LoadDX11(const char* FName, int Type = 0);
    int		LoadDX11(const char* FName, int Type, const r3dTL::TArray <D3DXMACRO> & defines);
    bool	CreateDX11PosTexInputLayout();
    bool	CreateDX11PosColorInputLayout();
    bool	SetDX11InputLayoutFromD3D9Decl(const void* d3d9Declaration);
    r3dDX11Shader* GetDX11Shader() const;

    void    SetActive(int Act = 0);
};

extern char LastCompilationError[ 16384 ];

#endif