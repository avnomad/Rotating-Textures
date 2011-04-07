#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
using std::clog;
using std::left;

#include <cstdlib>
using std::system;
using std::exit;

#include <GL/glew.h>
#include <GL/glut.h>

#include <windows.h>
#include <tchar.h>

#include "resource.h"

LRESULT CALLBACK soleWindowProcedure(HWND window,UINT message,WPARAM argW,LPARAM argL);

int main()
{
	WNDCLASS soleWindowClass;
	soleWindowClass.style = CS_HREDRAW|CS_VREDRAW;
	soleWindowClass.lpfnWndProc = soleWindowProcedure;
	soleWindowClass.cbClsExtra = 0;
	soleWindowClass.cbWndExtra = 0;
	soleWindowClass.hInstance = GetModuleHandle(nullptr);
	soleWindowClass.hIcon = LoadIcon(GetModuleHandle(nullptr),_T("RotatingTexturesIcon"));	// can be null
	soleWindowClass.hCursor = LoadCursor(nullptr,IDC_ARROW);	// can be null
	soleWindowClass.hbrBackground = nullptr;
	soleWindowClass.lpszMenuName = _T("RotatingTexturesMenu");
	soleWindowClass.lpszClassName = _T("RotatingTexturesClass");
	RegisterClass(&soleWindowClass);

	HWND window = CreateWindow(_T("RotatingTexturesClass"),_T("Rotating Textures"),WS_OVERLAPPEDWINDOW,
									320,120,640,480,nullptr,nullptr,GetModuleHandle(nullptr),nullptr);

	ShowWindow(window,SW_SHOWNORMAL);
	UpdateWindow(window);

	MSG message;
	while(GetMessage(&message,nullptr,0,0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	} // end while

	return message.wParam;
} // end function main


LRESULT CALLBACK soleWindowProcedure(HWND window,UINT message,WPARAM argW,LPARAM argL)
{
	PAINTSTRUCT ps;

	switch(message)
	{
	case WM_PAINT:
		BeginPaint(window,&ps);
		EndPaint(window,&ps);
		return 0;
	case WM_COMMAND:
		if(LOWORD(argW) == IDM_FILE_OPEN)
			MessageBeep(0);
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	} // end switch
	return DefWindowProc(window,message,argW,argL);
} // end function soleWindowProcedure
