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

#include <string>
using std::string;
using std::wstring;
using std::getline;

#include <GL/glew.h>
#include <GL/glut.h>

#include <windows.h>
#include <WindowsX.h>
#include <WinSpool.h>
#include <CommDlg.h>
#include <tchar.h>

#ifdef UNICODE
typedef wstring tstring;
#else
typedef string tstring;
#endif

#include "resource.h"

LRESULT CALLBACK soleWindowProcedure(HWND window,UINT message,WPARAM argW,LPARAM argL);

tstring windowTitle = _T("Rotating Textures");

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

	HWND window = CreateWindow(_T("RotatingTexturesClass"),_T("Rotating Textures"),WS_POPUP|WS_THICKFRAME|WS_CLIPCHILDREN
									|WS_CLIPSIBLINGS,320,120,640,480,nullptr,nullptr,GetModuleHandle(nullptr),nullptr);

	ShowWindow(window,SW_SHOWNORMAL);
	UpdateWindow(window);

	HMENU sysMenu = GetSystemMenu(window,FALSE);
	AppendMenu(sysMenu,MF_SEPARATOR,0,nullptr);
	AppendMenu(sysMenu,MF_STRING,IDM_FILE_OPEN,_T("&Open..."));
	AppendMenu(sysMenu,MF_STRING,IDM_SYS_BEEP,_T("&Beep"));

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
	static HDC gdiContext;
	static HGLRC glContext;
	int pixelFormatIndex;
	RECT r;
	HBITMAP gdiBitmap;
	static HDC gdiMemContext;
	static HGLRC glMemContext;
	static OPENFILENAME ofn;
	static const DWORD maxFileNameAndPathSize = 512;
	static TCHAR fileNameAndPath[maxFileNameAndPathSize] = _T("");	// can put the path of a default file to load here.
	static const DWORD maxFileNameSize = 128;		// but shouldn't include hardcoded private paths to public repositories! :)
	static TCHAR fileName[maxFileNameSize];
	static POINT oldMousePosition;
	POINT mousePosition;

	switch(message)
	{
	case WM_CREATE:
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = window;
		ofn.hInstance = nullptr;
		ofn.lpstrFilter = _T("Bitmap files (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0\0");
		ofn.lpstrCustomFilter = nullptr;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = fileNameAndPath;
		ofn.nMaxFile = maxFileNameAndPathSize;
		ofn.lpstrFileTitle = fileName;
		ofn.nMaxFileTitle = maxFileNameSize;
		ofn.lpstrInitialDir = nullptr;
		ofn.lpstrTitle = _T("Open a bitmap file for use as a texture.");
		ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
		ofn.nFileOffset = 0;
		ofn.nFileExtension = 0;
		ofn.lpstrDefExt = _T("bmp");
		ofn.lCustData = 0;
		ofn.lpfnHook = nullptr;
		ofn.lpTemplateName = nullptr;
		ofn.pvReserved = nullptr;
		ofn.dwReserved = 0;
		ofn.FlagsEx = 0;

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

		gdiContext = GetDC(window);
		pixelFormatIndex = ChoosePixelFormat(gdiContext,&pixelFormatDescription);
		SetPixelFormat(gdiContext,pixelFormatIndex,&pixelFormatDescription);
		glContext = wglCreateContext(gdiContext);
		wglMakeCurrent(gdiContext,glContext);
		glewInit();

		glClearColor(1,1,0.941,1);	// ivory
		return 0;
	case WM_SIZE:
		GetClientRect(window,&r);
		glViewport(0,0,r.right,r.bottom);
		return 0;
	case WM_LBUTTONDOWN:
		GetCursorPos(&oldMousePosition);
		SetCapture(window);
		return 0;
	case WM_MOUSEMOVE:
		if(window == GetCapture())
		{
			GetWindowRect(window,&r);
			GetCursorPos(&mousePosition);
			MoveWindow(window,mousePosition.x-oldMousePosition.x+r.left,mousePosition.y-oldMousePosition.y+r.top,r.right-r.left,r.bottom-r.top,FALSE);
			oldMousePosition = mousePosition;
		} // end if
		return 0;
	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	case WM_PAINT:
		display();
		angle += 1.5;
		SwapBuffers(gdiContext);
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
	case WM_KEYDOWN:
		if(argW == VK_ESCAPE)
		{
			SendMessage(window,WM_CLOSE,0,0);
			return 0;
		} // end if
		break;
	case WM_COMMAND:
		switch(LOWORD(argW))
		{
		case IDM_FILE_OPEN:
			GetOpenFileName(&ofn);
			SetWindowText(window,(windowTitle+_T(" - ")+ofn.lpstrFileTitle).c_str());
			return 0;
		case IDM_EDIT_COPY:
			GetClientRect(window,&r);
			gdiBitmap = CreateCompatibleBitmap(gdiContext,r.right,r.bottom);
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

			gdiMemContext = CreateCompatibleDC(gdiContext);
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

			wglMakeCurrent(gdiContext,glContext);
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
					gdiPrinterDC = CreateDC(nullptr,((PRINTER_INFO_4*)printers)[i].pPrinterName,nullptr,nullptr);					
						gdiBitmap = CreateCompatibleBitmap(gdiPrinterDC,GetDeviceCaps(gdiPrinterDC,HORZRES),GetDeviceCaps(gdiPrinterDC,VERTRES));
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

						gdiMemContext = CreateCompatibleDC(gdiPrinterDC);
						gdiBitmap = (HBITMAP)SelectObject(gdiMemContext,gdiBitmap);
						pixelFormatIndex = ChoosePixelFormat(gdiMemContext,&pixelFormatDescription);
						SetPixelFormat(gdiMemContext,pixelFormatIndex,&pixelFormatDescription);

						StartDoc(gdiPrinterDC,&di);
						StartPage(gdiPrinterDC);
							glMemContext = wglCreateContext(gdiMemContext);
							wglMakeCurrent(gdiMemContext,glMemContext);

							glClearColor(1,1,0.941,1);	// ivory
							glViewport(0,0,GetDeviceCaps(gdiPrinterDC,HORZRES),GetDeviceCaps(gdiPrinterDC,VERTRES));
							display();
							glFinish();	// essential!

							BitBlt(gdiPrinterDC,0,0,GetDeviceCaps(gdiPrinterDC,HORZRES),GetDeviceCaps(gdiPrinterDC,VERTRES),gdiMemContext,0,0,SRCCOPY);
						EndPage(gdiPrinterDC);
						EndDoc(gdiPrinterDC);
						gdiBitmap = (HBITMAP)SelectObject(gdiMemContext,gdiBitmap);
					wglMakeCurrent(gdiContext,glContext);
					wglDeleteContext(glMemContext);
					DeleteDC(gdiMemContext);
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
		//ReleaseDC(window,gdiContext);
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
