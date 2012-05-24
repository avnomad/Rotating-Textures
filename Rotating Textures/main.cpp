//	Copyright (C) 2011-2012 Vaptistis Anogeianakis <el05208@mail.ntua.gr>
/*
 *	This file is part of Rotating Textures.
 *
 *	Rotating Textures is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Rotating Textures is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Rotating Textures.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#include <array>
using std::array;

#include <stdexcept>
using std::runtime_error;

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
#include "bitmap.h"

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

	HWND window = CreateWindow(_T("RotatingTexturesClass"),_T("Rotating Textures"),WS_POPUP|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CAPTION|WS_SYSMENU
									|WS_CLIPSIBLINGS,320,120,640,480,nullptr,nullptr,GetModuleHandle(nullptr),nullptr);

	ShowWindow(window,SW_SHOWNORMAL);
	UpdateWindow(window);

	HMENU sysMenu = GetSystemMenu(window,FALSE);
	AppendMenu(sysMenu,MF_SEPARATOR,0,nullptr);
	AppendMenu(sysMenu,MF_STRING,IDM_SYS_BEEP,_T("&Beep"));

	MSG message;
	while(GetMessage(&message,nullptr,0,0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	} // end while

	return message.wParam;
} // end function main

void display(const array<GLuint,4> &textureIDs);
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
	static array<GLuint,4> textureIDs;
	static array<GLuint,4> memTextureIDs;
	static array<Bitmap,4> images;
	static GLuint currentTexture = 0;
	static const GLuint defaultTextureWidth = 3;
	static const GLuint defaultTextureHeight = 3;
	static GLubyte defaultTextures[4][defaultTextureWidth*defaultTextureHeight][3] = {
			// red-black check
			{{255u,0,0},{   0,0,0},{255u,0,0},
			 {   0,0,0},{255u,0,0},{   0,0,0},
			 {255u,0,0},{   0,0,0},{255u,0,0}},
			// green-black check
			{{0,255u,0},{0,   0,0},{0,255u,0},
			 {0,   0,0},{0,255u,0},{0,   0,0},
			 {0,255u,0},{0,   0,0},{0,255u,0}},
			// blue-black check
			{{0,0,255u},{0,0,   0},{0,0,255u},
			 {0,0,   0},{0,0,255u},{0,0,   0},
			 {0,0,255u},{0,0,   0},{0,0,255u}},
			// yellow-black check
			{{255u,255u,0},{   0,   0,0},{255u,255u,0},
			 {   0,   0,0},{255u,255u,0},{   0,   0,0},
			 {255u,255u,0},{   0,   0,0},{255u,255u,0}},
	};


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

		cout << glGetString(GL_VENDOR) << endl;
		cout << glGetString(GL_VERSION) << endl;

		// OpenGL context initialization
		glClearColor(1,1,0.941f,1);	// ivory
		glEnable(GL_TEXTURE_2D);
		glGenTextures(textureIDs.size(),textureIDs.data());
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		for(size_t i = 0 ; i < textureIDs.size() ; ++i)
		{
			glBindTexture(GL_TEXTURE_2D,textureIDs[i]);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,defaultTextureWidth,defaultTextureHeight,0,GL_RGB,GL_UNSIGNED_BYTE,defaultTextures[i]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		} // end for

		glPixelStorei(GL_UNPACK_ALIGNMENT,4);
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
		display(textureIDs);
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
			try{
				images[currentTexture] = Bitmap(ofn.lpstrFile);
				glBindTexture(GL_TEXTURE_2D,textureIDs[currentTexture]);
				glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,images[currentTexture].width,images[currentTexture].height,0,GL_BGR,GL_UNSIGNED_BYTE,images[currentTexture].data.get());
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				glGenerateMipmap(GL_TEXTURE_2D);
				if(++currentTexture >= textureIDs.size())
					currentTexture = 0;
			}catch(const runtime_error &e){
				MessageBoxA(window,e.what(),"Error",MB_ICONERROR);
			}
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

			cout << glGetString(GL_VENDOR) << endl;
			cout << glGetString(GL_VERSION) << endl;

			// OpenGL context initialization
			glClearColor(1,1,0.941f,1);	// ivory
			glEnable(GL_TEXTURE_2D);
			glGenTextures(memTextureIDs.size(),memTextureIDs.data());
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			
			glPixelStorei(GL_UNPACK_ALIGNMENT,4);
			for(size_t i = 0 ; i < textureIDs.size() ; ++i)
			{
				glBindTexture(GL_TEXTURE_2D,memTextureIDs[i]);
				gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,images[i].width,images[i].height,GL_BGR,GL_UNSIGNED_BYTE,images[i].data.get());
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			} // end for

			glViewport(0,0,r.right,r.bottom);
			display(memTextureIDs);
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
			//HGLRC glPrinterContext;
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

							glClearColor(1,1,0.941f,1);	// ivory
							glViewport(0,0,GetDeviceCaps(gdiPrinterDC,HORZRES),GetDeviceCaps(gdiPrinterDC,VERTRES));
							display(memTextureIDs);
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

void display(const array<GLuint,4> &textureIDs)
{
	static float rectanglePositions[4][2] = {
		{ 0.5f, 0.5f},
		{-0.5f, 0.5f},
		{-0.5f,-0.5f},
		{ 0.5f,-0.5f},
	};

	glClear(GL_COLOR_BUFFER_BIT);

	for(size_t i = 0 ; i < textureIDs.size() ; ++i)
	{
		glBindTexture(GL_TEXTURE_2D,textureIDs[i]);
		glLoadIdentity();
		glTranslatef(rectanglePositions[i][0],rectanglePositions[i][1],0);
		glRotatef(angle,0,0,1);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0,0.0);
			glVertex2f(-1.0f/3,-1.0f/3);
			glTexCoord2f(1.0,0.0);
			glVertex2f(1.0f/3,-1.0f/3);
			glTexCoord2f(1.0,1.0);
			glVertex2f(1.0f/3,1.0f/3);
			glTexCoord2f(0.0,1.0);
			glVertex2f(-1.0f/3,1.0f/3);
		glEnd();
	} // end for
} // end function display
