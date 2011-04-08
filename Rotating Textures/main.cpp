#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
using std::clog;
using std::left;
using std::right;

#include <iomanip>
using std::setw;
using std::setprecision;

#include <cstdlib>
using std::system;
using std::exit;

#include <GL/glew.h>
#include <GL/glut.h>

#include <windows.h>
#include <WinSpool.h>
#include <tchar.h>

#include "resource.h"

LRESULT CALLBACK soleWindowProcedure(HWND window,UINT message,WPARAM argW,LPARAM argL);

int main()
{
	WNDCLASS soleWindowClass;
	soleWindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	soleWindowClass.lpfnWndProc = soleWindowProcedure;
	soleWindowClass.cbClsExtra = 0;
	soleWindowClass.cbWndExtra = 0;
	soleWindowClass.hInstance = GetModuleHandle(nullptr);
	soleWindowClass.hIcon = LoadIcon(GetModuleHandle(nullptr),_T("RotatingTexturesIcon"));	// can be null
	soleWindowClass.hCursor = LoadCursor(GetModuleHandle(nullptr),MAKEINTRESOURCE(IDC_CROSSHAIR));	// can be null
	soleWindowClass.hbrBackground = nullptr;
	soleWindowClass.lpszMenuName = _T("RotatingTexturesMenu");
	soleWindowClass.lpszClassName = _T("RotatingTexturesClass");
	RegisterClass(&soleWindowClass);

	HWND window = CreateWindow(_T("RotatingTexturesClass"),_T("Rotating Textures"),WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN
									|WS_CLIPSIBLINGS,320,120,640,480,nullptr,nullptr,GetModuleHandle(nullptr),nullptr);

	ShowWindow(window,SW_SHOWNORMAL);
	UpdateWindow(window);

	HMENU sysMenu = GetSystemMenu(window,FALSE);
	AppendMenu(sysMenu,MF_SEPARATOR,0,nullptr);
	AppendMenu(sysMenu,MF_STRING,IDM_FILE_OPEN,_T("&Open..."));
	AppendMenu(sysMenu,MF_STRING,IDM_SYS_BEEP,"&Beep");

	MSG message;
	while(GetMessage(&message,nullptr,0,0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	} // end while

	return message.wParam;
} // end function main

void display();
float angle = 0; // in degrees

LRESULT CALLBACK soleWindowProcedure(HWND window,UINT message,WPARAM argW,LPARAM argL)
{
	static PIXELFORMATDESCRIPTOR pixelFormatDescription = {0};
	static HDC windowSurface;
	static HGLRC glContext;
	int pixelFormatIndex;
	RECT r;
	HBITMAP gdiBitmap;
	static HDC gdiMemContext;
	static HGLRC glMemContext;

	switch(message)
	{
	case WM_CREATE:
		pixelFormatDescription.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pixelFormatDescription.nVersion = 1;
		pixelFormatDescription.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER|PFD_STEREO_DONTCARE;
		pixelFormatDescription.iPixelType = PFD_TYPE_RGBA;
		pixelFormatDescription.cColorBits = 64;
		pixelFormatDescription.cAlphaBits = 16;
		pixelFormatDescription.cAccumBits = 128;
		pixelFormatDescription.cDepthBits = 64;
		pixelFormatDescription.cStencilBits = 32;
		pixelFormatDescription.cAuxBuffers = 128;
		pixelFormatDescription.iLayerType = PFD_MAIN_PLANE;

		windowSurface = GetDC(window);
		pixelFormatIndex = ChoosePixelFormat(windowSurface,&pixelFormatDescription);
		SetPixelFormat(windowSurface,pixelFormatIndex,&pixelFormatDescription);
		glContext = wglCreateContext(windowSurface);
		wglMakeCurrent(windowSurface,glContext);
		glewInit();

		glClearColor(1,1,0.941,1);	// ivory
		return 0;
	case WM_SIZE:
		GetClientRect(window,&r);
		glViewport(0,0,r.right,r.bottom);
		return 0;
	case WM_PAINT:
		display();
		angle += 1.5;
		SwapBuffers(windowSurface);
		ValidateRect(window,nullptr);
		InvalidateRect(window,nullptr,FALSE);
		return 0;
	case WM_SYSCOMMAND:
		switch(LOWORD(argW))
		{
		case IDM_SYS_BEEP:
			MessageBeep(0);
			return 0;
		case IDM_FILE_OPEN:
			SendMessage(window,WM_COMMAND,MAKEWPARAM(IDM_FILE_OPEN,0),0);
			return 0;
		} // end switch
		break;
	case WM_COMMAND:
		switch(LOWORD(argW))
		{
		case IDM_FILE_OPEN:
			MessageBeep(0);
			return 0;
		case IDM_EDIT_COPY:
			GetClientRect(window,&r);
			gdiBitmap = CreateCompatibleBitmap(windowSurface,r.right,r.bottom);
			ZeroMemory(&pixelFormatDescription,sizeof(PIXELFORMATDESCRIPTOR));
			pixelFormatDescription.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pixelFormatDescription.nVersion = 1;
			pixelFormatDescription.dwFlags = PFD_DRAW_TO_BITMAP|PFD_SUPPORT_OPENGL|PFD_STEREO_DONTCARE;
			pixelFormatDescription.iPixelType = PFD_TYPE_RGBA;
			pixelFormatDescription.cColorBits = 32;
			pixelFormatDescription.cAlphaBits = 8;
			pixelFormatDescription.cAccumBits = 0;
			pixelFormatDescription.cDepthBits = 0;
			pixelFormatDescription.cStencilBits = 0;
			pixelFormatDescription.cAuxBuffers = 0;
			pixelFormatDescription.iLayerType = PFD_MAIN_PLANE;

			gdiMemContext = CreateCompatibleDC(windowSurface);
			gdiBitmap = (HBITMAP)SelectObject(gdiMemContext,gdiBitmap);
			pixelFormatIndex = ChoosePixelFormat(gdiMemContext,&pixelFormatDescription);
			SetPixelFormat(gdiMemContext,pixelFormatIndex,&pixelFormatDescription);
			glMemContext = wglCreateContext(gdiMemContext);
			wglMakeCurrent(gdiMemContext,glMemContext);

			glClearColor(1,1,0.941,1);	// ivory
			glViewport(0,0,r.right,r.bottom);
			display();
			glFinish();	// essential!
			gdiBitmap = (HBITMAP)SelectObject(gdiMemContext,gdiBitmap);

			OpenClipboard(window);
			EmptyClipboard();
				SetClipboardData(CF_BITMAP,gdiBitmap);
			CloseClipboard();

			wglMakeCurrent(windowSurface,glContext);
			wglDeleteContext(glMemContext);
			return 0;
		case IDM_FILE_PRINT:
			{
			// printer OpenGL contexts.
			unsigned long requiredSize;
			unsigned long nPrinters;
			HDC gdiPrinterDC;
			HGLRC glPrinterContext;
			DOCINFO di = {0};
			di.cbSize = sizeof(di);
			di.lpszDocName = _T("Test");

			EnumPrinters(PRINTER_ENUM_LOCAL,nullptr,4,nullptr,0,&requiredSize,&nPrinters);	// get required size.
			BYTE *printers = new BYTE[requiredSize];
				EnumPrinters(PRINTER_ENUM_LOCAL,nullptr,4,printers,requiredSize,&requiredSize,&nPrinters);	// get printers list.
				for(unsigned long i = 0 ; i < 1/*nPrinters*/ ; ++i)
				{
					cout << setw(4) << ' ' << "OpenGL context for " << ((PRINTER_INFO_4*)printers)[i].pPrinterName << ".\n";
					gdiPrinterDC = CreateDC(nullptr,((PRINTER_INFO_4*)printers)[i].pPrinterName,nullptr,nullptr);
						ZeroMemory(&pixelFormatDescription,sizeof(PIXELFORMATDESCRIPTOR));
						pixelFormatDescription.nSize = sizeof(PIXELFORMATDESCRIPTOR);
						pixelFormatDescription.nVersion = 1;
						pixelFormatDescription.dwFlags = PFD_SUPPORT_OPENGL|PFD_STEREO_DONTCARE;
						pixelFormatDescription.iPixelType = PFD_TYPE_RGBA;
						pixelFormatDescription.cColorBits = 32;
						pixelFormatDescription.cAlphaBits = 8;
						pixelFormatDescription.cAccumBits = 0;
						pixelFormatDescription.cDepthBits = 0;
						pixelFormatDescription.cStencilBits = 0;
						pixelFormatDescription.cAuxBuffers = 0;
						pixelFormatDescription.iLayerType = PFD_MAIN_PLANE;

						pixelFormatIndex = ChoosePixelFormat(gdiPrinterDC,&pixelFormatDescription);
						SetPixelFormat(gdiPrinterDC,pixelFormatIndex,&pixelFormatDescription);
						StartDoc(gdiPrinterDC,&di);
						StartPage(gdiPrinterDC);
							glPrinterContext = wglCreateContext(gdiPrinterDC);
							wglMakeCurrent(gdiPrinterDC,glPrinterContext);

							SelectObject(gdiPrinterDC,GetStockObject(GRAY_BRUSH));
							Rectangle(gdiPrinterDC,0,0,640,640);

							glClearColor(1,1,0.941,1);	// ivory
							glViewport(0,0,GetDeviceCaps(gdiPrinterDC,HORZRES),GetDeviceCaps(gdiPrinterDC,VERTRES));
							display();
							glFinish();	// essential!
						EndPage(gdiPrinterDC);
						EndDoc(gdiPrinterDC);
					wglMakeCurrent(windowSurface,glContext);
					wglDeleteContext(glPrinterContext);
					DeleteDC(gdiPrinterDC);
				} // end for
			delete[] printers;
			}
			return 0;
		} // end switch
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_DESTROY:
		wglDeleteContext(glContext);
		DeleteDC(gdiMemContext);
		//ReleaseDC(window,windowSurface);
		PostQuitMessage(0);
		return 0;
	} // end switch
	return DefWindowProc(window,message,argW,argL);
} // end function soleWindowProcedure

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1,0,0);	// red
	glLoadIdentity();
	glTranslatef(0.5,0.5,0);
	glRotatef(angle,0,0,1);
	glRectf(-1.0/3,-1.0/3,1.0/3,1.0/3);

	glColor3f(0,1,0);	// green
	glLoadIdentity();
	glTranslatef(-0.5,0.5,0);
	glRotatef(angle,0,0,1);
	glRectf(-1.0/3,-1.0/3,1.0/3,1.0/3);

	glColor3f(0,0,1);	// blue
	glLoadIdentity();
	glTranslatef(-0.5,-0.5,0);
	glRotatef(angle,0,0,1);
	glRectf(-1.0/3,-1.0/3,1.0/3,1.0/3);

	glColor3f(1,1,0);	// yellow
	glLoadIdentity();
	glTranslatef(0.5,-0.5,0);
	glRotatef(angle,0,0,1);
	glRectf(-1.0/3,-1.0/3,1.0/3,1.0/3);

} // end function display
