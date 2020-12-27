#include <DirectXPCH.h>
using namespace DirectX;

const LONG WindowWidth = 1280;
const LONG WindowHeight = 720;

LPCSTR WindowClassName = "DirectXWindowClass";
LPCSTR WindowName = "DirectX Template";

HWND WindowHandle = 0;
const BOOL EnableVsync = TRUE;

// General DirectX 11 needed variables --------------------
ID3D11Device* D3dDevice = nullptr;
ID3D11DeviceContext* D3dDeviceContext = nullptr;
IDXGISwapChain* D3dSwapChain = nullptr;

ID3D11RenderTargetView* D3dRenderTargetView = nullptr;
ID3D11DepthStencilView* D3dDepthStencilView = nullptr;
ID3D11Texture2D* D3dDepthStencilBuffer = nullptr;

ID3D11DepthStencilState* D3dDepthStencilState = nullptr;
ID3D11RasterizerState* D3dRasterizerState = nullptr;
D3D11_VIEWPORT Viewport = { 0 };
// ---------------------------------------------------------

// Vertex buffer data
ID3D11InputLayout* D3dInputLayout = nullptr;
ID3D11Buffer* D3dVertexBuffer = nullptr;
ID3D11Buffer* D3dIndexBuffer = nullptr;

// Shader data
ID3D11VertexShader* D3dVertexShader = nullptr;
ID3D11PixelShader* D3dPixelShader = nullptr;

// Shader resources
enum ConstantBuffer
{
	CB_Application,
	CB_Frame,
	CB_Object,
	NumConstantBuffers
};

ID3D11Buffer* D3dConstantBuffers[ConstantBuffer::NumConstantBuffers];

// Demo parameters
XMMATRIX WorldMatrix;
XMMATRIX ViewMatrix;
XMMATRIX ProjMatrix;

// Vertex data for a colored cube.
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

VertexPosColor Vertices[8] =
{
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

WORD Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

template< class ShaderClass >
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile);

bool LoadContent();
void UnloadContent();

void Update(float deltaTime);
void Render();
void Cleanup();

// =============================================================
// =============================================================
// =============================================================

DXGI_RATIONAL QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync)
{
    DXGI_RATIONAL refreshRate = { 0, 1 };
    if (vsync)
    {
        IDXGIFactory* factory;
        IDXGIAdapter* adapter;
        IDXGIOutput* adapterOutput;
        DXGI_MODE_DESC* displayModeList;

        // Create a DirectX graphics interface factory.
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
        if (FAILED(hr))
        {
            MessageBox(0,
                TEXT("Could not create DXGIFactory instance."),
                TEXT("Query Refresh Rate"),
                MB_OK);

            throw new std::exception("Failed to create DXGIFactory.");
        }

        hr = factory->EnumAdapters(0, &adapter);
        if (FAILED(hr))
        {
            MessageBox(0,
                TEXT("Failed to enumerate adapters."),
                TEXT("Query Refresh Rate"),
                MB_OK);

            throw new std::exception("Failed to enumerate adapters.");
        }

        hr = adapter->EnumOutputs(0, &adapterOutput);
        if (FAILED(hr))
        {
            MessageBox(0,
                TEXT("Failed to enumerate adapter outputs."),
                TEXT("Query Refresh Rate"),
                MB_OK);

            throw new std::exception("Failed to enumerate adapter outputs.");
        }

        UINT numDisplayModes;
        hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
        if (FAILED(hr))
        {
            MessageBox(0,
                TEXT("Failed to query display mode list."),
                TEXT("Query Refresh Rate"),
                MB_OK);

            throw new std::exception("Failed to query display mode list.");
        }

        displayModeList = new DXGI_MODE_DESC[numDisplayModes];
        assert(displayModeList);

        hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList);
        if (FAILED(hr))
        {
            MessageBox(0,
                TEXT("Failed to query display mode list."),
                TEXT("Query Refresh Rate"),
                MB_OK);

            throw new std::exception("Failed to query display mode list.");
        }

        // Now store the refresh rate of the monitor that matches the width and height of the requested screen.
        for (UINT i = 0; i < numDisplayModes; ++i)
        {
            if (displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight)
            {
                refreshRate = displayModeList[i].RefreshRate;
            }
        }

        delete[] displayModeList;
        SafeRelease(adapterOutput);
        SafeRelease(adapter);
        SafeRelease(factory);
    }

    return refreshRate;
}

int InitApplication(HINSTANCE hInstance, int cmdShow)
{
    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = &WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = TEXT("DirectXWindowClass");

    if (!RegisterClassEx(&wndClass))
        return -1;

    RECT windowRect = { 0, 0, WindowWidth, WindowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    WindowHandle = CreateWindowA(
        WindowClassName, WindowName,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!WindowHandle)
        return -1;

    ShowWindow(WindowHandle, cmdShow);
    UpdateWindow(WindowHandle);

    return 0;
}

int InitDirectX(HINSTANCE hInstance, BOOL vSync)
{
    assert(WindowHandle != 0);

    RECT clientRect;
    GetClientRect(WindowHandle, &clientRect);

    unsigned int clientWidth = clientRect.right - clientRect.left;
    unsigned int clientHeight = clientRect.bottom - clientRect.top;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = clientWidth;
    swapChainDesc.BufferDesc.Height = clientHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate = QueryRefreshRate(clientWidth, clientHeight, vSync);
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = WindowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Windowed = TRUE;

    UINT createDeviceFlags = 0;
#if _DEBUG
    createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, createDeviceFlags, featureLevels, _countof(featureLevels),
        D3D11_SDK_VERSION, &swapChainDesc, &D3dSwapChain, &D3dDevice, &featureLevel,
        &D3dDeviceContext
    );

    if (hr == E_INVALIDARG)
    {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE,
            nullptr, createDeviceFlags, &featureLevels[1], _countof(featureLevels) - 1,
            D3D11_SDK_VERSION, &swapChainDesc, &D3dSwapChain, &D3dDevice, &featureLevel,
            &D3dDeviceContext
        );
    }

    if (FAILED(hr))
    {
        return -1;
    }

    ID3D11Texture2D* backBuffer;
    hr = D3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
    {
        return -1;
    }

    hr = D3dDevice->CreateRenderTargetView(backBuffer, nullptr, &D3dRenderTargetView);
    if (FAILED(hr))
    {
        return -1;
    }

    SafeRelease(backBuffer);

    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0;
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDesc.Width = clientWidth;
    depthStencilBufferDesc.Height = clientHeight;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.SampleDesc.Count = 1;
    depthStencilBufferDesc.SampleDesc.Quality = 0;
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    hr = D3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &D3dDepthStencilBuffer);
    if (FAILED(hr))
    {
        return -1;
    }

    hr = D3dDevice->CreateDepthStencilView(D3dDepthStencilBuffer, nullptr, &D3dDepthStencilView);
    if(FAILED(hr))
    {
        return -1;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    depthStencilStateDesc.DepthEnable = TRUE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilStateDesc.StencilEnable = FALSE;

    hr = D3dDevice->CreateDepthStencilState(&depthStencilStateDesc, &D3dDepthStencilState);
    if (FAILED(hr))
    {
        return -1;
    }

    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state object.
    hr = D3dDevice->CreateRasterizerState(&rasterizerDesc, &D3dRasterizerState);
    if (FAILED(hr))
    {
        return -1;
    }

    Viewport.Width = static_cast<float>(clientWidth);
    Viewport.Height = static_cast<float>(clientHeight);
    Viewport.TopLeftX = 0.0f;
    Viewport.TopLeftY = 0.0f;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;

    return 0;
}

template<class ShaderClass>
std::string GetLatestProfile();

template<>
std::string GetLatestProfile<ID3D11VertexShader>()
{
    assert(D3dDevice);

    D3D_FEATURE_LEVEL featureLevel = D3dDevice->GetFeatureLevel();

    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
        return "vs_5_0";
        break;
    case D3D_FEATURE_LEVEL_10_1:
        return "vs_4_1";
        break;
    case D3D_FEATURE_LEVEL_10_0:
        return "vs_4_0";
        break;
    case D3D_FEATURE_LEVEL_9_3:
        return "vs_4_0_level_9_3";
        break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
        return "vs_4_0_level_9_1";
        break;
    }

    return "";
}

template<>
std::string GetLatestProfile<ID3D11PixelShader>()
{
    assert(D3dDevice);

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = D3dDevice->GetFeatureLevel();
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "ps_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "ps_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "ps_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "ps_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "ps_4_0_level_9_1";
    }
    break;
    }
    return "";
}

bool LoadContent()
{
    assert(D3dDevice);

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.ByteWidth = sizeof(VertexPosColor) * _countof(Vertices);
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));

    resourceData.pSysMem = Vertices;

    HRESULT hr = D3dDevice->CreateBuffer(&vertexBufferDesc, &resourceData, &D3dVertexBuffer);
    if (FAILED(hr))
        return false;

    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.ByteWidth = sizeof(WORD) * _countof(Indicies);
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    resourceData.pSysMem = Indicies;

    hr = D3dDevice->CreateBuffer(&indexBufferDesc, &resourceData, &D3dIndexBuffer);
    if (FAILED(hr))
        return false;

    D3D11_BUFFER_DESC constantBufferDesc;
    ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
    constantBufferDesc.CPUAccessFlags = 0;
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    hr = D3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &D3dConstantBuffers[CB_Application]);
    if (FAILED(hr))
        return false;

    hr = D3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &D3dConstantBuffers[CB_Frame]);
    if (FAILED(hr))
        return false;

    hr = D3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &D3dConstantBuffers[CB_Object]);
    if (FAILED(hr))
        return false;

#pragma region ShadersLoading
    D3dVertexShader = LoadShader<ID3D11VertexShader>(L"inc/SimpleVertexShader.hlsl", "SimpleVertexShader", "latest");
    D3dPixelShader = LoadShader<ID3D11PixelShader>(L"inc/SimplePixelShader.hlsl", "SimplePixelShader", "latest");

    /*ID3DBlob* vertexShaderBlob;
#if _DEBUG
    LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader_d.cso";
#else
    LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader.cso";
#endif

    hr = D3DReadFileToBlob(compiledVertexShaderObject, &vertexShaderBlob);
    if (FAILED(hr))
    {
        return false;
    }

    hr = D3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &g_d3dVertexShader);
    if (FAILED(hr))
    {
        return false;
    }*/

    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = D3dDevice->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc), D3dVertexShader->GetBufferPointer(), D3dVertexShader->GetBufferSize(), &D3dInputLayout);
    if (FAILED(hr))
    {
        return false;
    }
#pragma endregion ShadersLoading

    // Setup the projection matrix.
    RECT clientRect;
    GetClientRect(WindowHandle, &clientRect);

    // Compute the exact client dimensions.
    // This is required for a correct projection matrix.
    float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
    float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

    ProjMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);

    D3dDeviceContext->UpdateSubresource(D3dConstantBuffers[CB_Application], 0, nullptr, &ProjMatrix, 0, 0);

    return true;
}

void UnloadContent()
{

}

template<class ShaderClass>
ShaderClass* CreateShader(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinckage);

template <>
ID3D11VertexShader* CreateShader<ID3D11VertexShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinckage)
{
    assert(D3dDevice);
    assert(pShaderBlob);

    ID3D11VertexShader* pVertexShader = nullptr;
    D3dDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinckage, &pVertexShader);

    return pVertexShader;
}

template <>
ID3D11PixelShader* CreateShader<ID3D11PixelShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinckage)
{
    assert(D3dDevice);
    assert(pShaderBlob);

    ID3D11PixelShader* pPixelShader = nullptr;
    D3dDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinckage, &pPixelShader);

    return pPixelShader;
}

template<class ShaderClass>
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile)
{
    ID3DBlob* pShaderBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;
    ShaderClass* pShader = nullptr;

    std::string profile = _profile;
    if (profile == "latest")
    {
        profile = GetLatestProfile<ShaderClass>();
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompileFromFile(
        fileName.c_str(), nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile.c_str(),
        flags, 0, &pShaderBlob, &pErrorBlob
    );

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            std::string errorMessage = (char*)pErrorBlob->GetBufferPointer();
            OutputDebugStringA(errorMessage.c_str());

            SafeRelease(pShaderBlob);
            SafeRelease(pErrorBlob);
        }

        return false;
    }

    pShader = CreateShader<ShaderClass>(pShaderBlob, nullptr);

    SafeRelease(pShaderBlob);
    SafeRelease(pErrorBlob);

    return pShader;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
    {
        hdc = BeginPaint(hwnd, &paintStruct);
        EndPaint(hwnd, &paintStruct);
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    default: 
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

int Run()
{
    MSG msg = { 0 };

    static DWORD prevTime = timeGetTime();
    static const float targetFramerate = 30.0f;
    static const float maxTimeStep = 1.0f / targetFramerate;

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            DWORD currentTime = timeGetTime();
            float deltaTime = (currentTime - prevTime) / 1000.0f;
            deltaTime = std::min<float>(deltaTime, maxTimeStep);

            Update(deltaTime);
            Render();
        }
    }

    return static_cast<int>(msg.wParam);
}

void Update(float deltaTime)
{
    XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
    XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
    XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    D3dDeviceContext->UpdateSubresource(D3dConstantBuffers[CB_Frame], 0, nullptr, &ViewMatrix, 0, 0);


    static float angle = 0.0f;
    angle += 90.0f * deltaTime;
    XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);

    WorldMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
    D3dDeviceContext->UpdateSubresource(D3dConstantBuffers[CB_Object], 0, nullptr, &WorldMatrix, 0, 0);
}

void Render()
{

}

void Cleanup()
{
    
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);

    if (!XMVerifyCPUSupport())
    {
        MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (InitApplication(hInstance, cmdShow) != 0)
    {
        MessageBox(nullptr, TEXT("Failed to create applicaiton window."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (InitDirectX(hInstance, EnableVsync) != 0)
    {
        MessageBox(nullptr, TEXT("Failed to create DirectX device and swap chain."), TEXT("Error"), MB_OK);
        return -1;
    }

    int ret = Run();

	return ret;
}