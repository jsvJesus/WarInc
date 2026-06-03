#include <max.h>
#include <windows.h>
#include <commdlg.h>

#include "resource.h"
#include "WarIncMaxBridge.h"

#define WARINC_IMPORTER_CLASS_ID Class_ID(0x6a7b4112, 0x18df64aa)

static HINSTANCE g_hInstance = 0;
static WI_MaxState g_State;

static std::string PickFile(HWND parent, const char* filter)
{
    char fileName[MAX_PATH];
    memset(fileName, 0, sizeof(fileName));

    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parent;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if(GetOpenFileNameA(&ofn))
        return fileName;

    return "";
}

static std::string PickFolder(HWND parent)
{
    char fileName[MAX_PATH];
    memset(fileName, 0, sizeof(fileName));

    BROWSEINFOA bi;
    memset(&bi, 0, sizeof(bi));

    bi.hwndOwner = parent;
    bi.lpszTitle = "Select WarInc root folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if(!pidl)
        return "";

    SHGetPathFromIDListA(pidl, fileName);

    IMalloc* pMalloc = 0;
    if(SHGetMalloc(&pMalloc) == NOERROR)
    {
        pMalloc->Free(pidl);
        pMalloc->Release();
    }

    return fileName;
}

static void SetStatus(HWND hWnd, const std::string& s)
{
    SetDlgItemTextA(hWnd, IDC_STATUS_TEXT, s.c_str());
}

class WarIncUtility : public UtilityObj
{
public:
    Interface* ip;
    IUtil* iu;
    HWND hPanel;

    WarIncUtility()
    {
        ip = 0;
        iu = 0;
        hPanel = 0;
    }

    void BeginEditParams(Interface* ip_, IUtil* iu_)
    {
        ip = ip_;
        iu = iu_;
        hPanel = ip->AddRollupPage(
            g_hInstance,
            MAKEINTRESOURCE(IDD_WARINC_PANEL),
            DialogProc,
            "WarInc Asset Importer",
            0
        );
    }

    void EndEditParams(Interface* ip_, IUtil* iu_)
    {
        if(hPanel)
        {
            ip_->DeleteRollupPage(hPanel);
            hPanel = 0;
        }

        ip = 0;
        iu = 0;
    }

    void DeleteThis() {}

    static INT_PTR CALLBACK DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if(msg == WM_COMMAND)
        {
            Interface* ip = GetCOREInterface();
            std::string file;
            std::string error;

            switch(LOWORD(wParam))
            {
            case IDC_SET_ROOT:
                {
                    std::string root = PickFolder(hWnd);
                    if(root.empty())
                        return TRUE;

                    if(g_State.index.Build(root))
                        SetStatus(hWnd, "Root indexed");
                    else
                        SetStatus(hWnd, "Root index failed");

                    return TRUE;
                }

            case IDC_IMPORT_SCO:
                {
                    file = PickFile(hWnd, "WarInc Model (*.sco)\0*.sco\0All Files (*.*)\0*.*\0");
                    if(file.empty())
                        return TRUE;

                    if(WI_MaxImportSCO(ip, g_State, file, error))
                        SetStatus(hWnd, "SCO imported");
                    else
                        SetStatus(hWnd, error);

                    return TRUE;
                }

            case IDC_IMPORT_SKL:
                {
                    file = PickFile(hWnd, "WarInc Skeleton (*.skl)\0*.skl\0All Files (*.*)\0*.*\0");
                    if(file.empty())
                        return TRUE;

                    if(WI_MaxImportSKL(ip, g_State, file, error))
                        SetStatus(hWnd, "SKL imported");
                    else
                        SetStatus(hWnd, error);

                    return TRUE;
                }

            case IDC_APPLY_WGT:
                {
                    file = PickFile(hWnd, "WarInc Weights (*.wgt)\0*.wgt\0All Files (*.*)\0*.*\0");
                    if(file.empty())
                        return TRUE;

                    if(WI_MaxApplyWGT(ip, g_State, file, error))
                        SetStatus(hWnd, "WGT applied");
                    else
                        SetStatus(hWnd, error);

                    return TRUE;
                }

            case IDC_IMPORT_ANM:
                {
                    file = PickFile(hWnd, "WarInc Animation (*.anm)\0*.anm\0All Files (*.*)\0*.*\0");
                    if(file.empty())
                        return TRUE;

                    if(WI_MaxImportANM(ip, g_State, file, error))
                        SetStatus(hWnd, "ANM imported");
                    else
                        SetStatus(hWnd, error);

                    return TRUE;
                }
            }
        }

        return FALSE;
    }
};

static WarIncUtility g_Utility;

class WarIncClassDesc : public ClassDesc2
{
public:
    int IsPublic() { return TRUE; }
    void* Create(BOOL loading = FALSE) { return &g_Utility; }
    const TCHAR* ClassName() { return _T("WarInc Asset Importer"); }
    SClass_ID SuperClassID() { return UTILITY_CLASS_ID; }
    Class_ID ClassID() { return WARINC_IMPORTER_CLASS_ID; }
    const TCHAR* Category() { return _T("WarInc"); }
    const TCHAR* InternalName() { return _T("WarIncAssetImporter"); }
    HINSTANCE HInstance() { return g_hInstance; }
};

static WarIncClassDesc g_ClassDesc;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
    }

    return TRUE;
}

extern "C" __declspec(dllexport) const TCHAR* LibDescription()
{
    return _T("WarInc 3ds Max 2012 Asset Importer");
}

extern "C" __declspec(dllexport) int LibNumberClasses()
{
    return 1;
}

extern "C" __declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
    if(i == 0)
        return &g_ClassDesc;

    return 0;
}

extern "C" __declspec(dllexport) ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

extern "C" __declspec(dllexport) ULONG CanAutoDefer()
{
    return 1;
}