/*
   Inputune prototype
*/

#define _WIN32_WINNT 0x0555
#define  WIN32_LEAN_AND_MEAN
#pragma warning (disable: 4514)   
#pragma warning (push, 1)
#include <windows.h>
#include <mmsystem.h>
#pragma warning (pop)

#pragma comment (lib, "user32")
#pragma comment (lib, "kernel32")
#pragma comment(lib, "winmm")


const LPCSTR FWWindowClass = "CWInputune";

typedef struct {
  int MIDIport;
  union { unsigned long dw; unsigned char b[4];} midi;
} ipt_config;

static HHOOK g_hook = NULL, g_hook2 = NULL;
static HINSTANCE hInst;
static int delta = 3;
static HMIDIOUT dev = 0;
static ipt_config cfg = {0, 0x000023c0}; // 0dc0
static DWORD o_vK = 0;


LRESULT CALLBACK WndProc ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK mhcbf ( int, WPARAM, LPARAM );
LRESULT CALLBACK khcbf( int, WPARAM, LPARAM);
__forceinline void sethook ( void );
__forceinline void rm_hook ( void );

int PASCAL WinMain (HINSTANCE hInstance,HINSTANCE hPrev,LPSTR lpCmd,int nShow)
{
  WNDCLASSEX wc = {0};
  HWND hWnd;
  MSG msg;

  if ( (hWnd = FindWindow(FWWindowClass, NULL)) != NULL )
  {
    SendMessage(hWnd, WM_CLOSE, 0, 0);
    return 1;
  }

  SystemParametersInfo( SPI_GETWHEELSCROLLLINES, sizeof(delta), &delta, 0);
  delta <<=3;


  hInst = hInstance;
  wc.cbSize        = sizeof(WNDCLASSEX);
  wc.lpfnWndProc   = WndProc;
  wc.hInstance     = hInstance;
  wc.lpszClassName = FWWindowClass;

  if(!RegisterClassEx(&wc)) return 1;

  hWnd = CreateWindowEx( WS_EX_TOOLWINDOW,
                         FWWindowClass,
                         NULL,
                         WS_OVERLAPPEDWINDOW | WS_ICONIC,
                         1, 1, 1, 1,
                         NULL, NULL, hInstance, NULL);
  if (hWnd != NULL) 
  {
     ShowWindow(hWnd, SW_HIDE);
     while ( GetMessage(&msg, NULL, 0, 0) > 0 )
     {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
     }
  }

  return 0;
} // WinMain


// --------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            Beep(1024, 50); 
            if (midiOutOpen(&dev, cfg.MIDIport, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) dev = 0;
            if (dev) midiOutShortMsg(dev, cfg.midi.dw);
            sethook();
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            rm_hook();
            if (dev) {midiOutReset(dev); midiOutClose(dev);}
            Beep(440, 70);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
} // WndProc


// ---------------------------------------------------------------------------
void sethook(void)
{
//  if ( g_hook == NULL ) g_hook = SetWindowsHookEx(WH_MOUSE_LL, mhcbf, hInst,0); 
  if ( g_hook2 == NULL ) g_hook2 = SetWindowsHookEx(WH_KEYBOARD_LL, khcbf, hInst,0); 
} // sethook


// ---------------------------------------------------------------------------
void rm_hook(void)
{
//  if ( g_hook != NULL ) if ( UnhookWindowsHookEx( g_hook) ) g_hook = NULL;
  if ( g_hook2 != NULL ) if ( UnhookWindowsHookEx( g_hook2) ) g_hook2 = NULL;
} // rm_hook


// ---------------------------------------------------------------------------
LRESULT CALLBACK mhcbf( int ncode, WPARAM wparam, LPARAM lparam)
{
  register PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT) lparam;
    
  if ( ncode == HC_ACTION )
  {
    if ( (wparam == WM_MOUSEWHEEL) ) Beep(770 + (int)((p->mouseData >> 16)/delta),1);
    else if ( wparam == WM_MOUSEMOVE ) Beep(42 + p->pt.x + p->pt.y,  1);
  }

  return ( CallNextHookEx(NULL, ncode, wparam, lparam) );
} // mhcbf


// ---------------------------------------------------------------------------
LRESULT CALLBACK khcbf( int ncode, WPARAM wparam, LPARAM lparam)
{
  register PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lparam;
  register unsigned long dw;
    
  if ( ncode == HC_ACTION )
  {
    if ( (wparam == WM_KEYDOWN) && (p->vkCode != o_vK)) 
    {
      o_vK = p->vkCode;
      dw = ((127 - p->vkCode % 128) << 8) | 0x00640090;
      if (dev) midiOutShortMsg(dev, dw);
    }
    else if ( wparam == WM_KEYUP) 
    {
      o_vK = 0;
      dw = ((127 - p->vkCode % 128) << 8) | 0x00000090;
      if (dev) midiOutShortMsg(dev, dw);
    }
  }

  return ( CallNextHookEx(NULL, ncode, wparam, lparam) );
} // khcbf
