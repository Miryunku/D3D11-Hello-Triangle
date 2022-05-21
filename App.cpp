#include "pch.h"
#include "App.h"
#include "Helpers.h"
#include "VertexStructures.h"

t::App::App(HINSTANCE instance)
    : m_instance(instance),
    m_window(nullptr)
{

}

int t::App::initialize()
{
    // Register the main window class

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = s_wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_instance;
    wc.hIcon = static_cast<HICON>(LoadImage(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));
    wc.hCursor = static_cast<HCURSOR>(LoadImage(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));
    wc.lpszClassName = L"main-window";
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.hIconSm = static_cast<HICON>(LoadImage(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));

    if (!RegisterClassEx(&wc))
    {
        return E_FAIL;
    }

    // Create the main window

    unsigned int dpi = GetDpiForWindow(GetDesktopWindow());
    RECT window_area;
    window_area.left = 0;
    window_area.top = 0;
    window_area.right = std::ceil(800.0f * (dpi / 96.0f));
    window_area.bottom = std::ceil(600.0f * (dpi / 96.0f));

    if (!AdjustWindowRectExForDpi(&window_area, WS_OVERLAPPEDWINDOW, false, 0u, dpi))
    {
        return E_FAIL;
    }

    m_window = CreateWindowEx(0u, L"main-window", L"Lluvia (version 1.0.0)", WS_OVERLAPPEDWINDOW, 20, 20, window_area.right - window_area.left, window_area.bottom - window_area.top, nullptr, nullptr, m_instance, this);
    if (!m_window)
    {
        return E_FAIL;
    }

    RECT rc;
    GetClientRect(m_window, &rc);
    m_client_width = rc.right;
    m_client_height = rc.bottom;

    // Create the device dependent resources

    int hr = create_dd_resources();
    if (FAILED(hr))
    {
        // MessageBoxEx inside create_dd_resources
        return hr;
    }

    // Create resources that depend on the swap chain

    hr = create_swapchain_resources();
    if (FAILED(hr))
    {
        // MessageBoxEx inside create_swapchain_resources
        return hr;
    }

    return hr;
}

void t::App::clean_up()
{

}

long long CALLBACK t::App::s_wndproc(HWND wnd, unsigned int msg, unsigned long long wparam, long long lparam)
{
    App* app = nullptr;

    if (msg == WM_CREATE)
    {
        CREATESTRUCT* creation_info = reinterpret_cast<CREATESTRUCT*>(lparam);
        app = reinterpret_cast<App*>(creation_info->lpCreateParams);
        SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<long long>(app));

        return DefWindowProc(wnd, msg, wparam, lparam);
    }
    else
    {
        app = reinterpret_cast<App*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
        if (app)
        {
            return app->wndproc(wnd, msg, wparam, lparam);
        }
        else
        {
            return DefWindowProc(wnd, msg, wparam, lparam);
        }
    }
}

long long CALLBACK t::App::wndproc(HWND wnd, unsigned int msg, unsigned long long wparam, long long lparam)
{
    switch (msg)
    {
        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);

            render();

            EndPaint(wnd, &ps);
            return 0;
        }

        case WM_CLOSE:
        {
            DestroyWindow(wnd);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        default:
            return DefWindowProc(wnd, msg, wparam, lparam);
    }
}

HWND t::App::get_window() const
{
    return m_window;
}

int t::App::create_dd_resources()
{
    if (m_device)
    {
        return S_OK;
    }

    // Create the device and device context

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;

    const D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0};

    int hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, feature_levels, 1u, D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, device_context.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"Device creation failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    hr = device->QueryInterface(m_device.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"Device upcast failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    hr = device_context->QueryInterface(m_device_context.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"Device context upcast failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    // Fetch the DXGI factory

    Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device;
    hr = m_device->QueryInterface(dxgi_device.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"IDXGIDevice creation failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    hr = dxgi_device->GetAdapter(adapter.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"IDXGIAdapter fetch failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    hr = adapter->GetParent(IID_PPV_ARGS(m_dxgi_factory.GetAddressOf()));
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"IDXGIFactory2 fetch failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    // Create the swap chain

    DXGI_SWAP_CHAIN_DESC1 desc1;
    desc1.Width = m_client_width;
    desc1.Height = m_client_height;
    desc1.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc1.Stereo = false;
    desc1.SampleDesc.Count = 1u;
    desc1.SampleDesc.Quality = 0u;
    desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc1.BufferCount = 2u;
    desc1.Scaling = DXGI_SCALING_STRETCH;
    desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc1.Flags = 0u;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_desc;
    fullscreen_desc.RefreshRate.Numerator = 60u;
    fullscreen_desc.RefreshRate.Denominator = 1u;
    fullscreen_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    fullscreen_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    fullscreen_desc.Windowed = true;

    hr = m_dxgi_factory->CreateSwapChainForHwnd(m_device.Get(), m_window, &desc1, &fullscreen_desc, nullptr, m_swap_chain.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"SwapChain1 creation failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    // Load vertex shader bytecode

    size_t bytecode_buffer_size = 0;
    unsigned char* vs_bytecode = load_binary_file("Main.VS.cso", &bytecode_buffer_size);
    if (!vs_bytecode)
    {
        MessageBoxEx(nullptr, L"Load bytecode for vertex shader failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return E_FAIL;
    }

    // Create input layout

    constexpr D3D11_INPUT_ELEMENT_DESC input_elems_desc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = m_device->CreateInputLayout(input_elems_desc, _countof(input_elems_desc), vs_bytecode, bytecode_buffer_size, m_input_layout.GetAddressOf());
    if (FAILED(hr))
    {
        free(vs_bytecode);
        MessageBoxEx(nullptr, L"Input layout creation failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    // Create the ID3D11VertexShader

    hr = m_device->CreateVertexShader(vs_bytecode, bytecode_buffer_size, nullptr, m_vertex_shader.GetAddressOf());
    if (FAILED(hr))
    {
        free(vs_bytecode);
        MessageBoxEx(nullptr, L"ID3D11VertexShader creation failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    free(vs_bytecode);

    // Load the pixel shader bytecode and then create the ID3D11PixelShader

    unsigned char* ps_bytecode = load_binary_file("Main.PS.cso", &bytecode_buffer_size);
    if (!ps_bytecode)
    {
        MessageBoxEx(nullptr, L"Load bytecode for pixel shader failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return E_FAIL;
    }

    hr = m_device->CreatePixelShader(ps_bytecode, bytecode_buffer_size, nullptr, m_pixel_shader.GetAddressOf());
    if (FAILED(hr))
    {
        free(ps_bytecode);
        MessageBoxEx(nullptr, L"Pixel shader creation failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    free(ps_bytecode);

    // Create the vertex buffer

    constexpr Vertex_data vertex_data[] =
    {
        // Data order: Position (FLOAT4), Color (FLOAT4)
        { {0.0f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
        { {0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
        { {-0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }
    };

    D3D11_BUFFER_DESC buffer_desc;
    buffer_desc.ByteWidth = sizeof(vertex_data);
    buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA subresource_data;
    subresource_data.pSysMem = vertex_data;
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;

    hr = m_device->CreateBuffer(&buffer_desc, &subresource_data, m_vertex_buffer.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"VertexBuffer creation failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    return hr;
}

int t::App::create_swapchain_resources()
{
    if (!m_swap_chain)
    {
        return E_FAIL;
    }

    // Create the render target view

    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    int hr = m_swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"Back buffer fetch failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    hr = m_device->CreateRenderTargetView(back_buffer.Get(), nullptr, m_back_buffer_view.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBoxEx(nullptr, L"CreateRenderTargetView for back buffer failed", L"Error", MB_ICONERROR | MB_OK, 0);
        return hr;
    }

    return hr;
}

int t::App::render()
{
    // IA Stage

    unsigned int stride = sizeof(Vertex_data);
    unsigned int offset = 0;
    m_device_context->IASetInputLayout(m_input_layout.Get());
    m_device_context->IASetVertexBuffers(0, 1, m_vertex_buffer.GetAddressOf(), &stride, &offset);
    m_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // VS Stage

    m_device_context->VSSetShader(m_vertex_shader.Get(), nullptr, 0);

    // RS Stage
    
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = m_client_width;
    viewport.Height = m_client_height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_device_context->RSSetViewports(1, &viewport);

    // PS Stage

    m_device_context->PSSetShader(m_pixel_shader.Get(), nullptr, 0);

    // OM Stage

    m_device_context->OMSetRenderTargets(1, m_back_buffer_view.GetAddressOf(), nullptr);

    // Render
    
    constexpr float clearing_color[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    m_device_context->ClearRenderTargetView(m_back_buffer_view.Get(), clearing_color);
    m_device_context->Draw(3, 0);
    m_swap_chain->Present(1, 0);

    return 0;
}
/*
template<typename T>
int t::App::load_shader(const char* filepath, Shader_type type, T** shader)
{
    size_t buffer_size = 0;
    unsigned char* buffer = load_binary_file(filepath, &buffer_size);
    if (!buffer)
    {
        return E_FAIL;
    }

    switch (type)
    {
        case Shader_type::Vertex:
        {
            int hr = m_device->CreateVertexShader(buffer, buffer_size, nullptr, shader);
            free(buffer);
            return hr;
        }

        case Shader_type::Pixel:
        {
            int hr = m_device->CreatePixelShader(buffer, buffer_size, nullptr, shader);
            free(buffer);
            return hr;
        }

        default:
        {
            free(buffer);
            return E_FAIL;
        }
    }
}

template int t::App::load_shader<ID3D11VertexShader>(const char* filepath, Shader_type type, ID3D11VertexShader** shader);
template int t::App::load_shader<ID3D11PixelShader>(const char* filepath, Shader_type type, ID3D11PixelShader** shader);
*/