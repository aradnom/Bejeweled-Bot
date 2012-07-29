#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace std;

// Bot constants
#define AMOUNT_GEMS = 8;
#define WINDOW_TITLE = "Bejeweled 3";

#define CORNERCOLOR_UL_R = 83;
#define CORNERCOLOR_UL_G = 30;
#define CORNERCOLOR_UL_B = 13;

#define CORNERCOLOR_LR_R = 181;
#define CORNERCOLOR_LR_G = 134;
#define CORNERCOLOR_LR_B = 120;

enum gemColors {RED, ORANGE, PINK, GREEN, WHITE, YELLOW, BLUE};


// Function declarations
bool getCorners( int &ULcornerX, int &ULcornerY, int &LRcornerX, int &LRcornerY );

int main () {
	// Opening vars
	int width, height, R, G, B;
	int ULcornerX, ULcornerY, LRcornerX, LRcornerY;
	HWND bjWindow;
	RECT window;
	BYTE * pixelPointer;

	// Find window before starting any processing
	do {
		bjWindow = FindWindow( NULL, TEXT("Bejeweled 3") );
		Sleep( 100 );
		cout << "Not found." << "\n";
	} while ( bjWindow == NULL );

	cout << "Found it!" << "\n";

	// Set up window
	
	GetWindowRect( bjWindow, &window );
	width = window.right - window.left;
	height = window.bottom - window.top;
	cout << "Top: " << window.top << " Right: " << window.right << " Bottom: " << window.bottom << " Left: " << window.left;

	// Capture screen
	HDC screen = GetDC( bjWindow );
	HDC HDCCom = CreateCompatibleDC( screen );
	HBITMAP HBitmap = CreateCompatibleBitmap( screen, width, height );
	SelectObject( HDCCom, HBitmap );
	BitBlt( HDCCom, 0, 0, width, height, screen, 0, 0, SRCCOPY );

	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof( bmi.bmiHeader );
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = width * height * 4;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	HBITMAP HBitmapSection = CreateDIBSection( HDCCom, &bmi, DIB_RGB_COLORS, (void**)&pixelPointer, NULL, NULL );
	SelectObject( HDCCom, HBitmapSection );
	BitBlt( HDCCom, 0, 0, width, height, screen, 0, 0, SRCCOPY );

	/*for ( int i = 0; i < width * 4; i += 4 ) {
		cout << (int)pixelPointer[i];
		cout << (int)pixelPointer[i+1];
		cout << (int)pixelPointer[i+2];
	}*/


	for ( int i = 0; i < width; ++i ) {
		for ( int j = 0; j < height; ++j ) {
			COLORREF col = GetPixel(HDCCom, i, j);
			R = (int) GetRValue(col);
			G = (int) GetGValue(col);
			B = (int) GetBValue(col);
		}
	}

	cout << "done";

	// End of main ////////////////////////////////////////////////////////////
	cin.get();
	return NULL;
}
