#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <tchar.h>

using namespace std;

// Bot constants
enum gemColors { RED, ORANGE, PINK, GREEN, WHITE, YELLOW, BLUE };

enum mode { REGULAR, LIGHTNING, ICE };

#define AMOUNT_GEMS			8

// Upper-left and lower-right corners for Zen and Classic modes
#define UL_R_REGULAR		83
#define UL_G_REGULAR		30
#define UL_B_REGULAR		13

#define LR_R_REGULAR		146
#define LR_G_REGULAR		77
#define LR_B_REGULAR		113

// Upper-left and lower-right corners for Lightning modes
#define UL_R_LIGHTNING		60
#define UL_G_LIGHTNING		17
#define UL_B_LIGHTNING		4

#define LR_R_LIGHTNING		233
#define LR_G_LIGHTNING		157
#define LR_B_LIGHTNING		98

#define SCREEN_WIDTH		GetSystemMetrics( SM_CXSCREEN )
#define SCREEN_HEIGHT		GetSystemMetrics( SM_CYSCREEN )

#define BEZEL_HEIGHT		30
#define BEZEL_WIDTH			8

#define VERBOSE				1
#define SHOW_GEMS_LOCATIONS	NULL
#define SHOW_CORNERS		NULL
#define SHOW_GAMEBOARD		NULL
#define FLIP_ALL			NULL
#define FLIP_ALL_DELAY		0
#define FLIP_DELAY			100
#define SEARCH_DELAY		100

#define MOVES				100

// Struct for a single point
struct Point {
	int x, y, color;
	Point () : x(0), y(0), color(0) {}
};

// Function declarations
HDC getBitmap( HWND window, int width, int height );
bool getCorners( Point &cornerUL, Point &cornerLR, int width, int height, HDC &HDCCom, int mode );
bool readPixels( int gameboard[][AMOUNT_GEMS], Point cornerUL, int gemSpacing, HDC HDCCom, HWND window, int width, int height );
bool compareGameboard( int gameboard[][AMOUNT_GEMS], int oldBoard[][AMOUNT_GEMS] );
int convertColor( int R, int G, int B );
int colorBin ( int colors[] );
void displayGameboard( int gameboard[][AMOUNT_GEMS] );
bool moveGem( int gameboard[][AMOUNT_GEMS], Point gemUL, int gemSpacing, int leftSpacing, int topSpacing );
bool colorMatch( int gem1color, int gem2color, int gem3color );
bool switchGems( Point gem1, Point gem2, Point gemUL, int gemSpacing, int leftSpacing, int topSpacing, int delay = 150 );
bool flipAll ( int gameboard[][AMOUNT_GEMS], Point gemUL, int gemSpacing, int leftSpacing, int topSpacing );
void showGemLocations ( int gameboard[][AMOUNT_GEMS], Point cornerUL, int gemSpacing, int leftSpacing, int topSpacing );

// Mouse functions
void leftClick( int x, int y );
void rightClick( int x, int y );
void moveMouse( int x, int y );

// Window interrupt function
LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam );

// Beginning of main //////////////////////////////////////////////////////////
int main () {
	// Opening vars
	int width, height, gemSpacing, leftSpacing, topSpacing, noMoves = 0, moves = 0, mode;
	HWND bjWindow; // Desktop needed to get total screen size
	RECT window;
	int gameboard[AMOUNT_GEMS][AMOUNT_GEMS], oldBoard[AMOUNT_GEMS][AMOUNT_GEMS];
	bool switched;	

	// Read in game mode
	cout << "Enter game mode:" << "\n\n";
	cout << "1. Classic or Zen" << "\n";
	cout << "2. Lightning" << "\n";
	cout << "3. Ice mode" << "\n\n";
	cout << "Enter mode: ";
	cin >> mode;

	switch ( mode ) {
	case 1:
		mode = REGULAR;
		break;
	case 2:
		mode = LIGHTNING;
		break;
	case 3:
		mode = ICE;
		break;
	}

	// Opening points for gameboard corners
	Point cornerUL, cornerLR, cornerUR, cornerLL;

	// Gems in UL, UR, LL positions, used to find other gems
	Point gemUL, gemUR, gemLL;

	// Find window before starting any processing
	do {
		bjWindow = FindWindow( NULL, TEXT("Bejeweled 3") );
		Sleep( 100 );
		cout << "Not found." << "\n";
	} while ( bjWindow == NULL );

	if ( VERBOSE )
		cout << "Found it!" << "\n";

	// Set up window
	
	GetWindowRect( bjWindow, &window );
	width = window.right - window.left;
	height = window.bottom - window.top;
	if ( VERBOSE )
		cout << "Top: " << window.top << " Right: " << window.right << " Bottom: " << window.bottom << " Left: " << window.left << "\n";

	// Save left and top coordinates for spacing so mouse coordinates work correctly
	leftSpacing = window.left;
	topSpacing = window.top;

	// Capture screen

	HDC HDCCom = getBitmap( bjWindow, width, height );

	// Find upper-left and lower-right corners

	if ( getCorners(cornerUL, cornerLR, width, height, HDCCom, mode) ) {
		if ( SHOW_CORNERS ) {
			cout << "UL found at: " << cornerUL.x << ", " << cornerUL.y << " LR found at: " << cornerLR.x << ", " << cornerLR.y << "\n";
			moveMouse( cornerUL.x + leftSpacing, cornerUL.y + topSpacing + BEZEL_HEIGHT );
			Sleep( 1500 );
			moveMouse( cornerLR.x + leftSpacing, cornerLR.y + topSpacing + BEZEL_WIDTH );
			Sleep( 1500 );

			cout << "Done displaying corners..." << "\n";

			cin.get();
			cin.ignore();
		}

		// Set other corners
		cornerUR.x = cornerLR.x;
		cornerUR.y = cornerUL.y;

		cornerLL.x = cornerUL.x;
		cornerLL.y = cornerLR.y;

		// Set gem distance
		gemSpacing = ( cornerUR.x - cornerUL.x ) / AMOUNT_GEMS;

		// Set standard gem middles
		gemUL.x = cornerUL.x + (gemSpacing / 2);
		gemUL.y = cornerUL.y + (gemSpacing / 2);

		gemUR.x = cornerUR.x - (gemSpacing / 2);
		gemUR.y = cornerUL.y + (gemSpacing / 2);

		gemLL.x = cornerLL.x + (gemSpacing / 2);
		gemLL.y = cornerLL.y - (gemSpacing / 2);
	} else {
		cout << "Couldn't find gameboard corners.  Sorry.";
		cin.get();
		return NULL;
	}

	// Display opening color values for calibration
	COLORREF col = GetPixel( HDCCom, gemUL.x, gemUL.y );
	if ( VERBOSE )
		cout << "UL color: " << (int)GetRValue(col) << ", " << (int)GetGValue(col) << ", " << (int)GetBValue(col) << "\n";

	col = GetPixel( HDCCom, gemUR.x, gemUR.y );
	if ( VERBOSE )
		cout << "UR color: " << (int)GetRValue(col) << ", " << (int)GetGValue(col) << ", " << (int)GetBValue(col) << "\n";

	col = GetPixel( HDCCom, gemLL.x, gemLL.y );
	if ( VERBOSE )
		cout << "LL color: " << (int)GetRValue(col) << ", " << (int)GetGValue(col) << ", " << (int)GetBValue(col) << "\n";

	if ( SHOW_GEMS_LOCATIONS ) {
		showGemLocations( gameboard, cornerUL, gemSpacing, leftSpacing, topSpacing );

		cout << "Done outputting gem locations" << "\n";
		cin.get();
		cin.ignore();
	}	

	// With corners set, set up interrupt and move into main loop

	if ( VERBOSE )
		cout << "Setting up keyboard hook..." << "\n";

	HINSTANCE appInstance = GetModuleHandle( NULL );
	SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc, appInstance, 0 );
	MSG msg;

	// Main loop /////////////////////////////////////////////////////////////////	

	while ( true ) {
		//TranslateMessage( &msg );
		//DispatchMessage( &msg );

		switched = false; // Assume no gems have been switched

		// Capture screen and read in gem values
		HDC HDCCom = getBitmap( bjWindow, width, height );

		readPixels( gameboard, cornerUL, gemSpacing, HDCCom, bjWindow, width, height );

		if ( FLIP_ALL ) {
			flipAll( gameboard, gemUL, gemSpacing, leftSpacing, topSpacing );

			cin.get();
			cin.ignore();
		}

		// Compare gameboards, if they're the same and increment noMoves because nothing changed
		if ( compareGameboard(gameboard, oldBoard) )
			noMoves++;

		// Show gameboard before moving gems
		if ( SHOW_GAMEBOARD ) {
			displayGameboard( gameboard );

			cin.get();
			cin.ignore();
		}

		// Look for gem combinations and move accordingly
		if ( moveGem( gameboard, gemUL, gemSpacing, leftSpacing, topSpacing ) ) {
			if ( VERBOSE )
				cout << "Gem switched!" << "\n";
		} else {
			cout << "No move found this round." << "\n";
			noMoves++;

			if ( noMoves > 15 ) {			
				//flipAll( gameboard, gemUL, gemSpacing, leftSpacing, topSpacing );
				noMoves = 0;
			}
		}

		// If there hasn't been any moves, flip everything to reset the board
		if ( noMoves > 15 ) {			
			//flipAll( gameboard, gemUL, gemSpacing, leftSpacing, topSpacing );
			noMoves = 0;
		}

		// Increment moves so it can be checked if you want to continue
		moves++;

		if ( moves > MOVES ) {
			cout << "\n\n" << "Continue?" << "\n\n";
			cin.get();
			cin.ignore();
			moves = 0;

			// Allow window to be activated before continuing because of autopause
			if ( mode == LIGHTNING ) {
				cout << "Pausing while window is made active...";
				Sleep( 1000 );
			}
		}
	}

	cout << "Done";

	// End of main ////////////////////////////////////////////////////////////
	cin.get();
	return 0;
}

// Function implementations ///////////////////////////////////////////////////

HDC getBitmap ( HWND window, int width, int height ) {
	HDC screen = GetDC( window );
	HDC HDCCom = CreateCompatibleDC( screen );
	HBITMAP HBitmap = CreateCompatibleBitmap( screen, width, height );
	SelectObject( HDCCom, HBitmap );
	BitBlt( HDCCom, 0, 0, width, height, screen, 0, 0, SRCCOPY );

	return HDCCom;
}

bool getCorners ( Point &cornerUL, Point &cornerLR, int width, int height, HDC &HDCCom, int mode ) {
	cornerUL.x = 356 + BEZEL_WIDTH;
	cornerUL.y = 50;

	cornerLR.x = 1050 + BEZEL_WIDTH;
	cornerLR.y = 763;

	if ( mode == ICE ) {
		cornerUL.y = 77;
		cornerLR.y = 790;
	}

	if ( mode == LIGHTNING ) {
		cornerUL.y = 87;
		cornerLR.y = 800;
	}

	return true;

	/*	All this used to calculate dynamic spacing, but static spacing is better as long
		as the window size doesn't change
	int UL_R, UL_G, UL_B, LR_R, LR_G, LR_B;

	// Set up correct colors for each mode
	if ( mode == REGULAR ) {
		UL_R = UL_R_REGULAR;
		UL_G = UL_G_REGULAR;
		UL_B = UL_B_REGULAR;

		LR_R = LR_R_REGULAR;
		LR_G = LR_G_REGULAR;
		LR_B = LR_B_REGULAR;
	} else if ( mode == LIGHTNING ) {
		UL_R = UL_R_LIGHTNING;
		UL_G = UL_G_LIGHTNING;
		UL_B = UL_B_LIGHTNING;

		LR_R = LR_R_LIGHTNING;
		LR_G = LR_G_LIGHTNING;
		LR_B = LR_B_LIGHTNING;
	}

	for ( int i = 0; i < width; ++i ) {
		for ( int j = 0; j < height; ++j ) {
			COLORREF col = GetPixel( HDCCom, i, j );
			//cout << (int)GetRValue(col);
			//cout << (int)GetGValue(col);
			//cout << (int)GetBValue(col);

			if ( (int)GetRValue(col) == UL_R && (int)GetGValue(col) == UL_G && (int)GetBValue(col) == UL_B ) {
				cornerUL.x = i;
				cornerUL.y = j;
				if ( VERBOSE )
					cout << "Found UL corner" << "\n";
			}

			if ( (int)GetRValue(col) == LR_R && (int)GetGValue(col) == LR_G && (int)GetBValue(col) == LR_B ) {
				cornerLR.x = i;
				cornerLR.y = j;
				if ( mode == REGULAR ) {
					cornerLR.x = (i - 7);
					cornerLR.y = (j - 7);
				}

				if ( VERBOSE )
					cout << "Found LR corner" << "\n";
				break;
			}
		}
	}

	if ( cornerUL.x != 0 && cornerLR.x != 0 ) 
		return true;

	return false;
	*/
}

bool readPixels ( int gameboard[][AMOUNT_GEMS], Point cornerUL, int gemSpacing, HDC HDCCom, HWND window, int width, int height ) {
	//int R, G, B;
	int x, y, zeroElement = 0;
	//int colorArray[5];
	COLORREF color;

	for ( int i = 0; i < AMOUNT_GEMS; i++ ) {
		for ( int j = 0; j < AMOUNT_GEMS; j++ ) {
			// Set base coordinates, then take 5 samples at each point to figure out what color is
			//x = (cornerUL.x + (gemSpacing / 2) + ((gemSpacing + 1) * j)) + leftSpacing - 2;
			//y = ((cornerUL.y + 15) + (gemSpacing / 2) + ((gemSpacing + 1) * i)) + topSpacing + 27;

			x = (cornerUL.x + (gemSpacing / 2) + ((gemSpacing + 1) * j)) - 12;
			y = ((cornerUL.y + 15) + (gemSpacing / 2) + ((gemSpacing + 1) * i)) - 3;

			color = GetPixel( HDCCom, x, y );
			gameboard[i][j] = convertColor( (int)GetRValue( color ), (int)GetGValue( color ), (int)GetBValue( color ) );

			//color = GetPixel( HDCCom, x, y );
			//colorArray[0] = convertColor( (int)GetRValue( color ), (int)GetGValue( color ), (int)GetBValue( color ) );

			//color = GetPixel( HDCCom, x, y + 7 );
			//colorArray[1] = convertColor( (int)GetRValue( color ), (int)GetGValue( color ), (int)GetBValue( color ) );

			//color = GetPixel( HDCCom, x + 7, y );
			//colorArray[2] = convertColor( (int)GetRValue( color ), (int)GetGValue( color ), (int)GetBValue( color ) );

			//color = GetPixel( HDCCom, x, y - 7 );
			//colorArray[3] = convertColor( (int)GetRValue( color ), (int)GetGValue( color ), (int)GetBValue( color ) );

			//color = GetPixel( HDCCom, x - 7, y );
			//colorArray[4] = convertColor( (int)GetRValue( color ), (int)GetGValue( color ), (int)GetBValue( color ) );

			//gameboard[i][j] = colorBin( colorArray );
			//cout << R << ", " << G << ", " << B << " ";
		}
		//cout << "\n";
	}

	// Check for a null array or array with too many default cases
	for ( int i = 0; i < AMOUNT_GEMS; i++ ) {
		for ( int j = 0; j < AMOUNT_GEMS; j++ ) {
			if ( gameboard[i][j] == 0 )
				zeroElement++;
		}
	}

	// Process zero elements
	if ( zeroElement > 20 ) {
		cout << "Invalid board" << "\n";
		Sleep( 200 );
		HDCCom = getBitmap( window, width, height );
		readPixels( gameboard, cornerUL, gemSpacing, HDCCom, window, width, height );
	}

	return true;
}

bool compareGameboard ( int gameboard[][AMOUNT_GEMS], int oldBoard[][AMOUNT_GEMS] ) {
	bool same = false;

	// Compare existing gameboards before copying
	for ( int i = 0; i < AMOUNT_GEMS; i++ ) {
		for ( int j = 0; j < AMOUNT_GEMS; j++ ) {
			if ( gameboard[i][j] != oldBoard[i][j] )
				same = false;
		}
	}

	same = true;

	// After comparing, copy gameboards for next runthrough
	for ( int i = 0; i < AMOUNT_GEMS; i++ ) {
		for ( int j = 0; j < AMOUNT_GEMS; j++ ) {
			oldBoard[i][j] = gameboard[i][j];
		}
	}

	return same;
}

int convertColor ( int R, int G, int B ) {
	if ( R > 200 && G < 50 && B < 100 ) {
		return RED;
	}
	
	if ( R > 200 && G > 200 && B < 60 ) {
		return YELLOW;
	}
	
	if ( R > 200 && G < 130 && B < 50 ) {
		return ORANGE;
	}
	
	if ( R > 200 && G < 40 && B > 200 ) {
		return PINK;
	}
	
	if ( R < 130 && G > 200 && B < 200 ) {
		return GREEN;
	}
	
	if ( R > 160 && G > 160 && B > 160 ) {
		return WHITE;
	}
	
	if ( R < 60 && G < 230 && B > 200 ) {
		return BLUE;
	}

	return 0;
}

int colorBin ( int colors[] ) {
	int reds = 0, oranges = 0, pinks = 0, greens = 0, whites = 0, yellows = 0, blues = 0;

	for ( int i = 0; i < 5; i++ ) {
		if ( colors[i] == RED )
			reds++;
		if ( colors[i] == ORANGE )
			oranges++;
		if ( colors[i] == PINK )
			pinks++;
		if ( colors[i] == GREEN )
			greens++;
		if ( colors[i] == WHITE )
			whites++;
		if ( colors[i] == YELLOW )
			yellows++;
		if ( colors[i] == BLUE )
			blues++;
	}

	if ( reds >= oranges && reds >= pinks && reds >= greens && reds >= whites && reds >= yellows && reds >= blues )
		return RED;

	if ( oranges >= reds && oranges >= pinks && oranges >= greens && oranges >= whites && oranges >= yellows && oranges >= blues )
		return ORANGE;

	if ( pinks >= oranges && pinks >= reds && pinks >= greens && pinks >= whites && pinks >= yellows && pinks >= blues )
		return PINK;

	if ( greens >= oranges && greens >= pinks && greens >= reds && greens >= whites && greens >= yellows && greens >= blues )
		return GREEN;

	if ( whites >= oranges && whites >= pinks && whites >= greens && whites >= reds && whites >= yellows && whites >= blues )
		return WHITE;

	if ( yellows >= oranges && yellows >= pinks && yellows >= greens && yellows >= whites && yellows >= reds && yellows >= blues )
		return YELLOW;

	if ( blues >= oranges && blues >= pinks && blues >= greens && blues >= whites && blues >= yellows && blues >= reds )
		return BLUE;

	return 0;
}

void displayGameboard( int gameboard[][AMOUNT_GEMS] ) {
	for ( int i = 0; i < AMOUNT_GEMS; i++ ) {
		for ( int j = 0; j < AMOUNT_GEMS; j++ ) {
			cout << gameboard[i][j];
		}
		cout << "\n";
	}
}

bool moveGem ( int gameboard[][AMOUNT_GEMS], Point gemUL, int gemSpacing, int leftSpacing, int topSpacing ) {
	Point gem1, gem2;
	int gemColor1, gemColor2, gemColor3, gemColor4, gemColor5, gemColor6;
	bool switched = false;

	// Beginning of gem-checking conditions ///////////////////////////////////
	
	if ( switched == false ) {
		for ( int i = (AMOUNT_GEMS - 1); i >= 0; i-- ) {
			for ( int j = (AMOUNT_GEMS - 4); j >= 0; j-- ) {
				gemColor1 = gameboard[i][j];
				gemColor2 = gameboard[i][j + 1];
				gemColor3 = gameboard[i][j + 2];
				gemColor4 = gameboard[i][j + 3];
				if ( VERBOSE )
					cout << "Colors: " << gemColor1 << gemColor2 << gemColor3 << gemColor4 << "\n";
				
				// XXOX block 1 - confirmed
				if ( colorMatch( gemColor1, gemColor2, gemColor4 ) ) {
					if ( VERBOSE )
						cout << "Match found - block 1." << "\n";

					gem1.x = j + 2;
					gem1.y = i;
					gem2.x = j + 3;
					gem2.y = i;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
				
				// XOXX block 2 - confirmed
				if ( colorMatch( gemColor1, gemColor3, gemColor4 ) ) {
					if ( VERBOSE )
						cout << "Match found - block 2." << "\n";

					gem1.x = j;
					gem1.y = i;
					gem2.x = j + 1;
					gem2.y = i;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
			}
		}
	} 	
	
	if ( switched == false ) {
		for ( int i = (AMOUNT_GEMS - 4); i >= 0 ; i-- ) {
			for ( int j = (AMOUNT_GEMS - 1); j >= 0; j-- ) {   
				gemColor1 = gameboard[i][j];
				gemColor2 = gameboard[i + 1][j];
				gemColor3 = gameboard[i + 2][j];
				gemColor4 = gameboard[i + 3][j];
				if ( VERBOSE )
					cout << "Colors: " << gemColor1 << gemColor2 << gemColor3 << gemColor4 << "\n";
				
				// X
				// X
				// O
				// X block 3 - confirmed
				
				if ( colorMatch(gemColor1, gemColor2, gemColor4) ) {
					if ( VERBOSE )
						cout << "Match found - block 3." << "\n";

					gem1.x = j;
					gem1.y = i + 2;
					gem2.x = j;
					gem2.y = i + 3;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 

				// X
				// O
				// X
				// X block 4 - confirmed  
				if ( colorMatch(gemColor1, gemColor3, gemColor4)) { 
					if ( VERBOSE )
						cout << "Match found - block 4." << "\n";

					gem1.x = j;
					gem1.y = i;
					gem2.x = j;
					gem2.y = i + 1;    
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
			}
		}
	}
	
	if ( switched == false ) {
		for ( int i = (AMOUNT_GEMS - 2); i >= 0; i-- ) {
			for ( int j = (AMOUNT_GEMS - 3); j >= 0; j-- ) {   
				gemColor1 = gameboard[i][j];
				gemColor2 = gameboard[i][j + 1];
				gemColor3 = gameboard[i][j + 2];
				gemColor4 = gameboard[i + 1][j];
				gemColor5 = gameboard[i + 1][j + 1];
				gemColor6 = gameboard[i + 1][j + 2];
				if ( VERBOSE )
					cout << "Colors: " << gemColor1 << gemColor2 << gemColor3 << "\n\t" << gemColor4 << gemColor5 << gemColor6 << "\n";

				// XXO or // OOX
				// OOX    // XXO block 5 - confirmed
				if ( colorMatch(gemColor1, gemColor2, gemColor6) || colorMatch(gemColor3, gemColor4, gemColor5) ) {
					if ( VERBOSE )
						cout << "Match found - block 5." << "\n";

					gem1.x = j + 2;
					gem1.y = i;
					gem2.x = j + 2;
					gem2.y = i + 1;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
 
				// XOX or // OXO
				// OXO    // XOX block 6 - confirmed
				if ( colorMatch(gemColor1, gemColor3, gemColor5) || colorMatch(gemColor2, gemColor4, gemColor6) ) {
					if ( VERBOSE )
						cout << "Match found - block 6." << "\n";

					gem1.x = j + 1;
					gem1.y = i;
					gem2.x = j + 1;
					gem2.y = i + 1;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
    
				// OXX or // XOO
				// XOO    // OXX block 7 - confirmed
				if ( colorMatch(gemColor2, gemColor3, gemColor4) || colorMatch(gemColor1, gemColor5, gemColor6) ) {
					if ( VERBOSE )
						cout << "Match found - block 7." << "\n";

					gem1.x = j;
					gem1.y = i;
					gem2.x = j;
					gem2.y = i + 1;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				}     
			}
		}
	}
	
	if ( switched == false ) {
		for ( int i = (AMOUNT_GEMS - 3); i >= 0; i-- ) {
			for ( int j = (AMOUNT_GEMS - 2); j >= 0; j-- ) {   
				gemColor1 = gameboard[i][j];
				gemColor2 = gameboard[i][j + 1];
				gemColor3 = gameboard[i + 1][j];
				gemColor4 = gameboard[i + 1][j + 1];
				gemColor5 = gameboard[i + 2][j];
				gemColor6 = gameboard[i + 2][j + 1];
				if ( VERBOSE )
					cout << "Colors: " << gemColor1 << gemColor2 << "\n\t" << gemColor3 << gemColor4 << "\n\t" << gemColor5 << gemColor6 << "\n";

				// XO or // OX
				// XO    // OX
				// OX    // XO block 8 - confirmed 
				if ( colorMatch(gemColor1, gemColor3, gemColor6) || colorMatch(gemColor2, gemColor4, gemColor5) ) {
					if ( VERBOSE )
						cout << "Match found - block 8." << "\n";

					gem1.x = j;
					gem1.y = i + 2;
					gem2.x = j + 1;
					gem2.y = i + 2;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
 
				// XO or // OX
				// OX    // XO
				// XO    // OX block 9 - confirmed   
				if ( colorMatch(gemColor1, gemColor4, gemColor5) || colorMatch(gemColor2, gemColor3, gemColor6) ) {
					if ( VERBOSE )
						cout << "Match found - block 9." << "\n";

					gem1.x = j;
					gem1.y = i + 1;
					gem2.x = j + 1;
					gem2.y = i + 1;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
    
				// OX or // XO
				// XO    // OX
				// XO    // OX block 10 - confirmed
				if ( colorMatch(gemColor2, gemColor3, gemColor5) || colorMatch(gemColor1, gemColor4, gemColor6) ) {
					if ( VERBOSE )
						cout << "Match found - block 10." << "\n";

					gem1.x = j;
					gem1.y = i;
					gem2.x = j + 1;
					gem2.y = i;
					switched = switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_DELAY );
					break;
				} 
			}
		}
	}

	// end of gem-checking conditions /////////////////////////////////////////

	if ( switched == true )
		Sleep( SEARCH_DELAY );

	// Return false if we got all this way without switching anything

	return switched;
}

bool colorMatch ( int gem1color, int gem2color, int gem3color ) {
	if ( gem1color == gem2color && gem2color == gem3color ) {
		return true;
	}

	return false;
}

bool switchGems ( Point gem1, Point gem2, Point gemUL, int gemSpacing, int leftSpacing, int topSpacing, int delay ) {
	leftClick( gemUL.x + (gemSpacing * gem1.x) + leftSpacing + BEZEL_WIDTH, gemUL.y + (gemSpacing * gem1.y) + topSpacing + BEZEL_HEIGHT );
	Sleep( delay );
	leftClick( gemUL.x + (gemSpacing * gem2.x) + leftSpacing + BEZEL_WIDTH, gemUL.y + (gemSpacing * gem2.y) + topSpacing + BEZEL_HEIGHT );
	Sleep( delay );

	return true;
}

bool flipAll ( int gameboard[][AMOUNT_GEMS], Point gemUL, int gemSpacing, int leftSpacing, int topSpacing ) {
	cout << "Flipping out all gems";
	Point gem1, gem2;

	// Flip vertically
	for ( int i = (AMOUNT_GEMS - 2); i >= 0; i-- ) {
		for ( int j = (AMOUNT_GEMS - 1); j >= 0; j-- ) {
			gem1.x = j;
			gem1.y = i;
			gem2.x = j;
			gem2.y = i + 1;

			switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_ALL_DELAY );
		}
	}

	Sleep( FLIP_ALL_DELAY );

	// Flip horizontally
	for ( int i = (AMOUNT_GEMS - 1); i >= 0; i-- ) {
		for ( int j = (AMOUNT_GEMS - 2); j >= 0; j-- ) {
			gem1.x = j;
			gem1.y = i;
			gem2.x = j + 1;
			gem2.y = i;

			switchGems( gem1, gem2, gemUL, gemSpacing, leftSpacing, topSpacing, FLIP_ALL_DELAY );
		}
	}

	return true;
}

void showGemLocations ( int gameboard[][AMOUNT_GEMS], Point cornerUL, int gemSpacing, int leftSpacing, int topSpacing ) {
	int x, y;

	cout << "Displaying gem locations" << "\n";

	for ( int i = 0; i < AMOUNT_GEMS; i++ ) {
		for ( int j = 0; j < AMOUNT_GEMS; j++ ) {
			x = (cornerUL.x + (gemSpacing / 2) + ((gemSpacing + 1) * j)) + leftSpacing - 12 + BEZEL_WIDTH;
			y = ((cornerUL.y + 15) + (gemSpacing / 2) + ((gemSpacing + 1) * i)) + topSpacing - 3 + BEZEL_HEIGHT;

			moveMouse( x, y );
			Sleep( 300 );

			//moveMouse( x, y + 7 );
			//Sleep( 150 );

			//moveMouse( x + 7, y );
			//Sleep( 150 );

			//moveMouse( x, y - 7 );
			//Sleep( 150 );

			//moveMouse( x - 7, y );
			//Sleep( 300 );
		}
	}
}

// Mouse function implementations /////////////////////////////////////////////

void leftClick ( int x, int y ) {
	// Convert coordinates
	float xCoord = ((float)65535 / SCREEN_WIDTH) * x;
	float yCoord = ((float)65535 / SCREEN_HEIGHT) * y;

	if ( VERBOSE )
		cout << "Sending left click at: " << (int)xCoord << ", " << (int)yCoord << "  (" << x << ", " << y << ")" << "\n";

	// Send mouse event
	mouse_event( MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE, (int)xCoord, (int)yCoord, 0, 0 );	
	mouse_event( MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0 );	
	mouse_event( MOUSEEVENTF_LEFTUP, 0, 0, 0, 0 );	

	if ( VERBOSE )
		cout << "Click sent" << "\n";
}

void rightClick ( int x, int y ) {
	// Convert coordinates
	float xCoord = ((float)65535 / SCREEN_WIDTH) * x;
	float yCoord = ((float)65535 / SCREEN_HEIGHT) * y;

	if ( VERBOSE )
		cout << "Sending right click at: " << (int)xCoord << ", " << (int)yCoord << "  (" << x << ", " << y << ")" << "\n";

	// Send mouse event
	mouse_event( MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE, (int)xCoord, (int)yCoord, 0, 0 );	
	mouse_event( MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0 );	
	mouse_event( MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0 );	

	if ( VERBOSE )
		cout << "Click sent" << "\n";
}

void moveMouse ( int x, int y ) {
	// Convert coordinates
	float xCoord = ((float)65535 / SCREEN_WIDTH) * x;
	float yCoord = ((float)65535 / SCREEN_HEIGHT) * y;

	if ( VERBOSE )
		cout << "Moving mouse to: " << (int)xCoord << ", " << (int)yCoord << "  (" << x << ", " << y << ")" << "\n";

	// Send mouse event
	mouse_event( MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE, (int)xCoord, (int)yCoord, 0, 0 );
}

// Console interrupt functionality
LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam ) {
	// Declare our pointer to the KBDLLHOOKSTRUCT
	KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;

	switch ( wParam ) {
	case WM_KEYUP: // When the key has been pressed and released
		{
			switch ( pKeyBoard->vkCode ) { // Check to see what key has been pushed			
			case VK_SPACE: // The return/enter key has been pressed
				DWORD timestamp = pKeyBoard->time; // This shows our timestamp when the key was pushed.
				cout << "Space key pressed.  Continue?" << "\n";
				cin.get();
				cin.ignore();
			break;
			}
		}
	default:
		return CallNextHookEx( NULL, nCode, wParam, lParam );
	}

	return 0;
}