#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <tchar.h>

using namespace std;

// Window interrupt function
LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam );

int main () {
	HINSTANCE appInstance = GetModuleHandle( NULL );
	SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc, appInstance, 0 );
	MSG msg;
	
	for ( int i = 0; i < 1000; i++ ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		Sleep( 5 );
		cout << "Loop done." << "\n";
	}

	cout << "Done.  Exiting..." << "\n";
	cin.get();
	cin.ignore();

	return 0;
}

// Console interrupt functionality
LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam ) {
	// Declare our pointer to the KBDLLHOOKSTRUCT
	KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;

	if ( wParam == WM_KEYUP ) {
		switch ( pKeyBoard->vkCode ) { // Check to see what key has been pushed			
		case VK_SPACE: // The return/enter key has been pressed
			cout << "Space pressed";
			cin.get();
			cin.ignore();
			break;
		}
	}

	//switch ( wParam ) {
	//case WM_KEYUP: // When the key has been pressed and released
	//	
	//default:
	//	return CallNextHookEx( NULL, nCode, wParam, lParam );
	//}

	return 0;
}