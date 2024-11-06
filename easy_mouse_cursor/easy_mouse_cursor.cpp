// easy_mouse_cursor.cpp : 定义应用程序的入口点。
//

#include "pch.h"
#include "framework.h"
#include "easy_mouse_cursor.h"

#include <string>
#include <format>
#include <cmath>
#include <vector>
#include <atlimage.h>

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define IMG_PATH "f:\\下载目录\\test.bmp"

class WndMouse
{
    static const int WND_WIDTH = 30;

    HWND _wnd = nullptr;

    UINT    _dpi = 0;
    long    _width = 0;
    long    _height = 0;

public:
    CImage _img;
public:
    LRESULT static CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_PAINT:
            {



                /*
                                PAINTSTRUCT ps;
                                HDC hdc = BeginPaint(hwnd, &ps);
                                FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
                                EndPaint(hwnd, &ps);*/
                return 0;
            }
            case WM_DPICHANGED:
            {
                UINT newDpi = LOWORD(wParam);
                //std::string es;
                //es = std::format("{}", newDpi);
                //::MessageBoxA(hwnd, es.c_str(), nullptr, MB_OK);

                WndMouse* pMouseWnd = (WndMouse*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                pMouseWnd->redraw();

                return 0;
            }
            case WM_SETCURSOR: {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }
            case WM_SHOWWINDOW:
            {
                WndMouse* pMouseWnd = (WndMouse*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                pMouseWnd->redraw();

                ::OutputDebugStringA("显示了\n");
             //break;
            }
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    //生成鼠标大小的图片
    void    gen_image(CImage& img)
    {
        double dpi = (double)GetDpiForWindow(_wnd) / 96;

        double src_width = WND_WIDTH * dpi;

        double dbRatio = src_width / _img.GetWidth();
        int w = (int)(dbRatio * _img.GetWidth());
        int h = (int)(dbRatio * _img.GetHeight());

        img.Create(w, h, 32);

        HDC destDC = img.GetDC();
        ::SetStretchBltMode(destDC, HALFTONE);
        _img.StretchBlt(destDC, 0, 0, w, h);
        img.ReleaseDC();

        int x = 0;
    }

    void test(POINT& pt)
    {

        //从项目目录下，加载图片文件
        HBITMAP hBmp = (HBITMAP)LoadImageA(NULL, IMG_PATH, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        //创建并获取图片信息
        BITMAP bmp;
        GetObject(hBmp, sizeof(BITMAP), &bmp);

        //获取图片宽高
        int width = bmp.bmWidth;
        int height = bmp.bmHeight;
        {
            //计算大小
            double dpi = (double)GetDpiForWindow(_wnd) / 96;
            double src_width = WND_WIDTH * dpi;
            double dbRatio = src_width / bmp.bmWidth;
            width = (int)(dbRatio * bmp.bmWidth);
            height = (int)(dbRatio * bmp.bmHeight);
        }
        
        ::SetWindowPos(_wnd, HWND_TOPMOST, pt.x, pt.y, width, height, SWP_NOREDRAW);//将窗口大小设置为图片大小使之相互合适

        auto hWnd = _wnd;

        //指定窗口客户端区域的DC的句柄。若hWnd=NULL，则提取整个屏幕的DC
        HDC hdc = GetDC(hWnd);
        //该函数创建一个与指定设备兼容的内存设备上下文环境（DC）
        HDC hdcMem = CreateCompatibleDC(hdc);
        //这里把hbmp的位图选择到兼容DC memdc,之后这个兼容DC就拥有和hbmp同样大小的绘图区域,注意超出位图返回的GDI输出都是无效的
        SelectObject(hdcMem, hBmp);

        //显示窗口
        //ShowWindow(hWnd, SW_SHOW);

        //绘制一定要在ShowWindow之后调用，否则绘图无效
        //BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

        SetStretchBltMode(hdc, HALFTONE);
        StretchBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

        /*
                //为样式添加附加层，用来支持透明样式
                LONG nExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
                nExStyle |= WS_EX_LAYERED;
                SetWindowLong(hWnd, GWL_EXSTYLE, nExStyle);*/
                //设置窗体属性
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 128, LWA_COLORKEY);

        //函数释放设备上下文环境（DC）供其他应用程序使用
        ReleaseDC(hWnd, hdc);
        ReleaseDC(hWnd, hdcMem);

        UpdateWindow(hWnd);

        //OnForceShow(hWnd);
    }

    //重绘鼠标
    void    redraw(POINT& pt)
    {
        test(pt);
        return;

        CImage img;
        gen_image(img);

        SIZE sz;//图片大小
        sz.cx = img.GetWidth();
        sz.cy = img.GetHeight();

        ::SetWindowPos(_wnd, NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOREDRAW);//将窗口大小设置为图片大小使之相互合适

        HDC hdc = ::GetDC(_wnd);//获取窗口设备句柄
        HDC hdcMem = CreateCompatibleDC(hdc);//创建一个与hdc相兼容的内存设备句柄
        HBITMAP    hBitmap = CreateCompatibleBitmap(hdc, sz.cx, sz.cy);
        SelectObject(hdcMem, (HGDIOBJ)hBitmap);
        img.Draw(hdcMem, 0, 0, sz.cx, sz.cy, 0, 0, sz.cx, sz.cy);

        //这下面是测试
        BLENDFUNCTION  bf = {0};
        bf.AlphaFormat = AC_SRC_ALPHA;
        bf.BlendOp = AC_SRC_OVER;
        bf.SourceConstantAlpha = 255;

        POINT ptdraw = {0};
        ::UpdateLayeredWindow(_wnd, hdc, NULL, &sz, hdcMem, &ptdraw, NULL, &bf, ULW_ALPHA);

        ::DeleteDC(hdcMem);
        ::ReleaseDC(_wnd, hdc);
    }
    void    redraw()
    {
        POINT pt = {0};
        GetCursorPos(&pt);
        redraw(pt);
    }

    bool    create()
    {
        const char CLASS_NAME[] = "easy_mouse_cursor_wnd";

        auto hInstance = ::GetModuleHandle(nullptr);

        WNDCLASSA wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);

        //任务管理器的置顶
        //https://stackoverflow.com/questions/39246590/is-task-manager-a-special-kind-of-always-on-top-window-for-windows-10

        DWORD dwExtStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_APPWINDOW;
        dwExtStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;     //穿透

        POINT pt = {0};
        GetCursorPos(&pt);

        _wnd = CreateWindowExA(dwExtStyle, CLASS_NAME, NULL, WS_POPUP, pt.x, pt.y, WND_WIDTH, 5
            , NULL, NULL, hInstance, NULL);

        SetWindowLongPtr(_wnd, GWLP_USERDATA, (LONG)this);

        //redraw();

        //SetLayeredWindowAttributes(_wnd, RGB(0, 0, 0), 255, LWA_ALPHA);

        ShowWindow(_wnd, SW_SHOW);
        redraw();

        return true;
    }

    void    move(POINT& pt)
    {
        /*
                // 获取图标信息
                CURSORINFO ci = {sizeof(CURSORINFO)};
                GetCursorInfo(&ci);*/

        SetWindowPos(_wnd, nullptr, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
};

WndMouse wndMouse;

static HHOOK	_hookMouse = nullptr;
LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    //console.time();
    LRESULT lresult = false;
    //if (nCode < 0)
    //{
    lresult = CallNextHookEx(_hookMouse, nCode, wParam, lParam);
    //}

    auto hookinfo = (MOUSEHOOKSTRUCT*)lParam;
    auto& pt = hookinfo->pt;

    std::string dbgstr;
    dbgstr = std::format("{}*{}\r\n", pt.x, pt.y);
    //::OutputDebugStringA(dbgstr.c_str());

    wndMouse.move(pt);

    return lresult;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    // Enable per-monitor DPI-awareness version 2
/*
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
    {
        return 1;
    }*/

    auto hr = wndMouse._img.Load(IMG_PATH);

    wndMouse.create();

    _hookMouse = SetWindowsHookExA(WH_MOUSE_LL, MouseHookProc, hInstance, NULL);

    MSG msg;
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EASYMOUSECURSOR));
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


    return 0;
}