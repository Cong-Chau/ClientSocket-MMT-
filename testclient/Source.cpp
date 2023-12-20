#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <cstring>
#include <conio.h>
#include <Windows.h>
#include <tchar.h>
#include <fstream>
#include <thread>
#include <vector>
//#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
#include "graphics.h"
#pragma comment(lib,"graphics.lib")
using namespace std;
//using namespace cv;
SOCKET Server1, Server2, Server3, Server4, Client1, Client2, Client3, Client4;
void Connection(SOCKET& Server, SOCKET& Client, int port) {
	WSADATA Wsadata;
	WSAStartup(MAKEWORD(2, 2), &Wsadata);
	Server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = port;

	char ServerName[] = "192.168.11.235";
	in_addr addr;
	inet_pton(AF_INET, ServerName, &addr);
	HOSTENT* pHostEnt = gethostbyname(ServerName);
	memcpy(&sin.sin_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
	connect(Server, (struct sockaddr*)&sin, sizeof(sin));
}
void ReceiveImageFile() {
	while (1) {

		int n = 0;
		recv(Server2, (char*)&n, sizeof(int), 0);
		ofstream f("image.jpg", ios::binary); char c[20];
		char buffer[1030];
		for (int i = 0; i < n; i++) {
			recv(Server2, c, sizeof(BYTE), 0);
			f.write(c, sizeof(BYTE));
		}

		f.close();
		cleardevice();
		readimagefile("image.jpg", 0, 0, 1920*2/3,1080*2/3);
		/*Mat img = imread("image.jpg");
		imshow("Share Desktop", img);*/
	}

}
void Mouse() {
	POINT p;
	while (1) {
		Sleep(50);

		if (((GetKeyState(VK_LBUTTON) & 0x8000) != 0)) {

			GetCursorPos(&p);
			ScreenToClient(GetConsoleWindow(), &p);
			int a[3] = { p.x,p.y,0 };
			send(Server3, (char*)&a[0], 3 * sizeof(int), 0);
			cout << p.x << ' ' << p.y << ' ' << a[2] << endl;
		}
		if (((GetKeyState(VK_RBUTTON) & 0x8000) != 0)) {
			POINT p;
			GetCursorPos(&p);

			int a[3] = { p.x,p.y,1 };
			send(Server3, (char*)&a[0], 3 * sizeof(int), 0);
		}
	}
}
void Keyboard() {
	bool Capslock = false; int Win = 0;
	vector <int> a = {8,9,13, 0x13,0x1B };
	for (int i = 0x20; i < 0xDE; i++)
		if (i != 144 && i != 164
			&& i != VK_LMENU && i != VK_RMENU && i != VK_LWIN && i != VK_RWIN
			&& i != VK_LCONTROL && i != VK_RCONTROL && i != VK_LSHIFT && i != VK_RSHIFT)
			a.push_back(i);
	while (1) {
		Sleep(90);
		if (!Capslock && GetKeyState(VK_CAPITAL)) Capslock = 1;
		if (Capslock && !GetKeyState(VK_CAPITAL)) {
			int a[5] = { VK_CAPITAL,0,0,0 ,0 };
			send(Server1, (char*)&a[0], 5 * sizeof(int), 0);
			cout << "send: " << VK_CAPITAL << "\n";
		}
		if (Win == 0 && (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_LWIN))) Win = 1;
		if (!GetAsyncKeyState(VK_LWIN) && !GetAsyncKeyState(VK_LWIN)) {
			if (Win == 1) {
				int a[5] = { VK_RWIN,0,0,0 ,0 };
				send(Server1, (char*)&a[0], 5 * sizeof(int), 0);
				cout << "send: " << VK_RWIN << "\n";
			}
			Win = 0;
		}
		for (int i : a)
			if (GetAsyncKeyState(i)) {
				int a[5] = { i,GetAsyncKeyState(VK_SHIFT),GetAsyncKeyState(VK_MENU),
				GetAsyncKeyState(VK_CONTROL),0 };

				if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_LWIN)) {
					Win = 2; a[4] = 1;
				}
				send(Server1, (char*)&a[0], 5 * sizeof(int), 0);
				cout << "send: " << i << "\n";
			}
	}
}
void Stop() {
	char b = 'b';
	while (1) {
		Sleep(500);
		recv(Server1, &b, sizeof(char), 0);
	} while (1);
}
int main() {
	std::cout << "I am Client!\n"; char b = 'b';
	Connection(Server1, Client1, 3000);
	Connection(Server2, Client2, 3555);
	Connection(Server3, Client3, 4444);
	Connection(Server4, Client4, 5000);
	initwindow(1300,1000);
    thread thr1(Keyboard);
	thread thr2(ReceiveImageFile);
	thread thr3(Mouse);
	thread thr4(Stop);
	thr1.join();
	thr2.join();
	thr3.join();
	thr4.join();
	closegraph();
	closesocket(Server1);
	closesocket(Server2);
	closesocket(Server3);
	closesocket(Server4);
	WSACleanup();

	return 0;
}