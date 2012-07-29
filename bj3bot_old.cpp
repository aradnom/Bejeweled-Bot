#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace std;

int main () {
	// Opening vars
	int gameboard[8][8], width, height;
	HWND bjWindow;
	HDC bj3DC, bj3DCcompat;
	HBITMAP bj3bitmap;
	RECT window;
	BITMAPINFO bmi;
	BITMAP capture;

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

	bj3DC = GetDC( bjWindow );

	if ( bj3DC != NULL ) 
		cout << "Device context captured" << "\n";	

	bj3DCcompat = CreateCompatibleDC( bj3DC );
	if ( bj3DC != NULL )
		cout << "Compatible DC created" << "\n";

	bj3bitmap = CreateCompatibleBitmap( bj3DC, width, height );
	if ( bj3bitmap != NULL )
		cout << "Compatible bitmap created" << "\n";

	// Put bitmap into device context
	SelectObject( bj3DCcompat, bj3bitmap );

	if ( BitBlt( bj3DCcompat, 0, 0, width, height, bj3DC, 0, 0, SRCCOPY ) != NULL )
		cout << "Block transfer complete" << "\n";

	GetObject( bj3bitmap, sizeof(BITMAP), &capture );

	// Fill BMI to pass to GetDIBits
	bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	DWORD dwBmpSize = ( (capture.bmWidth * bmi.bmiHeader.biBitCount + 31) / 32) * 4 * capture.bmHeight;

	HANDLE hDIB = GlobalAlloc( GHND, dwBmpSize );

	char * lpbitmap = (char *)GlobalLock( hDIB );

	// Buffer
	//BYTE * lpPixels = new BYTE[];

	// Get bits from bitmap
	if ( GetDIBits( bj3DC, bj3bitmap, 0, height, lpbitmap, &bmi, DIB_RGB_COLORS ) != NULL ) {
		cout << "Bits successfully captured" << "\n";
	}

	//int offset = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	//int x = 100, y = 100;

	//int scanLength = (width * (bmi.bmiHeader.biBitCount/8) + 3) / 4 * 4;
	//int firstLineOffset = (height - 1) * scanLength;
	//int pixelOffset = firstLineOffset - y * scanLength + x * (bmi.bmiHeader.biBitCount/8);

	

	// Temporary file capture /////////////////////////////////////////////////
	/*
	BITMAPFILEHEADER bmfHeader;
	// A file is created, this is where we will save the screen capture.
    HANDLE hFile = CreateFile(L"captureqwsx.bmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);   
    
    // Add the size of the headers to the size of the bitmap to get the total file size
    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
 
    //Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER); 
    
    //Size of the file
    bmfHeader.bfSize = dwSizeofDIB; 
    
    //bfType must always be BM for Bitmaps
    bmfHeader.bfType = 0x4D42; //BM   
 
    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
	
	//Close the handle for the file that was created
    CloseHandle(hFile);
    */

    //Unlock and Free the DIB from the heap
    GlobalUnlock(hDIB);    
    GlobalFree(hDIB);	

	//Clean up
    DeleteObject( bj3bitmap );
    DeleteObject( bj3DCcompat );
    ReleaseDC( bjWindow, bj3DC );	

	// End of main ////////////////////////////////////////////////////////////
	cin.get();
	return NULL;
}
