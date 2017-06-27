#include <wx/wx.h>
#include <wx/event.h>
#include <wx/busyinfo.h>
#include <wx/dcmemory.h>
#include <wx/printdlg.h>
#include <wx/filename.h>
#include <wx/dcprint.h>
#include <wx/dc.h>
#include <wx/accel.h>
#include <wx/defs.h>
#include <wx/utils.h>
#include <wx/dir.h>
#include <wx/numdlg.h>
#include <wx/stopwatch.h>
#include <wx/timer.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>
#include <string.h>
#include <stdlib.h>
#include <wx/statbox.h>
#include <wx/accel.h>
#include <wx/stattext.h>
#include <iostream>
#include <fstream>
#include <wx/cmdline.h>
#include <wx/choicdlg.h>
#include <wx/ffile.h>
#include <wx/file.h>
#include <wx/print.h>
#include <wx/xml/xml.h>
#include <wx/socket.h>
#include <wx/fontdlg.h>
#include <wx/colordlg.h>
#include <wx/icon.h>


#define		suLINE_THICKNESS			2
#define		suFONT_SIZE					8
#define		TIMER_ID					1
#define		GRID_BUTTON_ID				2
#define		ID_STATUS_BOX				3
#define		ID_PAN_BUTTON_1				4
#define		ID_PAN_BUTTON_2				5
#define		ID_PAN_BUTTON_3				6
#define		ID_PAN_BUTTON_4				7
#define		STATUS_TEXT_ID				8
#define		wxID_SAVE_AS				9
#define		wxID_DIFFICULTY				10
#define		wxID_DISABLE_TIMER			11
#define		wxID_DEFAULT_FONT			12
#define		wxID_GRID_COLOUR			13
#define		wxID_BG_COLOUR				14
#define		wxID_HIGH_SCORES			15
#define		wxID_RULES					16
#define		wxID_DONATE					17
#define		wxID_TUT_MODE				18
#define		wxID_PLAY_MODE				19
#define		wxID_PRINT					20
#define		wxID_SAVE					21
#define		wxID_NEW					22
#define		wxID_HELP					23
#define		wxID_LOAD					24
#define		wxID_PRINT_PREVIEW			25
#define		wxID_DEFAULT_FONT_COLOUR	26
#define		wxID_GB_COLOUR				27
#define		wxID_GB_FONT_COLOUR			28
#define		wxID_GB_TITLE_COLOUR		29
#define		wxID_TITLE_COLOUR			30
#define		wxID_SAVE_SCHEME			31
#define		wxID_RESET_SCHEME			32
#define		wxID_NET_PLAY				33
#define		wxID_NET_MSG				34
#define		wxID_NET_PORT				35
#define		SOCKET_ID					60
#define		SERVER_ID					61
#define		SERVER_START				62
#define		WXPRINT_QUIT				100
#define		WXPRINT_PRINT				101
#define		WXPRINT_PAGE_SETUP			103
#define		WXPRINT_PREVIEW				104
#define		WXPRINT_PRINT_PS			105
#define		WXPRINT_PAGE_SETUP_PS		107
#define		WXPRINT_PREVIEW_PS			108
#define		WXPRINT_ABOUT				109
#define		WXPRINT_ANGLEUP				110
#define		WXPRINT_ANGLEDOWN			111
