#pragma once

#include "pch.h"

// Dxgi1_5.lib
// DXGI.lib

namespace t
{
    /*
    enum class Shader_type
    {
        Vertex, Hull, Domain, Geometry, Pixel
    };
    */

    class App {
    private:
        HINSTANCE m_instance;
        HWND m_window;

        unsigned int m_client_width = 0;
        unsigned int m_client_height = 0;

        Microsoft::WRL::ComPtr<ID3D11Device1> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> m_device_context;

        Microsoft::WRL::ComPtr<IDXGIFactory2> m_dxgi_factory;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swap_chain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_back_buffer_view;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertex_shader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixel_shader;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_input_layout;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertex_buffer;

        int create_dd_resources(); // dd = device dependent
        int create_swapchain_resources();

        //template<typename T>
        //int load_shader(const char* filepath, Shader_type type, T** shader);

        int render();
    public:
        App(HINSTANCE instance);
        int initialize();
        void clean_up();

        HWND get_window() const;

        static long long CALLBACK s_wndproc(HWND wnd, unsigned int msg, unsigned long long wparam, long long lparam);
        long long CALLBACK wndproc(HWND wnd, unsigned int msg, unsigned long long wparam, long long lparam);
    };
}
