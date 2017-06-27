#include "sudoku.h"
#include "engine.h"



bool BasicApplication::OnInit()
{
	suCommon::suVersion = wxString(wxT(SUDOKU_VERSION_STRING));
	suCommon::InputFile = wxString(argv[1]);
	suCommon::default_title_text.Printf(wxT("Sudoku Game Solver Generator"));
	MainFrame *frame = new MainFrame(suCommon::default_title_text, wxPoint(50, 50), wxSize(730, 600));
	suCommon::suDC = new wxMemoryDC();
	suCommon::bitmap = wxBitmap(720, 530);
	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////

#include <map>

typedef std::map<std::wstring, std::string> AllAnsiStrings;
static AllAnsiStrings g_allAnsiStrings;

const char* ToAnsi(const wchar_t* sW)
{
	AllAnsiStrings::const_iterator it = g_allAnsiStrings.find(sW);

	if (it != g_allAnsiStrings.end())
		return it->second.c_str();

	size_t srcLen = wcslen(sW);
	char* sA = (char *)_alloca((srcLen + 1) * sizeof(char));

	if (sA == NULL)
		return "NULL";

	for (size_t i = 0; i < (srcLen + 1); ++i)
		sA[i] = char(sW[i]);

	g_allAnsiStrings[sW] = sA;

	return ToAnsi(sW);
}

static wchar_t g_appRootW[MAX_PATH];

bool static fillAppDataPathW(wchar_t szPathW[MAX_PATH], const wchar_t* subFileW)
{
	if (g_appRootW[0] != 0)
	{
		wcscpy(szPathW, g_appRootW);
		size_t ilen = wcslen(szPathW);
		wcscpy(&szPathW[ilen], subFileW);
		return true;
	}

	DWORD nModPathSize = ::GetModuleFileNameW(NULL, szPathW, MAX_PATH);

	if (nModPathSize == 0)
	{
		return false;
	}

	if (nModPathSize >= MAX_PATH)
	{
		return false;
	}

	if (g_appRootW[0] == 0)
	{
		size_t pLast = nModPathSize - 1;
		while (szPathW[pLast] != L'\\')
			--pLast;

		wmemcpy(g_appRootW, szPathW, pLast);
		g_appRootW[pLast] = 0;

		if (g_appRootW[0] == 0)
			abort();
	}

	if (subFileW == NULL)
		return true; // this running executable pathname was requested

	return fillAppDataPathW(szPathW, subFileW);
}

static wxString wxMyGetCfgFilePath()
{
	wchar_t szPathW[MAX_PATH];
	wchar_t* subFileW = L"\\uss.cfg";

	fillAppDataPathW(szPathW, subFileW);

	return wxString(ToAnsi(szPathW));
}

static wxString wxMyGetIconFilePath()
{
	wchar_t szPathW[MAX_PATH];
	wchar_t* subFileW = L"\\favicon.ico";

	fillAppDataPathW(szPathW, subFileW);

	return wxString(ToAnsi(szPathW));
}

////////////////////////////////////////////////////////////////////////////////////////////

MainFrame::MainFrame( const wxString title, wxPoint origin, wxSize size) 
		 :  wxFrame( NULL, wxID_ANY, title, origin, size )
{
	int i;
	suMode = 0;
	suDisabled = false;
	suNetPlay = false;
	timer_enabled = true;
	update_text = true;
	suModeCache = 99;
	ResetAllArrays();
	InitClock();
	DrawMenu();
	OriginRedraw = wxPoint(0, 0);
	InitStatusBar();
	SetStatusTime();
	suConfigFile = wxMyGetCfgFilePath();
	wxFFile file_obj;
	suPort = 207;
	suServerHost = wxString(wxT(""));
	suChatText.Clear();
	// if config file does not exist, create template file
	if (!wxFile::Exists(suConfigFile))
	{
		if (!wxDir::Exists(home_dir))
		{
			wxFileName::Mkdir(home_dir, 0644, wxPATH_MKDIR_FULL);
		}
		file_obj.Open(suConfigFile, wxT("w"));
		file_obj.Write(GenerateConfig());
	}
	file_obj.Close();
	timer_val=0;
	CurrentFileName=wxT("");
	init_run=true;
	// Set up all the default fonts and colours
	wxFont tmp_font(suFONT_SIZE, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL, false);
	suDefaultFont = tmp_font;
	suDefaultFont.SetPointSize(8);
	suDefaultFont.SetFamily(wxFONTFAMILY_SWISS);
	suDefaultFont.SetWeight(wxFONTWEIGHT_NORMAL);
	suDefaultFont.SetStyle(wxFONTSTYLE_NORMAL);
	suDefaultFontColour = wxColour(wxT("BLACK"));
	suGridColour = wxColour(255, 239, 213);
	suGBColour = wxColour(wxT("NAVY"));
	suGBFontColour = wxColour(wxT("RED"));
	suBGColour = wxColour(wxT("WHITE"));
	suTitleColour = wxColour(wxT("NAVY"));
	InitConfigFile();
	difficulties[0] = wxString(wxT("Yellow Belt"));
	difficulties[1] = wxString(wxT("Orange Belt"));
	difficulties[2] = wxString(wxT("Green Belt"));
	difficulties[3] = wxString(wxT("Blue Belt"));
	difficulties[4] = wxString(wxT("Brown Belt"));
	difficulties[5] = wxString(wxT("Black Belt"));
	difficulties[6] = wxString(wxT("Sudoku Game Jedi Master"));
	update_text = true;
	suNetAttempt = false;
	suProcessing = false;
	suGameOver = false;
	suComplete = 0;
	for (i=0; i<60; i++)
		suNetComplete[i] = 0;
	
	wxAcceleratorEntry entries[6];
	entries[0].Set(wxACCEL_CTRL, (int)'t', wxID_NET_MSG);
	entries[1].Set(wxACCEL_CTRL, (int)'s', wxID_SAVE);
	entries[2].Set(wxACCEL_CTRL, (int)'o', wxID_LOAD);
	entries[3].Set(wxACCEL_CTRL, (int)'p', wxID_PRINT);
	entries[4].Set(wxID_DONATE, (int)'d', wxID_PRINT);
	entries[5].Set(wxID_NEW, (int)'n', wxID_NEW);
	wxAcceleratorTable accel(6, entries);
	this->SetAcceleratorTable(accel);
	suConnectionEstablished = false;

	if (wxFile::Exists(suCommon::InputFile))
	{
		LoadFile(suCommon::InputFile);
	}

	if (wxFile::Exists(wxMyGetIconFilePath()))
	{
		wxIcon default_icon(wxMyGetIconFilePath(), wxBITMAP_TYPE_ICO);
		SetIcon(default_icon);
	}

	
	wxDateTime now = wxDateTime::Now();
	epoch_start = (long)now.GetTicks();
}

MainFrame::~MainFrame()
{

}


bool MainFrame::InitConfigFile(void)
{
	wxXmlDocument doc;
	wxXmlNode *level1;
	wxXmlNode *level2;
	wxXmlProperty *level3;
	wxString ContentStr;
	unsigned long int HighScoreVal_c[10], ContentUlong, max;
	wxString HighScoreDate_c[10];
	int offset=0, i, j;
	long ContentInt;
	wxString HighScoreName_c[10];
	double ContentNum;

	if (!doc.Load(suConfigFile))
		return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != wxT("suConfig"))
		return false;

	for (i=0; i<10; i++)
	{
		// Reset all the high score values
		HighScoreVal_c[i] = 0;
		HighScoreDate_c[i] = wxString(wxT(""));
		HighScoreName_c[i] = wxString(wxT(""));
	}

	// Level1 is the highest in the tree = level 2 is a child etc.
	level1 = doc.GetRoot()->GetChildren();
	while (level1) {
		offset = 0;
		// this is the highscores stanza
		if (level1->GetName() == wxT("highscores"))
		{
			level2 = level1->GetChildren();
			while (level2)
			{
				// this is the score tag
				if (level2->GetName() == wxT("score") && offset < 10)
				{
					HighScoreName_c[offset] = level2->GetPropVal(wxT("name"), wxString(wxT("")));
					HighScoreDate_c[offset] = level2->GetPropVal(wxT("date"), wxString(wxT("")));
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("0")));
					ContentStr.ToULong(&ContentUlong);
					HighScoreVal_c[offset] = ContentUlong;
					offset++;
				}
				level2 = level2->GetNext();
			}
		}
		if (level1->GetName() == wxT("default"))
		{
			level2 = level1->GetChildren();
			while (level2)
			{
				if (level2->GetName() == wxT("bg_colour"))
				{
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
						suBGColour.Set(ContentStr);

				}
				if (level2->GetName() == wxT("font_colour"))
				{
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
						suDefaultFontColour.Set(ContentStr);
				}
				if (level2->GetName() == wxT("grid_colour"))
				{
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
						suGridColour.Set(ContentStr);
				}
				if (level2->GetName() == wxT("title_colour"))
				{
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
						suTitleColour.Set(ContentStr);
				}
				if (level2->GetName() == wxT("gb_colour"))
				{
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
						suGBColour.Set(ContentStr);
				}
				if (level2->GetName() == wxT("gb_font_colour"))
				{
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
						suGBFontColour.Set(ContentStr);
				}
				if (level2->GetName() == wxT("netplay_port"))
				{
					ContentStr = level2->GetPropVal(wxT("value"), wxString(wxT("")));
					ContentStr.ToLong(&ContentInt);
					if ((int)ContentInt > 0)
						suPort = (int)ContentInt;
					
				}
				if (level2->GetName() == wxT("font"))
				{
					ContentStr = level2->GetPropVal(wxT("point"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
					{
						ContentNum = 0;
						ContentStr.ToLong(&ContentInt);
						suDefaultFont.SetPointSize((int)ContentInt);
					}
					ContentStr = level2->GetPropVal(wxT("family"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
					{
						ContentStr.ToDouble(&ContentNum);
						suDefaultFont.SetFamily((int)ContentNum);
					}
					ContentStr = level2->GetPropVal(wxT("style"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
					{
						ContentStr.ToDouble(&ContentNum);
						suDefaultFont.SetStyle((int)ContentNum);
					}
					ContentStr = level2->GetPropVal(wxT("weight"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
					{
						ContentStr.ToDouble(&ContentNum);
						suDefaultFont.SetWeight((int)ContentNum);
					}
					ContentStr = level2->GetPropVal(wxT("underlined"), wxString(wxT("")));
					if (ContentStr == wxString(wxT("true")))
						suDefaultFont.SetUnderlined(true);
					else if (ContentStr == wxString(wxT("false")))
						suDefaultFont.SetUnderlined(false);

					ContentStr = level2->GetPropVal(wxT("face"), wxString(wxT("")));
					if (ContentStr != wxString(wxT("")))
						suDefaultFont.SetFaceName(ContentStr);
				}
				level2 = level2->GetNext();
			}
		}
		level1 = level1->GetNext();
	}


	// Scores are all parsed at this stage - now to arrange them in order
	// to go into the non-cache, proper entries:
	for (i=0; i<10; i++)
	{
		offset = 0;
		max = 0;
		// Identify the highest value in the HighScoreVal_c array
		for (j=0; j<10; j++)
		{
			if (HighScoreVal_c[j] >= max)
			{
				max = HighScoreVal_c[j];
				offset = j;
			}
		}

		if (HighScoreVal_c[offset] > 0)
		{
			// Set this value to te highest available scores
			HighScoreVal[i] = HighScoreVal_c[offset];
			HighScoreDate[i] = HighScoreDate_c[offset];
			HighScoreName[i] = HighScoreName_c[offset];
		}
		else
		{
			HighScoreVal[i] = 0;
			HighScoreDate[i] = wxString(wxT(""));
			HighScoreName[i] = wxString(wxT(""));
		}

		// Reset HighScoreVal_c[j] so it gets discounted for next pass
		HighScoreVal_c[offset] = 0;
	}

	return true;
}

void MainFrame::NetPlayMenu(void)
{
	wxString serv_option[2];
	int answer;
	serv_option[0].Printf("Start Netplay as Server");
	serv_option[1].Printf("Start Netplay as Client");
	// Find out if user wants to be a server or a client
	// the client will poll the server every 4 seconds and
	// synchronize like that
	
	answer = wxGetSingleChoiceIndex(
			wxString(wxT("Are you going to act as the server or the client?")),
			wxString(wxT("Server or Client Configuration")),
			2, serv_option, this, -1, -1, true, 150, 200);
	if (answer == -1)
		return; 
	else if (answer == 0)
		StartSudServer();
	else
		StartSudClient();
}

void MainFrame::CheckUpToDate(void)
{
	wxHTTP http;
	wxString new_ver;
	wxString str;
	int answer;

	http.SetHeader ( wxT("Accept") , wxT("text/*") );
	http.SetHeader ( wxT("User-Agent"), wxGetApp().GetAppName() ); 
	// The wxWidgets default timeout is 10 minutes.
	http.SetTimeout ( 2 );

	// Note that Connect() wants a host address, not an URL. 80 is the server's port.
	if ( http.Connect ( "uss.snesreviews.co.uk", 80 ) )
	{
		if ( wxInputStream *stream = http.GetInputStream ( wxT("/update.txt") ) )
		{
			wxString data;
			// Will hold the content of the file (assuming that it's a text file).
			wxStringOutputStream out_stream ( &data );
			// The data will be streamed to the variable "data".
			stream->Read ( out_stream );
			new_ver = data;

			delete stream;
			if(!new_ver.Contains(suCommon::suVersion) && new_ver != wxEmptyString)
			{
				str.Printf("There is an update for Sudoku Game Solver Generator available (v");
				str.append(new_ver);
				str.append(")!\nWould you like to go to where you can download the latest\nversion?");
				answer = wxMessageBox(str, wxT("Sudoku Game Solver Generator update available"), wxYES_NO, this);
				if (answer == wxYES)
				{
					str.Printf(wxT("http://sourceforge.net/project/showfiles.php?group_id=189297"));
					wxLaunchDefaultBrowser(str, 0);
				}
			}
		}
	}
	
}

void MainFrame::OnNetPlay(wxCommandEvent& event)
{
	if(ConfirmBox(wxString("start a new net play session, abandoning all current games and ratings")) != true)
		return;
	
	if (suNetPlay)
		Socket->Destroy();
	if (suNetPlay)
		m_server->Destroy();
	suConnectionEstablished = false;
	suProcessing = false;
	suDisabled = false;
	suGameOver = false;
	suProcessing = false;
	suNetAttempt = false;
	Reset();
	NetPlayMenu();
}

bool MainFrame::InitNetPlay(wxString in_str)
{
	char str_dmp[100];
	int i, answer;
	// A response has been received form the server/client - 
	// confirm the user want's to go ahead with the game,
	// allows the server to select the difficulty and let the
	// game commence by setting suMode=2
	if (suServerMode)
	{
		// If currently running as a server
		SetStatusText(wxString("There has been a response from the other party..."), 0);
		answer = wxMessageBox(
			wxString(wxT("There has been a response from the other party - do you want to accept?")), 
			wxString(wxT("Sudoku Game Solver Generator")), 
			wxYES_NO, 
			this);
		if (answer == wxYES)
		{
			answer = wxGetSingleChoiceIndex(
				wxString(wxT("Choose your destiny, young one...")),
				wxString(wxT("Difficulty Selection")),
				7, difficulties, this, -1, -1, true, 150, 200);
			if (answer >= 0)
			{
				PuzzleDifficulty = answer;
				SetStatusText(wxString("Letting the client know we want to play..."), 0);
				return true;
			}
			else
				return false;
		}
		else
		{
			SetStatusText(wxString("Severing Connection with Client..."), 0);
			return false;
		}
	}
	else
	{
		SetStatusText(wxString("Attempting to connect to server..."), 0);
		for (i=0; i<99; i++)
		{
			str_dmp[i] = in_str.GetChar(i);
			if (str_dmp[i] != 'f')
				return false;
		}
		PuzzleDifficulty = in_str.GetChar(99);
		SetStatusText(wxString("Now connected to server..."), 0);
		// if all values are 0xff
		return true;
	}
}

void MainFrame::ContinueNetPlay(wxString in_str)
{
	suProcessing = true;
	int i, j, answer;
	if (suNetAttempt == false && suServerMode)
	{
		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
			{
				suValues[i][j] = 0;
				suModes[i][j] = 0;
			}

		suDisabled = true;
		GeneratePuzzle(PuzzleDifficulty);
		suDisabled = false;

		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
				if (suValues[i][j] > 0)
					suModes[i][j] = 2;

		suMode = 1;
		Refresh();
		suNetAttempt = true;
	}
	else if (suNetAttempt == true && suServerMode)
	{
		//SetStatusText(in_str, 0);
	}
	else if (suServerMode == false && suNetAttempt == false)
	{
		// Check the string to see if the other side is ready to go
		if (in_str.GetChar(4) == '1')
		{
			suDisabled = true;
			GeneratePuzzle(PuzzleDifficulty);
			suDisabled = false;

			for (i=0; i<9; i++)
				for (j=0; j<9; j++)
					if (suValues[i][j] > 0)
						suModes[i][j] = 2;

			suMode = 1;
			Refresh();
			suNetAttempt = true;
		}
	}

	suProcessing = false;
}


// This comes straight from a prompt by the user to start the server
// Then it waits for a server event
void MainFrame::StartSudServer(void)
{
	wxString str;
	suNetPlay = true;
	suServerMode = true;
	wxIPV4address addr;
	addr.Service(suPort);

	// Create the socket, we maintain a class pointer so we can shut it down
	m_server = new wxSocketServer(addr);

	// We use Ok() here to see if the server is really listening
	if (! m_server->Ok())
	{
		str.Printf("Could not start server - are you sure port %d is not already in use?", suPort);
		SetStatusText(str, 0);
		return;
	}

	// Setup the event handler and subscribe to connection events
	m_server->SetEventHandler(*this, SERVER_ID);
	m_server->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
	m_server->Notify(true);

	SetStatusText(wxString("Server initialised - waiting for someone to connect..."), 0);
}

void MainFrame::ProcessKeyboardShortcut(wxKeyEvent& event)
{
	int key_code;
	// Open up a chat dialogue when ctrl+t is entered
	if ( event.ControlDown() )
	{
		key_code = event.GetKeyCode();
		// Only process ctrl+(x) shortcuts
		switch(key_code)
		{
			case 20:
				if (suNetPlay)
					SendChatString();
				break;
			default:
				DebugInt(key_code);
				break;
		}
	}
	event.Skip(true);
}

void MainFrame::OnNetMsg(wxCommandEvent& event)
{
	SendChatString();
}

void MainFrame::OnNetPort(wxCommandEvent& event)
{
	wxString str;
	int port_no, answer;
	// Menu for editing netplay port
	port_no = (int)wxGetNumberFromUser( wxString("Port Value (0-8000)"), 
		wxString("Editing Default Port"), 
		wxString("Enter your chosen port for netplay - must be the same as \nyour opponent and opened up on your firewall"), (long)suPort, (long)0, (long)8000, this, wxDefaultPosition );
	if (port_no < 0)
		SetStatusText(wxString("Invalid Port Entered - value not saved"), 0);
	else
	{
		if (port_no != suPort)
		{
			str.Printf("You changed the port number, click on yes to make change permanent,\n");
			str.Append("no to only apply the change to this session, or cancel to not apply the\n");
			str.Append("change at all.");
			answer = wxMessageBox(str, 
				wxString("Sudoku Game Solver Generator"), 
				wxYES_NO | wxCANCEL, this);
			if (answer == wxYES)
			{
				// Save the port to the config file
				suPort = port_no;
				WriteNetPort(suPort);
			}
			else if (answer == wxNO)
			{
				suPort = port_no;
				str.Printf("Netplay port now changed to %d", suPort);
				SetStatusText(str, 0);
			}
			else if (answer == wxCANCEL)
			{
				str.Printf("Netplay port change aborted");
				SetStatusText(str, 0);
			}

		}
		else
			SetStatusText(wxString("Netplay Port unchanged"), 0);
	}

}

void MainFrame::SendChatString(void)
{
	if (suNetPlay == false)
		return;

	int i;
	char buff[100], str_buff[90];
	wxString chat_string;
	
	// prompt for a string which can be a maximum of 100 characters long
	suChatText = wxGetTextFromUser(
			_("Enter a chat string you would like to send to the other user - choose your words wisely!"),
			_("Chat to Challenger (maximum 90 characters)"),
			_(""));

	suChatText.Truncate(90);
	
	if (suChatText != wxEmptyString)
		AddNetString(wxString("You: "), suChatText);

}

void MainFrame::StartSudClient(void)
{
	suNetPlay = true;
	suServerMode = false;

	// Ask user for server address

	if (suServerHost == wxString(wxT("")))
	{
		suServerHost = wxGetTextFromUser(
			_("Enter the address of the other sudoku machine you are connecting to (IP address or hostname):"),
			_("Connect ..."),
			_("localhost"));
	}

	wxIPV4address addr;
	addr.Hostname(suServerHost);
	addr.Service(suPort);
	
	// Create the socket
	Socket = new wxSocketClient();

	// Set up the event handler and subscribe to most events
	Socket->SetEventHandler(*this, SOCKET_ID);
	Socket->SetNotify(wxSOCKET_CONNECTION_FLAG |
							wxSOCKET_INPUT_FLAG |
							wxSOCKET_LOST_FLAG);
	Socket->Notify(true);
	
	// Wait for the connection event
	Socket->Connect(addr, false);
}

void MainFrame::DecodeBufferData(wxString wx_in)
{
	int i, j, new_entry=0, orig_unsolved=0, comp_num=0, you_percent_complete;
	int punish_level, per_comp, orig_num=0, new_num=0;
	int opp_percent_complete, opp_comp, opp_req, you_comp, you_req, orig_puzzle;
	float opp_per_comp, you_per_comp;
	wxString tmp_str, str_c, str;
	
	
	// If this is a complete packet
	if (wx_in.Len() == 100)
	{
		wx_in.GetChar(3);
		
		opp_req = (int)(wx_in.GetChar(2)) - 40;
		opp_comp = (int)(wx_in.GetChar(3)) - 40;
		
		if (wx_in.GetChar(7) == '1' && !suGameOver && opp_req == opp_comp)
		{
			NetPlayGameOver();
			return;
		}
		opp_req = (int)(wx_in.GetChar(2)) - 40;
		if (opp_req == 81)
			for (i=0; i<60; i++)
			{
				suDisabled = true;
				suNetComplete[i] = 0;
			}
		else
			suDisabled = false;

		new_entry = wx_in.GetChar(1) - 48;
		// --------------------------------------------------------
		// |         Part to deal with eliminating values         |
		// --------------------------------------------------------
		// Count the number of entries the opponent has completed
		// in the last 60 seconds, not counting the last bout
		for (i=0; i<60; i++)
			orig_num = orig_num + suNetComplete[i];

		// Pop the last value, shift everything and insert the 
		// new value, and count the new number of entries the 
		// opponent has completed in the last 60 seconds
		for (i=58; i>=0; i--)
		{
			suNetComplete[(i + 1)] = suNetComplete[i];
			new_num = new_num + suNetComplete[(i+1)];
		}
		suNetComplete[0] = new_entry;
		new_num = new_entry + new_num;

		// if the number of squares the other person has completed has
		// decreased, deal out the punishment and reset the array
		if (new_num < orig_num)
		{
			//Debug();
			for (i=0; i<60; i++)
				suNetComplete[i] = 0;

			for (i=0; i<9; i++)
				for (j=0; j<9; j++)
					if (suValues[i][j] > 0 && suModes[i][j] < 2)
						comp_num++;

			// if the user has completed more than one square and the other
			// user has completed enough squares in the last minute to
			// translate to punishment
			if (comp_num > 0 && orig_num > 1)
			{
				PunishUser(comp_num, (orig_num - 1));
			}
		}
		
		// --------------------------------------------------------
		// |         Part to deal with Generating Percent         |
		// --------------------------------------------------------
		you_comp = 0;
		orig_puzzle = 0;

		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
				if (suValues[i][j] > 0 && suModes[i][j] < 2)
					you_comp++;
				else if (suValues[i][j] > 0 && suModes[i][j] == 2)
					orig_puzzle++;
		you_req = 81 - orig_puzzle;

		opp_per_comp = ( ((float)opp_comp) / ((float)opp_req) ) * (float)100;
		opp_percent_complete = (int)opp_per_comp;
		you_per_comp = ( ((float)you_comp) / ((float)you_req) ) * (float)100;
		you_percent_complete = (int)you_per_comp;

		str.Printf("You: %d%% Opponent: %d%% Incoming Punishment: %d Squares!",
			you_percent_complete, opp_percent_complete, orig_num);
		SetStatusText(str);

		// --------------------------------------------------------
		// |         Part to deal with Chat String                |
		// --------------------------------------------------------

		str = wx_in.SubString(10, 99);
		str.Trim(true);

		if (wx_in.GetChar(5) == '1' && str.SubString(0, 5) != wxString(wxT("ffffff")))
			AddNetString(wxString("Opponent: "), str);
	}
}

void MainFrame::AddNetString(wxString type, wxString wx_in)
{
	wxString tmp_str;
	tmp_str = type;
	tmp_str.append(wx_in);
	tmp_str.append("\n");
	suNetSenseiString = tmp_str.append(suNetSenseiString);
	update_text = true;
	RefreshRect(wxRect(490, 50, 690, 500));
}

void MainFrame::PunishUser(int curr_user_comp, int num_removed)
{
	int i, j, num_c, num_elim;
	wxString str;

	// we know the number of numbers the user has entered and
	// the number of entries removed
	if (curr_user_comp <= num_removed)
	{
		num_elim = curr_user_comp;
		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
				if (suValues[i][j] > 0 && suModes[i][j] < 2)
				{
					suValues[i][j] = 0;
					suModes[i][j] = 0;
				}
	}
	else
	{
		num_c = num_removed;
		num_elim = num_removed;
		while (num_c > 0)
		{
			for (i=0; i<9; i++)
				for (j=0; j<9; j++)
					// add an element of randomness
					if ((rand()%2) == 1 && suValues[i][j] > 0 && suModes[i][j] < 2)
					{
						suValues[i][j] = 0;
						suModes[i][j] = 0;
						num_c--;
					}
		}
	}
	str.Printf("Watch out - the other user blew away %d of your squares!", num_elim);
	AddNetString(wxString(wxT("System: ")), str);
	Refresh();
	return;
}

wxString MainFrame::GenerateBufferData(void)
{
	wxString buff_str, str;
	char orig_chr, num_chr, correct=0;

	buff_str = wxString(wxT(""));
	int i, j, orig_num=0, num_comp=0;
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
		{
			if (suModes[i][j] < 2)
				orig_num++;
			if (suValues[i][j] > 0 && suModes[i][j] < 2)
				num_comp++;
			if (PuzzleSolution[i][j] == suValues[i][j])
				correct++;
		}

	// This function will generate the 100 character array which
	// translates to the data which gets exchanged between machines
	// during netplay and can translate to anything from numbers changed
	// to text
	// [0]:     The difficulty of the puzzle (0-6)
	// [1]:     The number of values which have been added since the last sync (0-81)
	// [2]:     The number of squares which the current user has completed (of the 
	//          non-original values)
	// [3]:     The number of squares originally needing completion
	// [4]:     Whether or not the current program is ready to go
	// [5]:     Whether or not a chat string is included
    // [6]:     Whether or not the server is acting as a server (1) or client(0)
	// [7]:     Whether or not the current client has successfully completed their puzzle (1/0)
	// 
	// [8-9]:   Leaving empty for future development
	// [10-99]: Area for a string for short chat sessions (already set)
	wxString tmp_string = BufferData;
	num_chr = orig_num + 40;
	// add 40 to make our lives easier with regard to debugging (makes strings sent across visible
	orig_chr = num_comp + 40;
	BufferData.Printf("%d%d%c%c%c     ", PuzzleDifficulty, suComplete, num_chr, orig_chr, '1' );
	str.Printf("%d %d %d %d %d ", PuzzleDifficulty, suComplete, num_comp, orig_num, '1' );

	BufferData.SetChar(4, '1');

	if (suChatText.Length() > 0)
		BufferData.SetChar(5, '1');
	else
		BufferData.SetChar(5, '0');

	if (correct == 81)
		BufferData.SetChar(7, '1');
	else
		BufferData.SetChar(7, '0');

	if (suServerMode)
		BufferData.SetChar(6, '1');
	else
		BufferData.SetChar(6, '0');

	BufferData.append(suChatText);
	BufferData.Pad(100, ' ', true);

	suComplete = 0;
	suChatText.Clear();

	//SetStatusText(BufferData, 1);
	return BufferData;
}

// when a client connects to this server
void MainFrame::OnSudClientConnect(wxSocketEvent& WXUNUSED(event))
{
	// Accept the new connection and get the socket pointer
	wxSocketBase* sock = m_server->Accept(false);
	// Tell the new socket how and where to process its events
	sock->SetEventHandler(*this, SOCKET_ID);
	sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	sock->Notify(true);
}

void MainFrame::NetPlayGameOver(void)
{
	wxString str;
	suGameOver = true;
	str.Printf("Your adversary has gotten the better of you! Take revenge!\n");
	str.Append("The sudoku grid is now disabled until you start a new game");
	wxMessageBox(str, wxString("Sudoku Game Solver Generator"), wxOK, this);
}

void MainFrame::ProcessPackets(wxSocketEvent& event)
{
	wxSocketBase *sock = event.GetSocket();
	wxString wx_buf, wx_in;
	char buf[100];
	int i;
	
	if (suServerMode)
	{
		// If in server mode and a request was received
		switch(event.GetSocketEvent())
		{
			case wxSOCKET_INPUT:
			{
				suNetChange = true;
				// Read the data
				sock->Read(buf, sizeof(buf));

				// this will reflect the last time an event was raised with the server
				wx_in = wxString(buf, wxConvUTF8, 100);
				


				// Write something else back
				if (suConnectionEstablished == false)
					if(InitNetPlay(wx_in) == true)
					{
						// If the request was accepted, send f characters
						buf[99] = (char)PuzzleDifficulty;
						for (i=0; i<99; i++)
						{
							buf[i] = 'f';
						}
						suConnectionEstablished = true;
					}
					else
					{
						// If request was denied, send 0 characters
						for (i=0; i<100; i++)
						{
							buf[i] = '0';
						}
					}
				else if (suProcessing == false)
				{
					ContinueNetPlay(wx_in);
					//wx_buf = GenerateBufferData();
					wx_buf = GenerateBufferData();
					for (i=0; i<100; i++)
						buf[i] = wx_buf.GetChar(i);
					DecodeBufferData(wx_in);
				}

				// to let the client know we want it
				sock->Write(buf, sizeof(buf));
				break;
			}
			case wxSOCKET_LOST:
			{
				sock->Destroy();
				break;
			}
		}
	}
	else
	{
		// If in client mode, send a new string over
		switch(event.GetSocketEvent())
		{
			case wxSOCKET_CONNECTION:
			{
				// initially, send the relevant string, which triggers an input event
				// on the side of the server
				wx_buf = GenerateBufferData();
				for (i=0; i<100; i++)
					buf[i] = wx_buf.GetChar(i);
				sock->Write(buf, sizeof(buf));
				break;
			}
			case wxSOCKET_INPUT:
			{
				sock->Read(buf, sizeof(buf));
				
				if(sock->Error())
				{
					SetStatusText(wxString(wxT("ERROR: Connection Lost")), 0);
					return;
				}

				wx_in = wxString(buf, wxConvUTF8, 100);

				if (suConnectionEstablished == false)
					if (InitNetPlay(wx_in) == true)
					{
						suConnectionEstablished = true;
					}
					else
					{
						SetStatusText(wxString("Server has refused your request :(..."), 0);
					}
				else if (suProcessing == false)
				{
					ContinueNetPlay(wx_in);
					wx_buf = GenerateBufferData();
					for (i=0; i<100; i++)
						buf[i] = wx_buf.GetChar(i);
					DecodeBufferData(wx_in);
				}

				sock->Write(buf, sizeof(buf));
				sock->Destroy();
				break;
			}

			// The server hangs up after sending the data
			case wxSOCKET_LOST:
			{
				sock->Destroy();
				SetStatusText(wxString(wxT("ERROR: Connection Lost")), 0);
				break;
			}
			default:
				break;
		}
	}
}



void MainFrame::ResetAllArrays(void)
{
	int i, j, k;
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
		{
			suValues[i][j] = 0;
			suOriginalValues[i][j] = 0;
			suModes[i][j] = 0;
			grid_cache[i][j][0] = 0;
			for (k=1; k<10; k++)
			{
				grid_cache[i][j][k] = 0;
				suButModes[i][j][k] = 0;
			}
		}
}

void MainFrame::ResetPlayArrays(void)
{
	int i, j, k;
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
		{
			if (suModes[i][j] < 2)
			{
				suModes[i][j] = 0;
				suValues[i][j] = 0;
				suOriginalValues[i][j] = 0;
			}
			for (k=0; k<9; k++)
			{
				suButModes[i][j][k] = 0;
			}
		}
	timer_val = 0;

	return;
}

void MainFrame::OnPaint(wxPaintEvent& event)
{

	wxPaintDC dcp(this);
	wxColour bg_colour(suBGColour);
	dcp.SetBrush(wxBrush(bg_colour));
	dcp.SetPen(wxPen(bg_colour, 1));

	dcp.SetFont(wxFont(24, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false));
	wxRect update_region(OriginRedraw, GetClientSize());
	dcp.DrawRectangle(update_region);
	SetBackgroundColour(bg_colour);
	dcp.SetTextForeground(suTitleColour);
	dcp.DrawText(wxT("Sudoku Game Solver Generator"), wxPoint(140, 10));
	
	// Make sure the difference in points is divisable by 9
    if ( init_run == true )
    {
		but_pan_1 = new wxButton(this, ID_PAN_BUTTON_1, wxT(""));
		but_pan_2 = new wxButton(this, ID_PAN_BUTTON_2, wxT(""));
		but_pan_3 = new wxButton(this, ID_PAN_BUTTON_3, wxT(""));
		but_pan_4 = new wxButton(this, ID_PAN_BUTTON_4, wxT(""));
		// Initialise the Group Box Text boxes
		SenseiText.Create(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
			wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP);
		SenseiText.Hide();
		StatusText.Create(this, wxID_ANY, wxEmptyString);
		StaticSenseiText.Create(this, wxID_ANY, wxEmptyString);
	}

	DrawRightPanel(wxPoint(490, 50), wxPoint(690, 500));
	dcp.SetTextForeground(suDefaultFontColour);
	DrawSudokuGrid(wxPoint(25, 65), wxSize(450, 450));
	DrawOuterLetters(wxPoint(25, 65));
	DrawHighlightedSquares();
	if (!suCommon::suPrintEnabled)
	{
		suCommon::suDC->SelectObject(suCommon::bitmap);
		suCommon::suDC->SetBackground(*wxWHITE_BRUSH);
		suCommon::suDC->Clear();
		suCommon::suDC->Blit(wxPoint(0, 0), suCommon::InternalSize, &dcp, wxPoint(0, 0));
	}

    init_run = false;
}

wxString MainFrame::IntToWxString(int var)
{
	wxString str;
	str.Printf(wxT("%c"), var);
	return str;
}

void MainFrame::DrawOuterLetters(wxPoint GridTopLeft)
{
	int i;
	wxPoint top_ord, left_ord;
	wxClientDC dc(this);
	wxFont tmp_font;
	tmp_font = suDefaultFont;
	tmp_font.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(tmp_font);
	dc.SetTextForeground(suGBFontColour);
	top_ord.y = GridTopLeft.y - 15;
	left_ord.x = GridTopLeft.x - 10;


	for (i=0; i<9; i++)
	{
		top_ord.x = suSquareCoOrd[i][0].x + suButSize.x + suLINE_THICKNESS*2;
		left_ord.y = suSquareCoOrd[0][i].y + suButSize.x + suLINE_THICKNESS;

		suCommon::row_name[i] = IntToWxString(65+i);
		suCommon::col_name[i] = IntToWxString(49+i);

		dc.DrawText(suCommon::row_name[i], top_ord);
		dc.DrawText(suCommon::col_name[i], left_ord);
	}

}

wxPoint MainFrame::DrawGroupBox(wxPoint TopLeftBox, wxSize BoxSize, 
							 wxString TextTitle)
{
	wxClientDC dc(this);
	wxBrush dc_brush(suBGColour, wxSOLID);

	wxPoint text_offset((TopLeftBox.x + 30), (TopLeftBox.y - suFONT_SIZE));
	int width = (TextTitle.size()*(int)(suFONT_SIZE*3/4));
	dc.SetBrush(dc_brush);
	dc.SetPen(wxPen(suGBColour, 1));
	dc.SetFont(suDefaultFont);
	dc.DrawRoundedRectangle(TopLeftBox, BoxSize, 8);
	dc.SetPen(wxPen(suBGColour, 1));
	dc.DrawRectangle(wxPoint((text_offset.x - (int)(suFONT_SIZE/2)), text_offset.y), 
		wxSize(width, suFONT_SIZE*2));
	dc.SetTextForeground(suGBFontColour);
	dc.DrawText(TextTitle, text_offset);

	return wxPoint((TopLeftBox.x + suFONT_SIZE), (TopLeftBox.y + suFONT_SIZE));
}

void MainFrame::DrawStatusText(wxPoint StatusTextOrigin, wxSize StatusBoxSize)
{
	wxPoint status_text_origin = StatusTextOrigin;
	wxString str;

	if ( suMode == 0 )
	{
		str.Printf("Game is currently in entry mode - put in your entries and lets go!");
	}
	else if ( suMode == 1 && suNetPlay == false )
	{
		str.Printf("Game is currently in play mode - this means that the clock is running and");
		str.append(" you're good to go - Good Luck!");
	}
	else if ( suMode == 1 && suNetPlay)
	{
		str.Printf("Game is currently in net play mode ");
		if (suServerMode)
			str.append("(server).");
		else
			str.append("(client).");

		str.append(" Ctrl+t will send your opponent a message. Good Luck!");
	}
	else if ( suMode == 2 )
	{
		str.Printf("Game is currently in tutorial mode - you can play and get hints but your");
		str.append(" game won't be rated - Enjoy!");
	}
	else
	{
		str.Printf("OK - I have no clue what you're doing");
	}
	
	if (update_text == true)
	{
		StatusText.Hide();
		StatusText.SetPosition(StatusTextOrigin);
		StatusText.SetFont(suDefaultFont);
		StatusText.SetForegroundColour(suDefaultFontColour);
		StatusText.SetLabel(str);
		StatusText.Wrap(StatusBoxSize.GetX());
		StatusText.Show();
	}
}

wxString MainFrame::GenerateHintProverb(void)
{
	int proverb_count = 8;
	wxString proverb[8];
	
	proverb[0].Printf("Remember, mice may overcome canonball with gentle nibbles and muffled squeaks. ");
	proverb[1].Printf("Numbers are like people - only more logical. ");
	proverb[2].Printf("Half of everything is luck, the other half... fate. ");
	proverb[3].Printf("Search your feelings, luke... erm, I mean, advice for you I have... ");
	proverb[4].Printf("Be calm, be still, the answers will come themselves. ");
	proverb[5].Printf("Stillness is the first step to unlocking your chi. ");
	proverb[6].Printf("Step back, my child - see the greater picture. ");
	proverb[7].Printf("Much to learn you still have, my old Padawan. ");
	
	return proverb[GenerateRandomNumber(0, 7)];

}

wxString MainFrame::GeneratePlayProverb(void)
{
	int proverb_count = 8;
	wxString proverb[8];
	
	proverb[0].Printf("Be calm, be still, the answers will come. ");
	proverb[1].Printf("There is a great deal to be said about the road less travelled. ");
	proverb[2].Printf("I admire your bravery for taking on this task alone. ");
	proverb[3].Printf("BBBRRRRIIIINNNGGG ITTTT OOOONNNNN!! (With Stillness)");
	proverb[4].Printf("Nibble, my child, nibble. ");
	proverb[5].Printf("Bruce Lee liked to kung fu kick sudoku. ");
	proverb[6].Printf("Nice one! Go kick Daniel-San's ass! ");
	proverb[7].Printf("Numbers numbers numbers, fun fun FUN!. ");
	
	return proverb[GenerateRandomNumber(0, 7)];

}

wxString MainFrame::GenerateEntryProverb(void)
{
	int proverb_count = 8;
	wxString proverb[8];
	
	proverb[0].Printf("Remember - the tougher the puzzle, the more credit you get on the high scores menu for doing it quicker! ");
	proverb[1].Printf("If you like a puzzle or the way it's going, remember you can save at any point. ");
	proverb[2].Printf("Entry class contender? Try my yellow belt puzzles to get you started. ");
	proverb[3].Printf("Go on - live BEYOND your means! ");
	proverb[4].Printf("It's time to enter your puzzle - don't bite off more than you can chew! ");
	proverb[5].Printf("I am invincible! Mua hua hua huaaaa! ");
	proverb[6].Printf("If you like, you can enter a puzzle, enter into tutorial mode and I will help you through it. ");
	proverb[7].Printf("How brave are you? How much are you willing to risk? I can generate a puzzle to your taste... ");
	
	return proverb[GenerateRandomNumber(0, 7)];

}


void MainFrame::DrawSenseiText(wxPoint SenseiTextOrigin, wxSize SenseiSize)
{
	wxPoint sensei_text_origin = SenseiTextOrigin;
	wxString sensei_string;
	
	switch (suMode)
	{
		case 0:
			sensei_string = GenerateEntryProverb();
			break;
		case 1:
			sensei_string = GeneratePlayProverb();
			break;
		case 2:
			sensei_string = GenerateHintProverb();
			break;
		default:
			break;
	}
	if (suNetPlay)
		sensei_string = suNetSenseiString;

	if (suHint == true)
	{
		sensei_string.append(HintString);
		suHint = false;
	}

	if (update_text == true && suNetPlay)
	{
		StaticSenseiText.Hide();
		SenseiText.Hide();
		SenseiText.SetFont(suDefaultFont);
		SenseiText.SetPosition(sensei_text_origin);
		SenseiText.SetForegroundColour(suDefaultFontColour);
		SenseiText.Clear();
		SenseiText.WriteText(sensei_string);
		SenseiText.SetSize(SenseiSize);
		SenseiText.Show();
		update_text = false;
	}
	else if (update_text == true && !suNetPlay)
	{
		SenseiText.Hide();
		StaticSenseiText.Hide();
		StaticSenseiText.SetFont(suDefaultFont);
		StaticSenseiText.SetPosition(sensei_text_origin);
		StaticSenseiText.SetForegroundColour(suDefaultFontColour);
		StaticSenseiText.SetLabel(sensei_string);
		StaticSenseiText.SetSize(SenseiSize);
		StaticSenseiText.Wrap(SenseiSize.GetX());
		StaticSenseiText.Show();
		update_text = false;
	}
}

void MainFrame::UpdatePanelButtonText(void)
{
	if (suMode != suModeCache)
	{
		switch(suMode)
		{
			case 0:
				but_pan_1->Hide();
				but_pan_1->SetLabel(wxT("[QS Step 1] Generate New Puzzle"));
				but_pan_1->Show();
				//but_pan_1->Refresh();
				but_pan_2->Hide();
				but_pan_2->SetLabel(wxT("Reset and Begin Again"));
				but_pan_2->Show();
				//but_pan_2->Refresh();
				but_pan_3->SetLabel(wxT("[QS Step 2] Begin this Puzzle"));
				but_pan_3->Refresh();
				but_pan_4->SetLabel(wxT("Load Puzzle"));
				but_pan_4->Refresh();
				break;
			case 1:
				but_pan_1->Show();
				but_pan_1->SetLabel(wxT("Save Progress"));
				but_pan_1->Refresh();
				but_pan_2->SetLabel(wxT("Re-Attempt this Puzzle"));
				but_pan_2->Refresh();
				but_pan_3->SetLabel(wxT("Switch to Tutorial Mode"));
				but_pan_3->Refresh();
				but_pan_4->SetLabel(wxT("Pause (Minimises Window)"));
				but_pan_4->Refresh();
				break;
			case 2:
				but_pan_1->SetLabel(wxT("Skip Solving Steps"));
				but_pan_1->Refresh();
				but_pan_2->SetLabel(wxT("Check Current Errors"));
				but_pan_2->Refresh();
				but_pan_3->SetLabel(wxT("Reveal Solution"));
				but_pan_3->Refresh();
				but_pan_4->SetLabel(wxT("Hint"));
				but_pan_4->Refresh();
				break;
			default:
				break;
		}
                suModeCache = suMode;
	}
}

wxPoint MainFrame::NextLine(wxPoint CurrentPoint, int FontSize, int LineNumber)
{
	return wxPoint(CurrentPoint.x, (CurrentPoint.y + 2*FontSize*LineNumber));
}

void MainFrame::DrawRightPanel(wxPoint TopLeftPan, wxPoint BotRightPan)
{
	wxSize sensei_panel_size;
	wxSize sensei_status_size;
	wxPoint status_origin = DrawGroupBox(
		wxPoint(TopLeftPan.x, (TopLeftPan.y + suFONT_SIZE)), 
		wxSize((BotRightPan.x - TopLeftPan.x), 60),
		wxString(wxT("Status"))
		);
	sensei_status_size = wxSize((BotRightPan.x - TopLeftPan.x - 2*suFONT_SIZE), 
		(60 + 2*suFONT_SIZE));

	DrawStatusText(status_origin, sensei_status_size);

	// Timer is in here but it is dealt with in a different function
	// It's co-ordinates are 510x145 and size is 160x35

	but_pan_1->SetPosition(	wxPoint(TopLeftPan.x, (TopLeftPan.y+130)) );
	but_pan_2->SetPosition(	wxPoint(TopLeftPan.x, (TopLeftPan.y+190)) );
	but_pan_3->SetPosition(	wxPoint(TopLeftPan.x, (TopLeftPan.y+160)) ); // xchg positions
	but_pan_4->SetPosition(	wxPoint(TopLeftPan.x, (TopLeftPan.y+220)) );

	but_pan_1->SetSize(wxSize((BotRightPan.x - TopLeftPan.x), 25));
	but_pan_2->SetSize(wxSize((BotRightPan.x - TopLeftPan.x), 25));
	but_pan_3->SetSize(wxSize((BotRightPan.x - TopLeftPan.x), 25));
	but_pan_4->SetSize(wxSize((BotRightPan.x - TopLeftPan.x), 25));

	UpdatePanelButtonText();
	wxPoint sensei_origin = DrawGroupBox(
		wxPoint(TopLeftPan.x, (TopLeftPan.y + 270)), 
		wxSize((BotRightPan.x - TopLeftPan.x), 180),
		wxString(wxT("Our Sensei Says:"))
		);

	sensei_panel_size = wxSize((BotRightPan.x - TopLeftPan.x - 2*suFONT_SIZE), 
		(180 - 2*suFONT_SIZE));

	DrawSenseiText(sensei_origin, sensei_panel_size);

}

void MainFrame::DrawSudokuGrid(wxPoint TopLeft, wxSize GridSize)
{
	int width=GridSize.GetWidth();
	int height=GridSize.GetHeight();
	int height_unit=(int)(height/9);
	int width_unit=(int)(width/9);
	int i, j, k;
	wxClientDC dc(this);
	wxPoint BotRight((TopLeft.x + width), (TopLeft.y + height));

	suSquareSize = wxSize((width_unit-(int)suLINE_THICKNESS), (height_unit-(int)suLINE_THICKNESS));
	suButSize = MainFrame::GetButtonSize( suSquareSize );
	// Calculating these now to ease CPU
	int su_but_height = suButSize.GetHeight();
	int su_but_width = suButSize.GetHeight();

	// Initialise the dc line for the grid
	dc.SetPen(wxPen(wxColour(210, 210, 210), (int)suLINE_THICKNESS, wxLONG_DASH));

	for (i=0; i<10; i++)
	{
		if (i == 0)
		{
			x_tl[i] = TopLeft.x;
			y_tl[i] = TopLeft.y;
		} else {
			x_tl[i] = x_tl[i-1] + width_unit;
			y_tl[i] = y_tl[i-1] + height_unit;
		}

		// Vertical Lines
		dc.DrawLine( x_tl[i], (TopLeft.y - 10), x_tl[i], BotRight.y );
		// Horizontal Lines
		dc.DrawLine( (TopLeft.x - 10 - (int)(suLINE_THICKNESS/2)), y_tl[i], BotRight.x, y_tl[i] );
		
	}

	dc.SetPen(wxPen(wxColour(wxT("BLACK")), (int)suLINE_THICKNESS));

	for (i=0; i<10; i=(i+3))
	{
		if (i == 0)
		{
			x_tl[i] = TopLeft.x;
			y_tl[i] = TopLeft.y;
		} else {
			x_tl[i] = x_tl[i-1] + width_unit;
			y_tl[i] = y_tl[i-1] + height_unit;
		}

		// Vertical Lines
		dc.DrawLine( x_tl[i], TopLeft.y, x_tl[i], BotRight.y );
		// Horizontal Lines
		dc.DrawLine( (TopLeft.x - (int)(suLINE_THICKNESS/2)), y_tl[i], BotRight.x, y_tl[i] );
		
	}


	// rows
	for (i=0; i<9; i++)
	{
		// columns
		for (j=0; j<9; j++)
		{
			suSquareCoOrd[i][j] = wxPoint(
				 (x_tl[i] + (int)(suLINE_THICKNESS/2) ), 
				 (y_tl[j] + (int)(suLINE_THICKNESS/2) ) 
				);
			DrawSquare(su_but_height, su_but_width, i, j);
		}
	}

}

void MainFrame::DrawSquare(int su_but_height, int su_but_width,
						   int x_pt, int y_pt)
{
	int y_offset, x_offset, k, row, col, value;
	wxString str;
    wxClientDC dc(this);
	wxPoint top_left;

	wxFont dcFont = suDefaultFont;
	dcFont.SetPointSize(16);
	dcFont.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(dcFont);

	wxColour line_colour(wxT("BLACK"));
	dc.SetPen(wxPen(line_colour, 1));

	if (suModes[x_pt][y_pt] == 0)
	{
		// This is for the mode for entering values
		for (k=0; k<9; k++)
		{
			row = (int)(k/3);
			col = (int)(k%3);
			x_offset = (k%3)*su_but_width + suSquareCoOrd[x_pt][y_pt].x;
			y_offset = ((int)(k/3))*su_but_height + suSquareCoOrd[x_pt][y_pt].y;
			top_left = wxPoint(x_offset, y_offset);
			value = k + 1;
			str.Printf("%d", value);
			DrawButton(value, top_left);
		}
	} 
	else if (suModes[x_pt][y_pt] == 1)
	{
		// This is for the mode for entries before they're confirmed
		// and requires a button (for undo) and a text entry to denote
		// the value
		str.Printf("%d", suValues[x_pt][y_pt]);

		DrawButton(88, suSquareCoOrd[x_pt][y_pt]);

		dc.DrawText(str, 
			wxPoint(
				(suSquareCoOrd[x_pt][y_pt].x + (int)(suSquareSize.GetWidth()/3)), 
				(suSquareCoOrd[x_pt][y_pt].y + (int)(suSquareSize.GetHeight()/3))
			)
		);
	}
	else if (suModes[x_pt][y_pt] == 2)
	{
		// This is for entries once they're set in stone and can't
		// be undone. It requires a big text area (maybe bold)
		str.Printf("%d", suValues[x_pt][y_pt]);
		dc.SetPen(wxPen(wxColour(175, 238, 238), 1));
		dc.SetBrush(wxBrush(wxColour(175, 238, 238)));
		dc.DrawRectangle(wxRect(suSquareCoOrd[x_pt][y_pt], suSquareSize));
		dc.DrawText(str, 
			wxPoint(
				(suSquareCoOrd[x_pt][y_pt].x + (int)(suSquareSize.GetWidth()/3)), 
				(suSquareCoOrd[x_pt][y_pt].y + (int)(suSquareSize.GetHeight()/3))
			)
		);
	}
}

void MainFrame::DrawButton(int entry, wxPoint top_left)
{
	// Lots of native buttons were killing the CPU - these are my own,
	// custom made buttons created in the interest of speed, and in an
	// attempt to look like the real buttons.
	wxString str;
	wxRegion upd = GetUpdateRegion();
	wxClientDC dc(this);
	dc.SetBackgroundMode(wxTRANSPARENT);
	

	if (entry > 10)
		str.Printf("%c", entry);
	else
		str.Printf("%d", entry);
	

	wxPoint ins_tl((top_left.x + 2), (top_left.y + 2));
	wxSize ins_size((suButSize.GetWidth() - 4), (suButSize.GetHeight() - 4));
	wxPoint bot_right(
		(top_left.x + suButSize.GetWidth() - 1), 
		(top_left.y + suButSize.GetHeight() - 1)
	);

	top_left.x++;
	top_left.y++;

	dc.SetPen(wxPen(wxColour(wxT("WHITE")), 2));
	dc.DrawLine(top_left, wxPoint(bot_right.x, top_left.y));
	dc.DrawLine(top_left, wxPoint(top_left.x, bot_right.y));

	dc.SetPen(wxPen(wxColour(wxT("GREY")), 2));
	dc.DrawLine(bot_right, wxPoint(bot_right.x, top_left.y));
	dc.DrawLine(bot_right, wxPoint(top_left.x, bot_right.y));

	dc.SetBrush(wxBrush(suGridColour));
	dc.SetPen(wxPen(suGridColour, 1));
	dc.DrawRectangle(ins_tl, ins_size);

	dc.SetPen(wxPen(suDefaultFontColour, 1));
	dc.SetFont(suDefaultFont);
	dc.SetTextForeground(suDefaultFontColour);
	dc.DrawText(str, ins_tl);

	if (suButModes[GetCol(top_left)][GetRow(top_left)][entry] == 1 && entry != 88)
	{
		dc.SetPen(wxPen(suGBFontColour, 2, wxDOT));
		dc.DrawLine(top_left, bot_right);
		dc.DrawLine(wxPoint(top_left.x, bot_right.y), wxPoint(bot_right.x, top_left.y));
	}
	else if (suButModes[GetCol(top_left)][GetRow(top_left)][entry] == 2 && entry != 88)
	{
		dc.SetPen(wxPen(suGBFontColour, 2));
		dc.DrawLine(top_left, bot_right);
		dc.DrawLine(wxPoint(top_left.x, bot_right.y), wxPoint(bot_right.x, top_left.y));
	}

}
void MainFrame::SetStatusTime()
{
	wxDateTime now = wxDateTime::Now();
	//SetStatusText(now.FormatTime(), 1);
}

wxSize MainFrame::GetButtonSize(wxSize area)
{
	return wxSize((area.GetHeight()/3), (area.GetWidth()/3));
}


void MainFrame::InitClock(void)
{
	m_timer = new wxTimer();
    m_timer->SetOwner(this, TIMER_ID);
    m_timer->Start(1000);
}

void MainFrame::InitStatusBar(void)
{
	wxStatusBar *statusBar = CreateStatusBar(2);
	SetStatusBar(statusBar);
}

void MainFrame::DrawMenu(void)
{
	wxMenu *fileMenu = new wxMenu;
	//wxMenu *editMenu = new wxMenu;
	wxMenu *viewMenu = new wxMenu;
	wxMenu *helpMenu = new wxMenu;
	//wxMenu *netplayMenu = new wxMenu;
	wxMenuBar *menuBar = new wxMenuBar;
	fileMenu->Append(wxID_NEW, "&New...\tCtrl+n");
	fileMenu->Append(wxID_SAVE, "&Save\tCtrl+s");
	fileMenu->Append(wxID_SAVE_AS, "Save As...");
	fileMenu->Append(wxID_LOAD, "&Load\tCtrl+o");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_PRINT, "&Print\tCtrl+p");
	fileMenu->Append(wxID_PRINT_PREVIEW, "Print Preview");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "E&xit");
	//editMenu->Append(wxID_DISABLE_TIMER, "Turn Timer On/Off");
	//editMenu->Append(wxID_TUT_MODE, "Turn on Tutorial Mode");
	//editMenu->Append(wxID_PLAY_MODE, "Turn on Play Mode");
	//viewMenu->Append(wxID_DEFAULT_FONT, "Change Default Font");
	//viewMenu->Append(wxID_DEFAULT_FONT_COLOUR, "Change Default Font Colour");
	//viewMenu->Append(wxID_GRID_COLOUR, "Change Grid Colour");
	//viewMenu->Append(wxID_GB_COLOUR, "Change Group Box Colour");
	//viewMenu->Append(wxID_GB_FONT_COLOUR, "Change Minor Title's Colour");
	//viewMenu->Append(wxID_BG_COLOUR, "Change Background Colour");
	//viewMenu->Append(wxID_TITLE_COLOUR, "Change Main Title Colour");
	//viewMenu->AppendSeparator();
	//viewMenu->Append(wxID_SAVE_SCHEME, "Save the Current Colour Scheme");
	//viewMenu->Append(wxID_RESET_SCHEME, "Reset to the default Colour Scheme");
	//viewMenu->AppendSeparator();
	viewMenu->Append(wxID_HIGH_SCORES, "High Scores");
	//netplayMenu->Append(wxID_NET_PLAY, "Start New Net Play Session");
	//netplayMenu->Append(wxID_NET_PORT, "Edit the Port for Net Play (Advanced)");
	//netplayMenu->Append(wxID_NET_MSG, "Send Message to Your Enemy\tCtrl+t");
	//helpMenu->Append(wxID_HELP, "Help Contents");
	helpMenu->Append(wxID_RULES, "Sudoku &Rules");
	helpMenu->AppendSeparator();
	helpMenu->Append(wxID_ABOUT, "About...");
	//helpMenu->Append(wxID_DONATE, "Donate");
	menuBar->Append(fileMenu, "&Game");
	//menuBar->Append(editMenu, "&Edit");
	menuBar->Append(viewMenu, "&View");
	//menuBar->Append(netplayMenu, "&NetPlay");
	menuBar->Append(helpMenu, "&Help");
	SetMenuBar(menuBar);
}


void MainFrame::OnTimer(wxTimerEvent& event)
{
	wxString str;
	wxClientDC dc(this);
	wxFont tmp_font;
	wxBrush dc_brush(suBGColour, wxSOLID);
	tmp_font = suDefaultFont;
	tmp_font.SetPointSize(18);

	dc.SetBrush(dc_brush);

	if ( this->IsIconized() == false && timer_enabled && !suDisabled)
		timer_val++;

	
	// Only the client needs to send the updates - the server will 
	// respond with it's update back to the client
	if (suNetPlay && suServerMode == false && suServerHost != wxString(wxT(""))
		  && suConnectionEstablished == true && !suGameOver)
		StartSudClient();
	if (suNetPlay && suServerMode && suConnectionEstablished && !suDisabled)
		if (!suNetChange || !m_server->IsOk())
			SetStatusText(wxString(wxT("ERROR: Connection Lost")), 0);
	
	if (suGameOver)
		suDisabled = true;

	suNetChange = false;

	int secs, mins, hours;
	wxString secs_s, mins_s, hours_s;

	secs = (int)(timer_val%60);
	mins = (int)(((timer_val-secs)/60)%60);
	hours = (int)((timer_val-secs-(60*mins)));

	timer_bd[0] = secs;
	timer_bd[1] = mins;
	timer_bd[2] = hours;

	wxDateTime now = wxDateTime::Now();
	now.FormatTime();
	
	// If program is just started, check if it is up to date...
	if (epoch_start == ((long)now.GetTicks() - 5))
	{
		CheckUpToDate();
	}
	
	wxString str_time(now.Format("%c", wxDateTime::CET).c_str());
	if(suMode == 1)
		str.Printf("TIMER: %dh:%dm:%ds", hours, mins, secs);
	else
	{
		str.Printf("TIME: ", hours, mins, secs);
		str.append(now.FormatTime());
		str.append(wxT(" hrs"));
	}
	
	if (timer_enabled == false)
		str.Printf("TIMER Disabled");

	
	dc.SetPen(wxPen(suBGColour, 1));
	dc.DrawRectangle(wxPoint(490, 135), wxSize(220, 35));
	dc.SetPen(wxPen(suDefaultFontColour, 1));
	dc.SetFont(tmp_font);
	dc.DrawText(str, wxPoint(490, 135));
	SetStatusTime();
	
}

int MainFrame::GetRow(wxPoint mouse)
{
	int y_point = mouse.y;
	int u_limit = 0;
	int l_limit = 0;
	for (int i=0; i<10; i++)
	{
		u_limit = suSquareCoOrd[0][i].y + suSquareSize.y;
		l_limit = suSquareCoOrd[0][i].y;
		if (y_point >= l_limit && y_point <= u_limit)
			return i;
	}
	return 99;
}

int MainFrame::GetCol(wxPoint mouse)
{
	int x_point = mouse.x;
	int u_limit = 0;
	int l_limit = 0;
	for (int i=0; i<10; i++)
	{
		u_limit = suSquareCoOrd[i][0].x + suSquareSize.y;
		l_limit = suSquareCoOrd[i][0].x;
		if (x_point >= l_limit && x_point <= u_limit)
			return (i);
	}
	return 99;
}


int MainFrame::GetEntry(wxPoint mouse_rel, int row, int col)
{
	int i=0, but_row=0, but_col=0, offset=0, found_r=0, found_c=0;
	wxPoint TopLeft = suSquareCoOrd[col][row];

	for (i=1; i<4; i++)
	{
		offset = i*suButSize.GetWidth();
		if ((TopLeft.x + offset) > mouse_rel.x && found_c == 0)
		{
			but_col = i-1;
			found_c = 1;
		}
		offset = i*suButSize.GetHeight();
		if ((TopLeft.y + offset) > mouse_rel.y && found_r == 0)
		{
			but_row = i-1;
			found_r = 1;
		}
	}

	//
	return ((3*but_row) + but_col + 1);
}

bool MainFrame::SudGridPressCheck(wxPoint mouse_rel)
{
	int i, j;
	for (i=0; i<9; i++)
	{
		for (j=0; j<9; j++)
		{
			if (
				mouse_rel.x >= suSquareCoOrd[i][j].x &&
				mouse_rel.x <= (suSquareCoOrd[i][j].x + suSquareSize.GetHeight()) &&
				mouse_rel.y >= suSquareCoOrd[i][j].y &&
				mouse_rel.y <= (suSquareCoOrd[i][j].y + suSquareSize.GetHeight())
				)
				return true;
		}
	}
	return false;
}

void MainFrame::SaveFileAs()
{
	
	wxString str = wxFileSelector(wxT("Save As..."), wxT(""),
		CurrentFileName, wxT(".uss"), wxT("*.uss"), 
		wxOVERWRITE_PROMPT|wxSAVE, this);

	if (str != wxT(""))
	{
		SaveFile(str);
		CurrentFileName = str;
		wxString new_str, base_name;
		wxFileName file_name(str, wxPATH_NATIVE);
		
		base_name = file_name.GetFullName();
		new_str = suCommon::default_title_text;
		new_str.Append(" - ");
		new_str.Append(base_name);
		this->SetTitle(new_str);
		new_str.Clear();
	}

}


void MainFrame::OnSave(wxCommandEvent& event)
{
	if (CurrentFileName == wxT(""))
	{
		SaveFileAs();
	}
	else
	{
		SaveFile(CurrentFileName);
	}
}


void MainFrame::OnSaveAs(wxCommandEvent& event)
{
	SaveFileAs();
}


void MainFrame::OnNew(wxCommandEvent& event)
{
	if ( ! ConfirmBox(wxString(wxT("start again with a blank page"))) )
		return;
	Reset();
}

void MainFrame::LoadPuzzle(void)
{
	wxString base_name;
	wxString str = wxFileSelector(wxT("Load Puzzle..."), wxT(""),
		CurrentFileName, wxT(".uss"), wxT("*.uss"), 
		wxFILE_MUST_EXIST|wxOPEN, this);
	
	if (str != wxT(""))
	{
		LoadFile(str);
		CurrentFileName = str;
		wxString new_str;
		wxFileName file_name(str, wxPATH_NATIVE);
		
		base_name = file_name.GetFullName();
		new_str = suCommon::default_title_text;
		new_str.Append(" - ");
		new_str.Append(base_name);
		this->SetTitle(new_str);
		new_str.Clear();
	}

}

void MainFrame::OnLoad(wxCommandEvent& event)
{
	LoadPuzzle();
}


void MainFrame::OnPrintPreview(wxCommandEvent& event)
{
	suCommon::suPrintEnabled = true;
	// Pass two printout objects: for preview, and possible printing.
	wxPrintPreview *preview = new wxPrintPreview(new suPrint, new suPrint);
	wxPreviewFrame *frame = new wxPreviewFrame(preview, this, 
		"Sudoku Game Solver Generator printout", wxPoint(100, 100), 
		wxSize(600, 650));
	frame->Centre(wxBOTH);
	frame->Initialize();
	frame->Show(true);
	suCommon::suPrintEnabled = false;
}


void MainFrame::OnDisableTimer(wxCommandEvent& event)
{
	wxString str;

	if (timer_enabled)
	{
		if ( ! ConfirmBox(wxString(wxT("disable the 'play mode' timer? It means your games would be unrated"))) )
			return;
		timer_enabled = false;
		str.Printf("Play mode timer is now disabled");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
	else
	{
		timer_enabled = true;
		str.Printf("Play mode timer is now enabled");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}

	return;
}


void MainFrame::OnDefaultFont(wxCommandEvent& event)
{
	wxString str;
	wxFont tmp_font;

	tmp_font = wxGetFontFromUser(this, suDefaultFont, wxString(wxT("Select your default Font (keep below 12pt)")));
	if (tmp_font.IsOk() && tmp_font.GetPointSize() < 12)
	{
		suDefaultFont = tmp_font;
		update_text = true;
		Refresh();
		str.Printf("The default font has now been changed");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
	else if (tmp_font.GetPointSize() < 12 || tmp_font.IsOk())
	{
		str.Printf("Font is invalid - did you enter a size above 12?");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
}

void MainFrame::OnDefaultFontColour(wxCommandEvent& event)
{
	wxColour tmp_colour;
	wxString str;
	tmp_colour = wxGetColourFromUser(this, suDefaultFontColour, wxString(wxT("Select your default font colour")));
	
	if (tmp_colour.IsOk())
	{
		suDefaultFontColour = tmp_colour;
		update_text = true;
		Refresh();
		str.Printf("The default font colour has now been changed");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
}

void MainFrame::OnDefaultGBColour(wxCommandEvent& event)
{
	wxColour tmp_colour;
	wxString str;
	tmp_colour = wxGetColourFromUser(this, suGBColour, wxString(wxT("Select your default group box colour (right panel)")));
	if (tmp_colour.IsOk())
	{
		suGBColour = tmp_colour;
		Refresh();
		str.Printf("The group box colour has now been changed");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
}

void MainFrame::OnDefaultHeadingColour(wxCommandEvent& event)
{
	wxString str;
	wxColour tmp_colour;

	tmp_colour = wxGetColourFromUser(this, suGBColour, wxString(wxT("Select your default title colour")));
	if (tmp_colour.IsOk())
	{
		suGBFontColour = tmp_colour;
		Refresh();
		str.Printf("The title's colour has now been changed");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
}

void MainFrame::OnDefaultTitleColour(wxCommandEvent& event)
{
	wxColour tmp_colour;
	wxString str;

	tmp_colour = wxGetColourFromUser(this, suTitleColour, 
		wxString(wxT("Select your default main title colour")));

	if (tmp_colour.IsOk())
	{
		suTitleColour = tmp_colour;
		Refresh();
		str.Printf("The main title's colour has now been changed");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
}




void MainFrame::OnGridColour(wxCommandEvent& event)
{
	wxColour tmp_colour;
	wxString str;
	tmp_colour = wxGetColourFromUser(this, suGridColour, wxString(wxT("Select your default grid button colour")));
	
	if (tmp_colour.IsOk())
	{
		suGridColour = tmp_colour;
		Refresh();
		str.Printf("The grid's colours are now changed");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}
}


void MainFrame::OnBgColour(wxCommandEvent& event)
{
	wxColour tmp_colour;
	wxString str;
	tmp_colour = wxGetColourFromUser(this, suBGColour, wxString(wxT("Select your default background colour")));
	
	if (tmp_colour.IsOk())
	{
		suBGColour = tmp_colour;
		Refresh();
		str.Printf("The background's colours are now changed");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
	}

}


void MainFrame::OnHighScores(wxCommandEvent& event)
{
	DrawHighScores();
}

void MainFrame::DrawHighScores(void)
{
	wxString str, HighString, name_str, score_str, date_str;
	wxDateTime dt;
	int i, pos;
	wxString name;
	double score, date;

	str.Printf("Fame is the least important aspect of the Sudoku Sensei Way...\n\n");
	HighString.append(str);
	str.Printf("Still - It's always nice to know who's the best and most feared\n");
	HighString.append(str);
	str.Printf("of the Sudoku Sensei clan so one can more wisely choose thine\n");
	HighString.append(str);
	str.Printf("enemy! Go ahead! Gloat!\n\n");
	HighString.append(str);

	for (i=0; i<10; i++)
	{

		if (HighScoreName[i] == wxString(wxT("")))
			name_str.Printf("Unset\t");
		else
		{
			name_str = HighScoreName[i];
			str.Printf("\t");
			name_str.append(str);
		}
		
		if (HighScoreVal[i] == 0)
			score_str.Printf(" - \t");
		else
			score_str.Printf("%d\t", HighScoreVal[i]);

		if (HighScoreDate[i] == wxString(wxT("")))
			date_str.Printf("\t - \t\n");
		else
		{
			date_str.Printf("\t");
			date_str.append(HighScoreDate[i]);
			str.Printf("\t\n");
			date_str.append(str);
		}

		pos = i+1;
		str.Printf(" %d)\t", pos);
		str.append(	name_str );
		str.append(	score_str );
		str.append( date_str );
		HighString.append(str);
	}

	wxMessageBox(HighString, wxT("Sudoku Game Solver Generator"), wxOK, this);
}

void MainFrame::WriteNetPort(int port_no)
{
	wxXmlDocument doc;
	wxString str;
	wxXmlNode *level1;
	wxXmlNode *level2;

	if (!doc.Load(suConfigFile))
	{
		DebugString(wxString("Unable to open file"));
		return;
	}
	if (doc.GetRoot()->GetName() != wxT("suConfig"))
	    return;

	
	// Level1 is the highest in the tree = level 2 is a child etc.
	level1 = doc.GetRoot()->GetChildren();
	while (level1) {
		// this is the highscores stanza
		if (level1->GetName() == wxT("default"))
		{
			level2 = level1->GetChildren();
			while (level2)
			{
				if (level2->GetName() == wxT("netplay_port"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("value"));
					str.Printf("%d", port_no);
					level2->AddProperty(wxT("value"), str);
				}
				level2 = level2->GetNext();
			}
		}
		level1 = level1->GetNext();
	}

	if (!doc.Save(suConfigFile))
		return;
}

bool MainFrame::WriteHighScores(void)
{
	int offset, i;
	wxXmlDocument doc;
	wxString str;
	wxXmlNode *level1;
	wxXmlNode *level2;
	wxXmlProperty *level3;
	wxXmlNode *nodeTxt;
	
	nodeTxt = new wxXmlNode(wxXML_TEXT_NODE,"");
	wxString ContentStr;

	if (!doc.Load(suConfigFile))
		return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != wxT("suConfig"))
		return false;

	offset = 0;

	// Level1 is the highest in the tree = level 2 is a child etc.
	level1 = doc.GetRoot()->GetChildren();
	while (level1 && offset < 10) {
		// this is the highscores stanza
		if (level1->GetName() == wxT("highscores"))
		{
			level2 = level1->GetChildren();
			while (level2)
			{
				// this is the score stanza
				if (level2->GetName() == wxT("score"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("date"));
					level2->DeleteProperty(wxT("value"));
					level2->DeleteProperty(wxT("name"));

					ContentStr.Printf("%d", HighScoreVal[offset]);
					level2->AddProperty(wxString(wxT("date")), HighScoreDate[offset]);
					level2->AddProperty(wxString(wxT("value")), ContentStr);
					level2->AddProperty(wxString(wxT("name")), HighScoreName[offset]);
				}
				level2 = level2->GetNext();
				offset++;
			}
		}
		level1 = level1->GetNext();
	}
	if (!doc.Save(suConfigFile))
		return false;

	str.Printf(suConfigFile);

	return true;
}

void MainFrame::OnRules(wxCommandEvent& event)
{
	wxString str;
	str.Printf(wxT("https://en.wikipedia.org/wiki/Sudoku"));
	wxLaunchDefaultBrowser(str, 0);
}


void MainFrame::OnDonate(wxCommandEvent& event)
{
	wxString str;
	str.Printf(wxT("https://en.wikipedia.org/wiki/Sudoku")); // TODO: Federica Domani project on sourceforge
	wxLaunchDefaultBrowser(str, 0);
}


void MainFrame::OnHelp(wxCommandEvent& event)
{
	wxString str;
	str.Printf(wxT("http://snesreviews.co.uk/uss_home/"));
	wxLaunchDefaultBrowser(str, 0);
}


void MainFrame::OnTutorialMode(wxCommandEvent& event)
{
	wxString str;

	switch (suMode)
	{
		case 0:
			if ( ! ConfirmBox(wxString(wxT("enter tutorial mode and abandon your possible rating"))) )
				return;
			PuzzleDifficulty = VerifySolvable(2);
			update_text = true;
			break;
		case 1:
			if ( ! ConfirmBox(wxString(wxT("enter tutorial mode and abandon your possible rating"))) )
				return;
			suMode = 2;
			update_text = true;
			break;
		case 2:
			str.Printf("You're already in tutorial mode (check top right corner). Hope it helps!");
			wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
			break;
		default:
			break;
	}
	Refresh();
}


void MainFrame::OnPlayMode(wxCommandEvent& event)
{
	wxString str;

	switch (suMode)
	{
		case 0:
			if ( ! ConfirmBox(wxString(wxT("enter play mode and settle with this puzzle"))) )
				return;
			PuzzleDifficulty = VerifySolvable(1);
			update_text = true;
			break;
		case 1:
			str.Printf("You're already in play mode (check top right corner). Good luck!");
			wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
			break;
		case 2:
			str.Printf("Sorry - you can't go back to play mode from tutorial mode - it effects high scores.");
			wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK | wxICON_ERROR, this);
			break;
		default:
			break;
	}
}


void MainFrame::OnPrint(wxCommandEvent& event)
{
	wxPrinter printer;
	suPrint printout("Sudoku Game Solver Generator printout");
	printer.Print(this, &printout, true);
}

void MainFrame::GeneratePuzzleScreen(void)
{
	int answer;

	answer = wxGetSingleChoiceIndex(
			wxString(wxT("Choose your destiny, young one...")),
			wxString(wxT("Difficulty Selection")),
			7, difficulties, this, -1, -1, true, 150, 200);

	if (answer == -1)
		return;
	else
	{
		GeneratePuzzle(answer);
	}

	return;
}

void MainFrame::DrawHighlightedSquares(void)
{
	wxClientDC dc(this);
	int i;
	double rad=5;
	if (suMode == 2)
	{
		for (i=0; i<highlight_ac; i++)
		{
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.SetPen(wxPen(wxColour(255, 165, 0), 3, wxLONG_DASH));
			dc.DrawRoundedRectangle(highlight_pt[i], highlight_sz[i], rad);
		}
	}
}

void MainFrame::HighlightSquares(int square1_x, int square1_y, int square2_x, int square2_y)
{
	wxClientDC dc(this);

	wxPoint TopLeft = wxPoint(
			(suSquareCoOrd[square1_x][square1_y].x + suLINE_THICKNESS),
			(suSquareCoOrd[square1_x][square1_y].y + suLINE_THICKNESS)
		);
	wxPoint BotRight = wxPoint(
			(suSquareCoOrd[square2_x][square2_y].x + suSquareSize.x - suLINE_THICKNESS), 
			(suSquareCoOrd[square2_x][square2_y].y + suSquareSize.y - suLINE_THICKNESS)
		);
	wxSize reg_size = wxSize(
			(BotRight.x - TopLeft.x),
			(BotRight.y - TopLeft.y)
		);

	highlight_pt[highlight_ac] = TopLeft;
	highlight_sz[highlight_ac] = reg_size;
	highlight_ac++;
	Refresh();
	return;
}

void MainFrame::SkipStepsScreen(void)
{
	int num_choices = 0, n;
	int input_array[9][9][10];
	wxArrayInt selection;
	int step_num, i, j, k;
	char solve_mesh = 0x00;
	wxString step_opt[3];

	step_opt[0] = wxString(wxT("Skip all available simple elimination algorithms"));
	step_opt[1] = wxString(wxT("Skip all available lone ranger algorithms"));
	step_opt[2] = wxString(wxT("Skip all available step theory algorithms"));

	step_num = (int)wxGetMultipleChoices(
		selection,
		wxString(wxT("Pick which steps to skip past...")),
		wxString(wxT("Difficulty Selection")), 
		3, step_opt, this, -1, -1,
		true, 150, 200);

	if (step_num == -1)
		return;
	else
	{
		for ( n = 0; n < step_num; n++ )
		{
			solve_mesh = solve_mesh | (0x01 << selection[n]);
		}
		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
				for (k=0; k<10; k++)
				{
					if (k == 0)
						input_array[i][j][k] = suOriginalValues[i][j];
					else
						input_array[i][j][k] = suButModes[i][j][k];
				}
		SudokuPuzzle skip_puzzle(input_array);

		skip_puzzle.SolvePuzzle(solve_mesh);

		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
				for (k=0; k<10; k++)
				{
					if (k == 0)
					{
						if (suValues[i][j] == 0 && grid_cache[i][j][0] > 0)
						{
							suValues[i][j] = skip_puzzle.grid_cache[i][j][0];
							suModes[i][j] = 1;
						}
					}
					else
						suButModes[i][j][k] = skip_puzzle.grid_cache[i][j][k];
				}
		Refresh();
	}

	return;
}


void MainFrame::OnPan1(wxCommandEvent& event)
{
	switch(suMode)
	{
		//Commit Entries
		case 0:
			GeneratePuzzleScreen();
			break;
		case 1:
			if (CurrentFileName == wxT(""))
				SaveFileAs();
			else
				SaveFile(CurrentFileName);
			break;
		case 2:
			SkipStepsScreen();
			break;
		default:
			break;
	}
	UpdatePanelButtonText();
}

void MainFrame::Reset(void)
{
	ResetAllArrays();
	// Turn on entry mode
	suMode = 0;
	this->SetTitle(suCommon::default_title_text);
	suDisabled = false;
	update_text = true;
	CurrentFileName = wxT("");
	UpdatePanelButtonText();
	ResetAllArrays();
	Refresh();
}

void MainFrame::OnPan2(wxCommandEvent& event)
{
	switch(suMode)
	{
		//Start Again
		case 0:
			if ( ! ConfirmBox(wxString(wxT("start again with a blank page"))) )
				return;
			Reset();
			break;
		case 1:
			if ( ! ConfirmBox(wxString(wxT("start this puzzle again from scratch"))) )
				return;
			ResetPlayArrays();
			Refresh();
			break;
		case 2:
			DrawCheckDialogue();
			break;
		default:
			break;
	}
	UpdatePanelButtonText();
}

void MainFrame::DrawCheckDialogue(void)
{
	bool found = CheckAnswer();
	int answer;

	if (found == true)
		wxMessageBox("Looks good so far!", 
			wxT("Sudoku Game Solver Generator"), 
			wxOK, this);
	else
		answer = wxMessageBox("Whoops! There's a problem. Would you like me to show you the errors?", 
			wxT("Sudoku Game Solver Generator"), 
			wxYES_NO | wxICON_ERROR, this);
	if (answer == wxYES)
	{
		HighlightErrors();
		Refresh();
	}
	else
	{
		wxMessageBox("That's the spirit child, grin through the pain", 
			wxT("Sudoku Game Solver Generator"), 
			wxOK, this);
	}

}

bool MainFrame::CheckAnswer(void)
{
	int i, j, k;
	int input_array[9][9][10];

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<10; k++)
			{
				if (k == 0)
					input_array[i][j][k] = suOriginalValues[i][j];
				else
					input_array[i][j][k] = suButModes[i][j][k];
			}

	SudokuPuzzle check_puzzle(input_array);
	check_puzzle.SolvePuzzle(0xff);

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			if (check_puzzle.grid_cache[i][j][0] != suValues[i][j]
			    && suValues[i][j] > 0
				&& suValues[i][j] != suOriginalValues[i][j])
				{
					return false;
				}
			else if (suButModes[i][j][(check_puzzle.grid_cache[i][j][0])] == 2)
			{
				return false;
			}

	return true;
}


void MainFrame::HighlightErrors()
{
	int i, j, k;
	int input_array[9][9][10];

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<10; k++)
			{
				if (k == 0)
					input_array[i][j][k] = suOriginalValues[i][j];
				else
					input_array[i][j][k] = suButModes[i][j][k];
			}

	SudokuPuzzle highlight_puzzle(input_array);
	highlight_puzzle.SolvePuzzle(0xff);

	highlight_ac = 0;
	
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			if (highlight_puzzle.grid_cache[i][j][0] != suValues[i][j]
			    && suValues[i][j] > 0
				&& suValues[i][j] != suOriginalValues[i][j])
			{
				HighlightSquares(i, j, i, j);
			}
			else if (suButModes[i][j][(highlight_puzzle.grid_cache[i][j][0])] == 2)
			{
				HighlightSquares(i, j, i, j);
			}
}

bool MainFrame::ConfirmBox(wxString action)
{
	int answer;
	wxString contents;
	contents.Printf(wxT("Are you sure you want to "));
	contents.append(action);
	contents.append(wxString(wxT("?")));
	answer = wxMessageBox(contents, 
			wxString(wxT("Sudoku Game Solver Generator")),
			wxYES_NO, this);

	if ( answer == wxYES )
		return true;
	else
		return false;
}

int MainFrame::VerifySolvable( int new_mode )
{
	int solved;
	int diff=-1, i, j, k;
	int answer;
	wxString str;

	int input_array[9][9][10];

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<10; k++)
			{
				if (k == 0)
					input_array[i][j][k] = suValues[i][j];
				else
					input_array[i][j][k] = suButModes[i][j][k];
			}

	SudokuPuzzle verify_puzzle(input_array);
	solved = verify_puzzle.SolvePuzzle(0xff);

	if (solved == 0)
	{
		suMode = new_mode;
		update_text = true;
		if (suMode == 1)
		{
			diff = verify_puzzle.PuzzleDifficulty;
			str.Printf(wxT("Puzzle appears solvable (detected as having a difficulty of\n'"));
			str.append(difficulties[diff]);
			str.append(wxT("') - Timer is now going, Good Luck!"));
			wxMessageBox(str, 
				wxT("Sudoku Game Solver Generator"),
				wxOK, this);
		}
		else if (suMode == 2)
		{
			wxMessageBox(wxString(wxT("Puzzle appears solvable - Now entering tutorial mode, Good Luck!")), 
				wxT("Sudoku Game Solver Generator"),
				wxOK, this);
		}
		int i, j;
		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
				if (suModes[i][j] == 1)
					suModes[i][j]++;
		timer_val=0;
		Refresh();
	}
	else if (solved == 1)
	{
		answer = wxMessageBox("Error: The Puzzle Entered Does not appear to be solvable with pure logic. Are you sure you want to use this puzzle?", 
			wxT("Sudoku Game Solver Generator"), 
			wxYES_NO | wxICON_WARNING, this);
		if (answer == wxYES)
		{
			suMode = new_mode;
			update_text = true;
			wxMessageBox(wxString(wxT("On Your Head be it!")), 
				wxT("Sudoku Game Solver Generator"),
				wxOK, this);
			int i, j;
			for (i=0; i<9; i++)
				for (j=0; j<9; j++)
					if (suModes[i][j] == 1)
						suModes[i][j]++;
			timer_val=0;
			Refresh();
		}
	}
	else if (solved == 2 || solved == 3)
	{
		wxMessageBox(wxString(wxT("Error: The Puzzle Entered is impossible to solve, \
								  \nare you sure you entered it correctly? Try again")), 
			wxT("Sudoku Game Solver Generator"), 
			wxOK | wxICON_ERROR, this);
	}

	if ( suMode == 1 )
		SetOriginalValues();

	return diff;

}

void MainFrame::SetOriginalValues(void)
{
	int i, j;
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			suOriginalValues[i][j] = suValues[i][j];
}

void MainFrame::RevealSolution(void)
{
	int i, j, k;
	int input_array[9][9][10];

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<10; k++)
			{
				if (k == 0)
					input_array[i][j][k] = suOriginalValues[i][j];
				else
					input_array[i][j][k] = suButModes[i][j][k];
			}

	SudokuPuzzle reveal_puzzle(input_array);
	reveal_puzzle.SolvePuzzle(0xff);

	
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
		{
			if (suModes[i][j] < 2)
			{
				suValues[i][j] = reveal_puzzle.PuzzleSolution[i][j];
				suModes[i][j] = 1;
			}
		}
	update_text = true;
}

void MainFrame::OnPan3(wxCommandEvent& event)
{
	switch(suMode)
	{

		//Finish Selection
		case 0:
			PuzzleDifficulty = VerifySolvable(1);
			break;
		case 1:
			if ( ! ConfirmBox(wxString(wxT("enter tutorial mode and abandon your possible rating"))) )
				return;
			suMode = 2;
			update_text = true;
			Refresh();
			break;
		case 2:
			if ( ! ConfirmBox(wxString(wxT("reveal the solution and lose all your hard work"))) )
				return;
			RevealSolution();
			Refresh();
			break;
		default:
			break;
	}
	UpdatePanelButtonText();

}


int MainFrame::GenerateRandomNumber(int lower, int upper)
{
	// Generate Random Number between lower and upper inclusive
	int limit, initial;
	// get the floor
	limit = upper - lower + 1;
	initial = rand() % limit;
	return (initial + lower);
}


void MainFrame::OnPan4(wxCommandEvent& event)
{
	wxString str, new_str;
	int i, j, k;
	int input_puzzle[9][9][10];

	switch(suMode)
	{
		//Load Puzzle
		case 0:
			LoadPuzzle();
			break;
		// Minimise Window/Pause
		case 1:
			this->Iconize(true);
			break;
		case 2:
			suHint = true;
			for (i=0; i<9; i++)
				for (j=0; j<9; j++)
				{
					input_puzzle[i][j][0] = suValues[i][j];
					for (k=1; k<10; k++)
					{
						if (suButModes[i][j][k] == 2)
						{
							input_puzzle[i][j][k] = 2;
						}
						else
						{
							input_puzzle[i][j][k] = 0;
						}
					}
				}
			SudokuPuzzle hint_puzzle(input_puzzle);

			HintString = hint_puzzle.ProvideHint();
			if (HintString != wxEmptyString)
			{
				highlight_ac = 0;
				for (i=0; i<3; i++)
				{
					if (hint_puzzle.HintSquares[i][4] == 1)
					{
						HighlightSquares(hint_puzzle.HintSquares[i][0], hint_puzzle.HintSquares[i][1],
							hint_puzzle.HintSquares[i][2], hint_puzzle.HintSquares[i][3]);
					}
				}
				update_text = true;
				Refresh();

			}
			break;
	}

	UpdatePanelButtonText();
}

void MainFrame::OnRightPress(wxMouseEvent& event)
{
	if(suDisabled)
		return;

	wxPoint mouse_rel = ScreenToClient(wxGetMousePosition());
	wxRect ref_rect;
	int row = GetRow(mouse_rel);
	int col = GetCol(mouse_rel);
	bool change;
	int i, pos=0, pos_value;
	int entry = GetEntry(mouse_rel, row, col);
	int current_mode = suButModes[col][row][entry];
	if (SudGridPressCheck(mouse_rel))
	{
		if (suMode >= 1 && suModes[col][row] < 2)
		{
			if (current_mode == 0)
				suButModes[col][row][entry] = 2;
			else if (current_mode == 1)
				suButModes[col][row][entry] = 0;
			else if (current_mode == 2)
				suButModes[col][row][entry] = 1;

			if (suButModes[col][row][entry] != current_mode)
				change = true;

			for (i=1; i<=9; i++) 
				if (suButModes[col][row][i] < 2)
				{
					pos++;
					pos_value=i;
				}
			// If they are all X except 1
			if (pos == 1)
			{
				suModes[col][row] = 1;
				suValues[col][row] = pos_value;
			}

		}
		if (change)
		{
			ref_rect = wxRect(suSquareCoOrd[col][row], suSquareSize);
			Refresh(true, &ref_rect);
			CheckComplete();
		}
	}
	
}

int MainFrame::RandNum( void )
{
	return (GenerateRandomNumber(1, 9));
}

int MainFrame::GeneratePuzzle(int difficulty)
{
	int i, j, k;
	wxString str;

	Reset();
	
	str.Printf(wxT("Patience, child - I'm generating your \""));
	str.append(difficulties[difficulty]);
	str.append(wxT("\" puzzle..."));
	SetStatusText(str, 0);

	SudokuPuzzle generated_puzzle;
	generated_puzzle.GeneratePuzzle(difficulty);

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
		{
			suValues[i][j] = generated_puzzle.GeneratedPuzzle[i][j];
			suModes[i][j] = 1;
			if (suValues[i][j] == 0)
				suModes[i][j] = 0;
		}
	str.Printf(wxT("Your \""));
	str.append(difficulties[difficulty]);
	str.append(wxT("\" puzzle is ready to go - Click on \"Start this Puzzle\"..."));
	SetStatusText(str, 0);

	return 1;
}



int MainFrame::CharToInt(char num)
{
	int ascii_num = (int)num;
	int return_num = ascii_num - 48;
	return return_num;
}

bool MainFrame::LoadFile(wxString FileName)
{
	int size=891, l=0, i=0, j=0, k=0;
	long int tmp;
	wxXmlDocument doc;
	wxXmlNode *level1;
	wxString all_val_str;
	wxString timer_dump;
	wxString mode_dump;
	wxString default_str = wxString(wxT(""));
	wxString debug_string;
	wxString base_name;
	wxString str;
	wxFileName file_name(FileName, wxPATH_NATIVE);
	
	base_name = file_name.GetFullName();
	str = suCommon::default_title_text;
	str.Append(" - ");
	str.Append(base_name);
	this->SetTitle(str);
	str.Clear();

	doc.Load(FileName);
	// start processing the XML file
	if (doc.GetRoot()->GetName() != wxT("suSave"))
		return false;

	for (i=0; i<size; i++)
		default_str.append("0");

	// Level1 is the highest in the tree = level 2 is a child etc.
	level1 = doc.GetRoot()->GetChildren();
	while (level1) {
		// this is the highscores stanza
		if (level1->GetName() == wxT("puzzledef"))
		{
			all_val_str = level1->GetPropVal(wxString(wxT("grid_dump")), default_str);
			timer_dump = level1->GetPropVal(wxString(wxT("timer_dump")), wxString(wxT("0")));
			mode_dump = level1->GetPropVal(wxString(wxT("mode_dump")), wxString(wxT("0")));
		}
		level1 = level1->GetNext();
	}
	
	timer_dump.ToULong(&timer_val);
	mode_dump.ToLong(&tmp);
	suMode = (int)tmp;

	for (i=0; i<size; i=(i+11))
	{
		all_val_str.GetChar(i);
		suModes[j][k] = CharToInt(all_val_str.GetChar(i));
		suValues[j][k] = CharToInt(all_val_str.GetChar(i+1));
		if (suModes[j][k] == 2 || suMode == 0)
			suOriginalValues[j][k] = suValues[j][k];

		for (l=1; l<10; l++)
		{
			suButModes[j][k][l] = CharToInt(all_val_str.GetChar(i+1+l));
		}

		j++;

		if (j == 9)
		{
			j = 0;
			k++;
		}
	}

	update_text = true;

	Refresh();
	return true;
}

bool MainFrame::SaveFile(wxString FileName)
{
	int i, j, k;
	wxXmlDocument doc;
	wxXmlNode *level1;
	wxString grid_dump;
	wxString timer_dump;
	wxString mode_dump;
	wxString save_template;
	wxString ContentStr;
	wxString str;
	wxFFile file_obj;

	// Write blank stanza to file
	file_obj.Open(FileName, wxT("w"));
	save_template.Printf("<suSave>\n  <puzzledef></puzzledef>\n</suSave>");
	file_obj.Write(save_template);
	file_obj.Close();

	grid_dump.Clear();

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
		{
			str.Printf("%d", suModes[j][i]);
			grid_dump.append(str);
			str.Printf("%d", suValues[j][i]);
			grid_dump.append(str);
			for (k=1; k<10; k++)
			{
				str.Printf("%d", suButModes[j][i][k]);
				grid_dump.append(str);
			}
		}

	timer_dump.Printf("%d", timer_val);
	mode_dump.Printf("%d", suMode);

	doc.Load(FileName);
	// start processing the XML file
	if (doc.GetRoot()->GetName() != wxT("suSave"))
		return false;

	// Level1 is the highest in the tree = level 2 is a child etc.
	level1 = doc.GetRoot()->GetChildren();
	while (level1) {
		// this is the highscores stanza
		if (level1->GetName() == wxT("puzzledef"))
		{
			level1->AddProperty(wxString(wxT("grid_dump")), grid_dump);
			level1->AddProperty(wxString(wxT("timer_dump")), timer_dump);
			level1->AddProperty(wxString(wxT("mode_dump")), mode_dump);
		}
		level1 = level1->GetNext();
	}

	if (!doc.Save(FileName))
		return false;

	return true;
}

bool MainFrame::CheckComplete(void)
{
	int i, j, k, pos, cutoff_hour;
	int input_array[9][9][10];
	double start_score, second_penalty, time_now;
	bool solved = true, high_enough = false;
	wxString prompt, title;
	// initialize the caches
	unsigned long int HighScoreVal_c[10], final_score;
	wxString HighScoreDate_c[10];
	wxString str;
	wxString HighScoreName_c[10];
	wxString time_date_str;
	wxString date_str;
	wxString time_str;
	wxString user_name;
	wxDateTime now = wxDateTime::Now();
	int wxMyDefault = wxOK;

	time_str = now.FormatISOTime();
	date_str = now.FormatISODate();
	time_date_str.append(time_str);
	time_date_str.append(", ");
	time_date_str.append(date_str);

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			if (suValues[i][j] == 0)
				return false;

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<10; k++)
			{
				if (k == 0)
					input_array[i][j][k] = suOriginalValues[i][j];
				else
					input_array[i][j][k] = suButModes[i][j][k];
			}

	SudokuPuzzle check_puzzle(input_array);
	check_puzzle.SolvePuzzle(0xff);
	
	// Only gets this far if all the suValues entries contain a value
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			if ( suValues[i][j] != check_puzzle.PuzzleSolution[i][j] )
				solved = false;

	for (i=0; i<10; i++)
	{
		// populate the caches with the original values
		HighScoreVal_c[i] = HighScoreVal[i];
		HighScoreDate_c[i] = HighScoreDate[i];
		HighScoreName_c[i] = HighScoreName[i];
	}

	if (solved)
	{
		prompt.Printf("My you ARE impressive! The puzzle has been completed\ncorrectly - Congratulations!");
		title.Printf("Sudoku Game Solver Generator");
	}
	else
	{
		prompt.Printf("Sorry - you have completed all of the squares but \nsome of them are incorrect - try again...");
		title.Printf("Sudoku Game Solver Generator");
		wxMyDefault = wxOK | wxICON_ERROR;
	}

	if (suMode == 1 && solved && PuzzleDifficulty >= 0)
	{
		// Calculate a score based on this difficulty and solution
		// start_score is an impossible-to-reach value which
		// is then factored according to how long the puzzle took
		// to complete
		start_score = (PuzzleDifficulty + 3) * (double)1000000;

		// cutoff_hour is the number of hours after which a puzzle will score zero
		// will be in the range 3-14 hours, depending on difficulty
		cutoff_hour = (PuzzleDifficulty + 1) * 2;

		if (timer_bd[2] >= cutoff_hour)
			final_score = 0;
		else
		{
			// This is the number of points to be taken away from start_score with
			// each second it took to complete the puzzle
			second_penalty = (start_score / ((double)cutoff_hour * (double)3600));
			final_score = (long unsigned int)(start_score - ( (double)timer_val * second_penalty ));
			if (final_score < 0)
				final_score = 0;
		}

		str.Printf("\n\nYou completed a '");
		str.append(difficulties[PuzzleDifficulty]);
		prompt.append(str);
		str.Printf("' puzzle in \n\n%d hours, %d minutes and %d seconds\n\nwhich gives you a score of %d",
			timer_bd[2], timer_bd[1], timer_bd[0], final_score);
		prompt.append(str);
	}
	

	if (suMode == 1 && solved && PuzzleDifficulty >= 0)
	{
		
		// See if this score is high enough to be in the High Scores list,
		// and fill pos with the position it if going to inherit

		pos = -1;
		for (i=0; i<10; i++)
			if (final_score > HighScoreVal[i] && !high_enough)
			{
				high_enough = true;
				pos = i;
				HighScoreDate[i] = time_date_str;
				HighScoreVal[i] = final_score;
				HighScoreName[i].Printf("");
			}
			else if (high_enough && pos >= 0 && i < 9)
			{
				HighScoreVal[i+1] = HighScoreVal_c[i];
				HighScoreDate[i+1] = HighScoreDate_c[i];
				HighScoreName[i+1] = HighScoreName_c[i];
			}
	}
	
	wxMessageBox(prompt, title, wxMyDefault, this);


	// If it is high enough, prompt user for their name
	if (high_enough)
	{
		str.Printf("Congratulations - you have made the Dojo of Fame (%d)! \nEnter your name below", (pos + 1));
		user_name = wxGetTextFromUser(str, 
			wxString(wxT("Dojo of Fame Name Entry")),
			wxString(wxT("Undisclosed")), this);

		if (user_name == wxString(wxT("")))
			user_name.Printf("Undisclosed");
		
		HighScoreName[pos] = user_name;

		if (!WriteHighScores())
			DebugString(wxString(wxT("FAILED to write to config file")));
	}
	// Write this information out to the XML file

	return true;
}

void MainFrame::OnLeftPress(wxMouseEvent& event)
{
	if(suDisabled)
	{
		DebugString(wxString("Exiting as currently disabled"));
		return;
	}

	wxPoint mouse_rel = ScreenToClient(wxGetMousePosition());
	wxRect rect;
	bool change;
	int row = GetRow(mouse_rel);
	int col = GetCol(mouse_rel);
	int entry = GetEntry(mouse_rel, row, col);
	int current_mode = suButModes[col][row][entry];
	if (SudGridPressCheck(mouse_rel))
	{
		update_text = true;
		if (suModes[col][row] == 0 && suMode == 0)
		{
			// If we are in entry mode and the button has been pressed,
			// set the whole square to that value
			suModes[col][row] = 1;
			suValues[col][row] = entry;
			change = true;
		}
		else if (suModes[col][row] == 1 && entry == 1)
		{
			// If we're in entry mode, the button has a value, and the
			// x has been pressed, revert the square to it's default
			change = true;
			OnUndo();
		}
		else if (suMode >= 1 && suModes[col][row] == 0)
		{
			suModes[col][row] = 1;
			suValues[col][row] = entry;
			if (PuzzleSolution[col][row] == suValues[col][row] && suNetPlay)
			{
				suComplete++;
			}
			change = true;
		}
	}
	if (change == true)
	{
		rect = wxRect(suSquareCoOrd[col][row], suSquareSize);
		Refresh(true, &rect);
		CheckComplete();
	}
}

void MainFrame::Debug(void)
{
		wxString str;
		str.Printf("The debug function was reached");
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
}

void MainFrame::DebugString(wxString str)
{
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
}

void MainFrame::DebugChar(char chr_name)
{
		wxString str;
		str.Printf("%c", chr_name);
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
}

void MainFrame::DebugInt(int int_name)
{
		wxString str;
		str.Printf("%d", int_name);
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
}

void MainFrame::DebugLong(long int_name)
{
		wxString str;
		str.Printf("%d", int_name);
		wxMessageBox(str, wxT("Sudoku Game Solver Generator"), wxOK, this);
}


void MainFrame::OnUndo()
{
	wxPoint mouse_rel = ScreenToClient(wxGetMousePosition());
	wxRect rect;
	int row = GetRow(mouse_rel);
	int col = GetCol(mouse_rel);
	if (suModes[col][row] == 1)
		suModes[col][row] = 0;
	suValues[col][row] = 0;
	rect = wxRect(suSquareCoOrd[col][row], suSquareSize);
	Refresh(true, &rect);
	//SetStatusTime();
}

wxString MainFrame::GenerateConfig(void)
{
	int i;
	wxString str;

	str.Printf("<suConfig>\n  <highscores>\n");
	for (i=0; i<10; i++)
		str.append("    <score date=\"\" value=\"0\" name=\"\"/>\n");
	str.append("  </highscores>\n  <default>\n    <font/>\n");
	str.append("    <font_colour/>\n    <grid_colour/>\n    <gb_colour/>\n");
	str.append("    <gb_font_colour/>\n    <bg_colour/>\n    <title_colour/>\n");
	str.append("    <netplay_port/>\n");
	str.append("  </default>\n</suConfig>");

	return str;

}


void MainFrame::OnAbout(wxCommandEvent& event)
{
	wxString msg, str;

	msg.Printf(wxT("Sudoku Game Solver Generator "));
	msg.append(suCommon::suVersion);
	str.Printf(wxT(",\ncreated using %s."), wxVERSION_STRING);
	msg.append(str);
	msg.append(wxT("\n\nCopyright 2017-2018 Open Source Developer Federica Domani.\nhttps://federicadomani.wordpress.com\nSpecial thanks to Frank Quinn, Queen's University of Belfast."));
	wxMessageBox(msg, wxT("About Sudoku Game Solver Generator"), wxOK, this);
}

void MainFrame::SaveScheme(void)
{
	wxXmlDocument doc;
	wxXmlNode *level1;
	wxXmlNode *level2;
	wxString str;

	if (!doc.Load(suConfigFile))
	{
		DebugString(wxString("Unable to open file"));
		return;
	}
	if (doc.GetRoot()->GetName() != wxT("suConfig"))
	    return;

	
	// Level1 is the highest in the tree = level 2 is a child etc.
	level1 = doc.GetRoot()->GetChildren();
	while (level1) {
		// this is the highscores stanza
		if (level1->GetName() == wxT("default"))
		{
			level2 = level1->GetChildren();
			while (level2)
			{
				if (level2->GetName() == wxT("bg_colour"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("value"));
					str = suBGColour.GetAsString(wxC2S_HTML_SYNTAX);
					level2->AddProperty(wxT("value"), str);
				}
				if (level2->GetName() == wxT("font_colour"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("value"));
					str = suDefaultFontColour.GetAsString(wxC2S_HTML_SYNTAX);
					level2->AddProperty(wxT("value"), str);
				}
				if (level2->GetName() == wxT("grid_colour"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("value"));
					str = suGridColour.GetAsString(wxC2S_HTML_SYNTAX);
					level2->AddProperty(wxT("value"), str);
				}
				if (level2->GetName() == wxT("title_colour"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("value"));
					str = suTitleColour.GetAsString(wxC2S_HTML_SYNTAX);
					level2->AddProperty(wxT("value"), str);
				}
				if (level2->GetName() == wxT("gb_colour"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("value"));
					str = suGBColour.GetAsString(wxC2S_HTML_SYNTAX);
					level2->AddProperty(wxT("value"), str);
				}
				if (level2->GetName() == wxT("gb_font_colour"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("value"));
					str = suGBFontColour.GetAsString(wxC2S_HTML_SYNTAX);
					level2->AddProperty(wxT("value"), str);
				}
				if (level2->GetName() == wxT("font"))
				{
					// Delete all properties
					level2->DeleteProperty(wxT("point"));
					level2->DeleteProperty(wxT("family"));
					level2->DeleteProperty(wxT("style"));
					level2->DeleteProperty(wxT("weight"));
					level2->DeleteProperty(wxT("underlined"));
					level2->DeleteProperty(wxT("face"));
					str.Printf("%d", suDefaultFont.GetPointSize());
					level2->AddProperty(wxT("point"), str);
					str.Printf("%d", suDefaultFont.GetFamily());
					level2->AddProperty(wxT("family"), str);
					str.Printf("%d", suDefaultFont.GetStyle());
					level2->AddProperty(wxT("style"), str);
					str.Printf("%d", suDefaultFont.GetWeight());
					level2->AddProperty(wxT("weight"), str);
					if (suDefaultFont.GetUnderlined())
						level2->AddProperty(wxT("underlined"), wxT("true"));
					else
						level2->AddProperty(wxT("underlined"), wxT("false"));
					level2->AddProperty(wxT("face"), suDefaultFont.GetFaceName());
				}
				level2 = level2->GetNext();
			}
		}
		level1 = level1->GetNext();
	}
	if (!doc.Save(suConfigFile))
		return;

}

void MainFrame::OnSaveScheme(wxCommandEvent& event)
{
	SaveScheme();
	return;

}

void MainFrame::OnResetScheme(wxCommandEvent& event)
{
	if ( ! ConfirmBox(wxString(wxT("Reset to defaults and lose your current colour scheme"))) )
		return;
	wxFont tmp_font(suFONT_SIZE, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL, false);
	suDefaultFont = tmp_font;
	suDefaultFont.SetPointSize(8);
	suDefaultFont.SetFamily(wxFONTFAMILY_SWISS);
	suDefaultFont.SetWeight(wxFONTWEIGHT_NORMAL);
	suDefaultFont.SetStyle(wxFONTSTYLE_NORMAL);
	suDefaultFontColour = wxColour(wxT("BLACK"));
	suGridColour = wxColour(255, 239, 213);
	suGBColour = wxColour(wxT("NAVY"));
	suGBFontColour = wxColour(wxT("RED"));
	suBGColour = wxColour(wxT("WHITE"));
	suTitleColour = wxColour(wxT("NAVY"));
	update_text = true;
	Refresh();
	SaveScheme();
}

void MainFrame::OnQuit(wxCommandEvent& event)
{
	Socket->Destroy();
	Destroy();
}

bool suPrint::OnPrintPage(int page)
{
	suCommon::suPrintEnabled = true;
    wxDC *dc = GetDC();
	wxSize printer_resolution = dc->GetSize();
	wxString str;
	wxImage prev_img, new_img;
	wxBitmap prev_bmp;
	float x_scale, y_scale, p_scale;
	int p_width, p_height;

	GetPageSizePixels(&p_width, &p_height);

	x_scale = (float)(printer_resolution.GetWidth())/(float)(suCommon::InternalSize.GetWidth());
	y_scale = (float)(printer_resolution.GetHeight())/(float)(suCommon::InternalSize.GetHeight());

	if (x_scale > y_scale)
		p_scale = y_scale;
	else
		p_scale = x_scale;


	prev_img = suCommon::bitmap.ConvertToImage();

	prev_bmp = wxBitmap(prev_img.Rotate90(true));
    FitThisSizeToPage(wxSize(prev_bmp.GetWidth(), prev_bmp.GetWidth()));

	dc->SetBackground(*wxWHITE_BRUSH);
	dc->Clear();
	dc->DrawBitmap(prev_bmp, 0, 0);

	if (dc)
	{
        return true;
    }
    else
        return false;
}

bool suPrint::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;

    return true;
}

void suPrint::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool suPrint::HasPage(int pageNum)
{
    return (pageNum == 1);
}

