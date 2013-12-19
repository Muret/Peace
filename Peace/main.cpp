


#include "window.h"
#include "d11.h"
#include "includes.h"
#include "sph.h"
#include "Camera.h"

extern HINSTANCE g_hinstance;
extern int g_nCmdShow;

void main_render_loop();
string ExePath();

extern int frame_count;
extern int sort_time;

//we all love main.cpp's
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	g_hinstance = hInstance;
	g_nCmdShow = iCmdshow;

	openWindow();

	frame_count = 0;
	int last_frame_count = 0;

	initializeEngine();
	init_sph();
	init();
	string path = ExePath();

	MSG msg = {};
	while (1)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) //Or use an if statement
		{
			switch (msg.message)
			{
			case WM_QUIT:
				exit(0);
				break;
			case WM_KEYDOWN:
				if (msg.wParam == VK_ESCAPE)
					exit(0);
				else
					handle_user_input_down((char)msg.wParam);
				break;
			case WM_KEYUP:
				handle_user_input_up((char)msg.wParam);
				break;
			case WM_TIMER:
				last_frame_count = frame_count;
				frame_count = 0;
				break;
			case WM_RBUTTONDOWN:
				startMovingCamera();
				break;
			case WM_RBUTTONUP:
				stopMovingCamera();
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		tick_user_inputs();

		BeginScene();

		main_render_loop();

		drawText(last_frame_count, 10);
		
		EndScene();

		frame_count++;
	}


	closeEngine();


	return 0;
}


void main_render_loop()
{
	float color[4];
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 1;

	clearScreen(color);
	set_render_constants_buffer();
	render_sph();
}


string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, (LPWSTR)buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}