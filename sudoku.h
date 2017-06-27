/*! \mainpage Documentation for Sudoku Game Solver Generator
 */
#include "common.h"

#define SUDOKU_VERSION_STRING "1.0.0"

/// This is the class which wxWidgets needs to IMPLEMENT in order
/// to provide the main entry point in the application (wxWidgets
/// applications do not provide the user with a main() entry point
/// by default, instead, a class derived from wxApp is initiated
/// and this can be called by the native wxWidgets IMPLEMENT_APP
/// macro). This class in turn generated the MainFrame class.
class BasicApplication : public wxApp
{
public:
	/// This is a pointer to the MainFrame wxWindow which is central
	/// to the remainder of the application
	int frame;
	/// This function provides the API with instructions on what to do at
	/// load time
	virtual bool OnInit();
};

/// This is a common namespace designed to be accessable from all classes
/// to act as a relay point for functions that are restricted (due to the
/// class they were derived from) as being private functions, or accessing
/// private functions
namespace suCommon
{
	wxString default_title_text;
	/// This is an array of strings containing the descriptions of the various
	/// solving methods, mainly used in the ProvideHint() function.
	wxString desc_string[5];
	/// This wxString array contains the values of the entries of the rows (1-9) so
	/// they can be referenced and re-named to anything we want at a later point.
	wxString row_name[9];
	/// This wxString array contains the values of the entries on the column (A-I)
	/// they can be referenced and re-named to anything we want at a later point.
	wxString col_name[9];
	/// wxMemoryDC to act as a bitmap cache for relaying wxDC objects between
	/// different classes
	wxMemoryDC *suDC;
	/// This string represents the current version of the software
	wxString suVersion;
	/// This string represends the first argument passed to the program - means
	/// the application can accept one command line parameter.
	wxString InputFile;
	/// wxSize representing the entire size of the suDC context
	wxSize InternalSize(720, 530);
	/// wxBitmap representing the sizez of the temp memory bitmap written to
	/// and referenced through wxMemoryDC
	wxBitmap bitmap;
	/// Boolean representing whether a print screen has been enabled or not, to
	/// prevent corruption of the suDC context with the print menu (it shouldn't
	/// happen but it does).
	bool suPrintEnabled = false;
}


/// This is the class which provides a means through which to print. wxWidgets
/// cannot access the printer unless a class such as this which is derived from
/// wxPrintout is enabled. Print Previews are also handled by this class and
/// is through this class that print previews can be generated and the
/// print format can be manipulated.
class suPrint : public wxPrintout
{
public:
	/// Constructor for the wxPrintout object for allowing the program to 
	/// interface with the printer the printer
	suPrint(const wxChar *title = _T("Sudoku Game Solver Generator printout")):wxPrintout(title) {}
	/// Overriding wxPrintout default function to handle what to do when a print
	/// request is initialized.
	bool OnPrintPage(int page);
	/// Overriding wxPrintout default function to handle whether specified page is
	/// valid.
	bool HasPage(int page);
	/// Overriding wxPrintout default function to handle what to do when a printer
	/// context is being initialized.
	bool OnBeginDocument(int startPage, int endPage);
	/// Overridding wxPrintout default function to handle putting the properties
	/// of the printout onto the wxPrintout class.
	void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};


/// This is the class derived from wxFrame which contains almost all of the sudoku 
/// functions
class MainFrame : public wxFrame
{

/// Public functions of MainFrame class
public:
	/// MainFrame Constructor
	MainFrame( const wxString title, wxPoint origin, wxSize size );
	/// MainFrame Destructor
	~MainFrame();

/// Private functions of MainFrame class
private:
	// +-------------------------------------------------------------+
	// |                  Debugging Functions                        |
	// +-------------------------------------------------------------+
	//
	void DebugLong(long int_name);
	/// Handy little debug function for development purposes only
	/// which outputs a prompt box	
	void Debug(void);
	/// Handy little debug function for development purposes only
	/// which outputs a prompt box printinf a wxString
	void DebugString(wxString str);
	/// Handy little debug function for development purposes only
	/// which outputs a prompt box printing an integer
	void DebugInt(int int_name);
	/// Handy little debug function for development purposes only
	/// which outputs a prompt box printing an Char
	void DebugChar(char chr_name);

	// +-------------------------------------------------------------+
	// |                 Class Global Variables                      |
	// +-------------------------------------------------------------+
	//
	/// This is the static text object for the status text in the top right
	/// of the application. It only gets update when the update_text flag
	/// is raised
	wxStaticText StatusText;
	/// This is the static text object for the sensei text in the bottom right
	/// of the application. It only gets update when the update_text flag
	/// is raised
	wxStaticText StaticSenseiText;
	/// This is the text control object for the status text in the bottom right
	/// of the application. This replaces StaticSenseiText only during net play
	/// to give scrolling capabilities for logs/chat etc.
	wxTextCtrl SenseiText;
	/// This is the client socked for reading to and writing to while acting as
	/// a client.
	wxSocketClient* Socket;
	/// This is a complete copy of the sudoku solution as generated by SolvePuzzle(0x0f)
	int PuzzleSolution[9][9];
	/// Due to threading, we need to know during netplay whether or not the application
	/// is already dealing with the consequences of a certain flag being raised - this
	/// flag lets us know this.
	bool suProcessing;
	/// This flag is raised during netplay when the current game should be declared as over
	bool suGameOver;
	/// This is a boolean representing whether or not entry in the sudoku grid is enabled
	bool suDisabled;
	/// This is the epoch time representing when the application started (so we can check for
	/// an update after 5 seconds have elapsed - for some reason the http function don't work
	/// until after the OnPaint event is completed.
	long epoch_start;
	/// This is the location of the configuration file for the software, holding information
	/// life default colours and high scores.
	wxString suConfigFile;
	/// suServerMode=true for server, false for client during netplay mode
	bool suServerMode;
	/// true if net play is currently running
	bool suNetPlay;
	/// Whether or not we are in netplay mode (which accompanies play mode)
	bool suNetChange;
	// represents if something has just changed which will need
	// to be communicated with the server/client
	bool suChange;
	bool suConnectionEstablished;
	/// This is where the directory of the config file is stored.
	wxString home_dir;
	/// This is an integer 0-6 representing the difficulty of the puzzle being
	/// played at the moment.
	int PuzzleDifficulty;
	/// This is the socket to be read and written to during netplay by the server
	wxSocketServer *m_server;
	/// This is the temporary working area which is constantly written to when working
	/// on solving or finding hints for a puzzle. grid_cache[col][row][0] = value, 
	/// grid_cache[col][row][1-9] = suButModes
	int grid_cache[9][9][10];
	/// Represents whether or nit the timer is disabled
	bool timer_enabled;
	/// This represents the colour of the background
	wxColour suBGColour;
	/// This represents the default font in the application
	wxFont suDefaultFont;
	/// This represents the colour of the default font in the application
	wxColour suDefaultFontColour;
	/// This represents the colour of the colour of the sudoku grid buttons
	wxColour suGridColour;
	/// This represents the colour of the main title at the top of the application
	wxColour suTitleColour;
	/// This is a wxTimer object required to keep track of the clock in the application.
	/// The timer can be initialized manually, then the wxWidgets lib generates
	/// events which can update the screen or increment registers accordingly.
	wxTimer *m_timer;
	/// Status text only referenced during netplay as appended to with text and system
	/// notices (like alerting the user when they have just had squares blown away!
	wxString suNetSenseiString;
	/// Buffer of all the highlight sizes (for ProvideHint)
	wxSize highlight_sz[100];
	/// Buffer of all the highlight points (for ProvideHint)
	wxPoint highlight_pt[100];
	/// Number representing how many of the highlight_* array elements are actually useful
	int highlight_ac;
	/// Whether or not a hint is currently being requested
	bool suHint;
	/// This is a string representing the chat string which you currently are
	/// queuing during netplay for next synchronising with the other machine
	wxString suChatText;
	/// This is an array of strings which constitute descriptions of them
	wxString difficulties[7];
	/// This is a string provided in tutorial mode which can tell the user
	/// exactly what their next step should be
	wxString HintString;
	/// The number of completed squares completed in the last second
	int suComplete;
	/// The number of completed squares completed by opponent in last 60 seconds
	int suNetComplete[60];
	/// This is a wxString which will then be translated to a standard c string
	/// for printing out to the socket. 
	wxString BufferData;
	/// This is the name of the server which, during netplay, the client will connect to
	wxString suServerHost;
	/// This is the port to be used during netplay
	int suPort;
	/// Whether or not to proceed with the network game starting attempt
	bool suNetAttempt;
	/// Cache of the mode to determine whether to update button text or not
	int suModeCache;
	/// Boolean for whether to initialise buttons and other entities when
	/// the application is being starte dup for the first time
	bool init_run;
	/// wxSize of the suSquare, neglecting line thicknesses
	wxSize suSquareSize;
	/// wxSize of the size of the small buttons
	wxSize suButSize;
	/// The origin point on the main OnPaint wxPaintDC, default (0, 0)
	wxPoint OriginRedraw;
	/// Variable containing the filename to determine if Save is
	/// Save as is called
	wxString CurrentFileName;
	/// This contains the co-ordinates of each of the suSquares and is called frequently
	/// any time there is a refresh to reduce flicker (by only updating the regions that
	/// will have changed instead of the entire grid).
	wxPoint suSquareCoOrd[9][9];
	/// This is an array corresponding to the curren values of each of the elements
	/// in the sudoku grid. There is one entry here for each of the suSquares.
	int suValues[9][9];
	/// This is a copy of suValues[9][9] at the time which a game entered play
	/// mode so that any entries can be undone at any stage.
	int suOriginalValues[9][9];
	/// 2D array representing the modes of each of the suSquares.
	/// There are three possible modes - entry(0), entered(1) and finalised(2).
	/// When a suSquare is entered, an undo is still available, but when it is
	/// finalized, this option goes away.
	int suModes[9][9];
	/// This is a 3-dimensional integer array which holds information on which of the
	/// small buttons have been eliminated etc. Note that the index of the third
	/// parameter begins with "1" so to determine the possibility of the third row's
	/// fourth column's entry of "5", we look in suButModes[3][2][5].
	/// There are three Possible Button Modes - possible(0), unlikely(1) and impossible(2)
	int suButModes[9][9][9];
	/// The main wxPaintDC as called by the OnPaint function
	wxPaintDC dcp;
	/// wxButton on the right hand panel with dynamic text and appearance
	wxButton *but_pan_1;
	/// wxButton on the right hand panel with dynamic text and appearance
	wxButton *but_pan_2;
	/// wxButton on the right hand panel with dynamic text and appearance
	wxButton *but_pan_3;
	/// wxButton on the right hand panel with dynamic text and appearance
	wxButton *but_pan_4;
	/// The x-co-ordinates of the top-left corner of each of the suSquares
	int x_tl[10];
	/// The y-co-ordinates of the top-left corner of each of the suSquares
	int y_tl[10];
	/// This represents the colour of the group box's outline colour
	wxColour suGBColour;
	/// This represents the colour of the group box's font colour
	wxColour suGBFontColour;
	/// Names of the top ten values of the high scores menu
	unsigned long int HighScoreVal[10];
	/// Names of the top ten dates of the high scores menu
	wxString HighScoreDate[10];
	/// Names of the top ten members of the high scores menu
	wxString HighScoreName[10];
	/// This describes whether or not the text in the groupboxes needs to get updated
	/// (i.e. it has probably just changed).
	bool update_text;
	/// This variable houses the current mode of the game, possibilities and 
	/// corresponding meanings are 0: Entry; 1: Play; 2: Tutorial respectively
	int suMode;
	/// This is an integer storing the game's timer and starts counting from zero as soon
	/// as the suMode is set to Play Mode.
	unsigned long int timer_val;
	/// Breakdown of the timer values:
	/// [0] - Seconds
	/// [1] - Minutes
	/// [2] - Hours
	int timer_bd[3];


	// +-------------------------------------------------------------+
	// |                  GUI Drawing Functions                      |
	// +-------------------------------------------------------------+
	//
	/// This function is triggered after a number is entered, then it needs to
	/// be undone by clicking on the "x" in the top left of the square
	void OnUndo();
	/// This function is for handling text in the Sensei text's Group box. 
	/// The wxString is generated itself within the function.
	void DrawSenseiText(wxPoint SenseiTextOrigin, wxSize SenseiSize);
	/// When a mouse event is triggered, it calls this function to check if the mouse
	/// was on top of the sudoku grid at the time of the click. The function returns
	/// true is the click was over the sudoku grid, false otherwise.
	bool SudGridPressCheck(wxPoint mouse_rel);
	/// This is a function which generates the prompt initially for net play
	void NetPlayMenu(void);
	/// Due to the event-driven nature of wxWidgets and the lack of efficiency
	/// in the way it draws and refreshes pages, I was forced to provide my own
	/// functions to draw my own buttons. It also has the avantage of making the 
	/// buttons look and behave in exactly the same way regardless of the platform 
	/// on which the application is ported to. The function expects an integer "entry"
	/// which corresponds to the value which will be written on the button. The "entry"
	/// variable may also be an ascii character (this is how the little undo "x" boxes
	/// are created when a square is right-clicked. the top_left wxPoint corresponds to
	/// the co-ordinates of the top-left of the button to be drawn. The function also
	/// requires a device context.
	void DrawButton(int entry, wxPoint top_left);
	/// This function will change the text on the face of the navigational
	/// Buttons to correspond to any new meanings they may take on when the
	/// suMode variable changes.
	void UpdatePanelButtonText(void);
	/// This is the main function which administers the drawing of the main
	/// sudoku grid. The grid is drawn to scale in accordance with the parameters
	/// it is passed and the suLINE_THICKNESS #defined variable. 
	void DrawSudokuGrid(wxPoint TopLeft, wxSize GridSize);
	/// This function will draw the Right hand side status boxes and timers,
	/// buttons etc. They are incorporated into their own functions in order
	/// to allow them me moved around if needs be at a later stage.
	void DrawRightPanel(wxPoint TopLeftPan, wxPoint BotRightPan);
	/// This function writes the text inside the program's manually drawn
	/// DrawGroupBox object for the Status box on the top left and prints out
	/// a message relating to the current suMode variable so the user knows
	/// whether he/she is in entry/play/tutorial mode.
	void DrawStatusText(wxPoint StatusTextOrigin, wxSize StatusBoxSize);
	/// The wxWidgets group box looks terrible so I designed this function to provide
	/// the same functionalisy but with a nicer look and feel. The function expects 
	/// the origin and size of the groupbox in terms of co-ordinates on the wxDC context,
	/// and wxString which contains the title of the string. It returns a wxPoint
	/// corresponding to the inner top-left corner where writing inside the GroupBox
	/// can begin.
	wxPoint DrawGroupBox(wxPoint TopLeftBox, wxSize BoxSize, wxString TextTitle);
	/// This function returns the top-left hand corner of the next line based 
	/// on the size of the font and the LineNumber offset and was designed
	/// for use within the DrawGroupBox function.
	wxPoint NextLine(wxPoint CurrentPoint, int FontSize, int LineNumber);
	/// This function dynamically generates the size of the button based on the
	/// Size of the square on the grid which it is a part of, and scales such that
	/// There is no whitespace between buttons and that exactly 9 buttons can fit in
	/// the square. The return type is wxSize.
	wxSize GetButtonSize(wxSize area);
	/// This function is called by DrawSudokuGrid in order to draw the suSquares in
	/// the grid. This function then in turn calls DrawButton etc.
	void DrawSquare(int su_but_height, int su_but_width, int x_pt, int y_pt);
	/// This is a function which returns true if yes is clicked, and false otherwise.
	/// it expects a string which corresponds to Are you sure you want to <wxString action>?
	bool ConfirmBox(wxString action);
	/// This is the prompt to pop up during netplay when the game is over and you have been defeated
	void NetPlayGameOver(void);
	/// This is a prompt asking the user for a difficulty of a puzzle, and setting PuzzleDifficulty 
	/// accordingly
	void GeneratePuzzleScreen(void);
	/// This function simply takes the values in PuzzleSolution[9][9] and puts them out to screen.
	/// It is only accessible in tutorial mode
	void RevealSolution(void);
	/// This takes the values in highlight_* and draws them out to screen highlighted
	void DrawHighlightedSquares(void);
	/// This will go through all of the squares, compare the values with PuzzleSolution,
	/// and highlight the squares which differ
	void HighlightErrors(void);
	/// This will highlight the squares passed to it. If the region contains multiple
	/// squares, a rectangle will be drawn.
	void HighlightSquares(int square1_x, int square1_y, int square2_x, int square2_y);
	/// This screen asks the user which steps they want to skip over whilst in tutorial mode
	void SkipStepsScreen(void);
	/// Function for drawing out the high scores screen
	void DrawHighScores(void);
	/// Picks one of a random few proverbs for dumping out to the sensei text
	wxString GenerateHintProverb(void);
	/// Picks one of a random few proverbs for dumping out to the sensei text
	wxString GenerateEntryProverb(void);
	/// Picks one of a random few proverbs for dumping out to the sensei text
	wxString GeneratePlayProverb(void);
	/// This function checks all of the current choices in a solution and informs
	/// the user if any of them are wrong.
	void DrawCheckDialogue();
	/// This function draws the outer letters onthe grid to the specified
	/// wxDC.
	void DrawOuterLetters(wxPoint GridTopLeft);
	/// This function returns the col in which a click was detected (in the range
	/// of 0-9), or returns 99 in the case of an error.
	int GetCol(wxPoint mouse_rel);
	/// This function returns the row in which a click was detected (in the range
	/// of 0-9), or returns 99 in the case of an error.
	int GetRow(wxPoint mouse_rel);
	/// This function is usually called after a mouse event, and after GetCol()
	/// and GetRow() to determine which entry in the specified square (specified
	/// by the row and the col) has been pressed. The function also expects
	/// the co-ordinates of the mouse relative to the top left corner of the inner-
	/// application. The function returns an integer of values 1-9 accordingly.
	int GetEntry(wxPoint mouse_rel, int row, int col);
	/// This function is for updating the status bar and is called with the OnTimer
	/// event every second to increment the timer in the status bar.
	void SetStatusTime(void);
	/// This function resets the counter and begins incrementing immediately upon
	/// application initialization.
	void InitClock(void);
	/// This is the function for writing out initial text to the status bar at the
	/// bottom of the gui.
	void InitStatusBar(void);
	/// This is a function to populate the top menu bar with entries and include the
	/// event codes which click on each entry represents.
	void DrawMenu(void);


	// +-------------------------------------------------------------+
	// |                 File/Internet Functions                     |
	// +-------------------------------------------------------------+
	//
	/// This function performs the loading of a file's contents directly into the
	/// puzzle's native variables (suModes[9][9], suValues[j][i], suButModes[9][9][9])
	/// as generated by the OnLoad event handler's wxFileSelector dialogue box.
	bool LoadFile(wxString FileName);
	/// This function performs the physical writing out of the variables representing
	/// the entire puzzle (suModes[9][9], suValues[j][i], suButModes[9][9][9]) to the
	/// filename passed to it (which is generated by the SaveFileAs() function dialogue
	/// box or alternatively, if the file has already been loaded or saved, the file
	/// name is obtained via the wxString MainFrame::CurrentFileName.
	bool SaveFile(wxString FileName);
	/// This will take all of the current settings for appearance and sequentially write
	/// the results to the suConfigFile
	void SaveScheme(void);
	/// Called by OnSave - basically just checks whether to perform a quick save or a
	/// SaveAs() function.
	void SaveFileAs(void);
	/// This is the load screen for loading previous attempts at USS puzzles
	void LoadPuzzle(void);
	/// This function is for writing out the chosen network port to file
	void WriteNetPort(int port_no);
	/// This will read in the config file and apply its settings to the current instance
	/// of the program
	bool InitConfigFile(void);
	/// This function will return a default string for a default config file.
	wxString GenerateConfig(void);
	/// This will go online and see if there is a more recent version of the software available
	void CheckUpToDate(void);

	// +-------------------------------------------------------------+
	// |                    Netplay Functions                        |
	// +-------------------------------------------------------------+
	//
	/// This function will write out all of the high scores to the config file
	bool WriteHighScores(void);
	/// This will add a string to the netplay string, with "type" referring to the
	/// source of the string and "wx_in" referring to the accompanying text
	void AddNetString(wxString type, wxString wx_in);
	/// This will write the current chat string to the buffer, ready to send to the
	//// other user.
	void SendChatString(void);
	/// This is the network function called immediately after a connection has been
	/// established.
	void ContinueNetPlay(wxString in_str);
	/// This is the function called to try and trigger a CONNECT event
	bool InitNetPlay(wxString in_str);
	/// This is the basic function for commencing and setting up the server during netplay
	void StartSudServer(void);
	/// This is the basic function for commencing and setting up the client during netplay
	void StartSudClient(void);
	/// This function will generate buffer data based on the current readings for sending
	/// to the other user.
	wxString GenerateBufferData(void);
	/// This function is for decoding the buffer data received from the other machine
	/// and interpreting its findings
	void DecodeBufferData(wxString wx_in);
	/// This function is for dealing out punishment to the other user and it expects
	/// the number of squares the user currently has completed, and the number of squares
	/// which are going to need to be removed
	void PunishUser(int curr_user_comp, int num_removed);

	// +-------------------------------------------------------------+
	// |                      Sudoku Engine                          |
	// +-------------------------------------------------------------+
	//
	/// This function will find the optimal next step for the user in hint mode
	void ProvideHint(void);
	/// This is a useful function which will turn an integer into a wxString containing
	/// the integer's ascii character and is used throughout the program.
	wxString IntToWxString(int var);
	/// This is a simple function to translate a char into it's numeric counterpart
	/// (similar to masking - (char)integerval).
	int CharToInt(char num);
	/// This function is called at the start of the application and when a new puzzle
	/// is beginning - it simply wipes all relevant arrays and gives them 0 values.
	void ResetPlayArrays(void);
	/// This generates a puzzle based on repeatedly picking random numbers,
	/// calling SolvePuzzle(0x0f), sees if it's solvable and of a specific
	/// difficulty, and only leaves the function when it is both.
	int GeneratePuzzle(int difficulty);
	/// This generates a pseudo random number between 1 and 10
	int RandNum( void );
	/// This function resets all of the global variables in the function which
	/// effect the puzzle directly.
	void Reset(void);
	/// This will check of the puzzle is solvable before advancing into the mode
	/// specified, and return the difficulty if it is
	int VerifySolvable( int new_mode );
	/// This is called after every move to check if the solution is both complete
	/// and correct, and if so, it checks whether it constitutes a high score
	bool CheckComplete(void);
	/// This checks if the answer so far is correct or not
	bool CheckAnswer(void);
	/// This returns a pseudo random number between lower and upper
	int GenerateRandomNumber(int lower, int upper);
	/// Copies the current suValues and plants them into suOriginalValues
	void SetOriginalValues(void);
	/// This function is called at the start of the application and when a new puzzle
	/// is beginning - it simply wipes all arrays and gives them 0 values.
	void ResetAllArrays(void);


	// +-------------------------------------------------------------+
	// |                 Event Handling Functions                    |
	// +-------------------------------------------------------------+
	//
	/// Event Handler for File/Exit Menu Item
	void OnQuit(wxCommandEvent& event);
	/// Event Handler for File/Save Menu Item
	void OnSave(wxCommandEvent& event);
	/// Event Handler for File/Save As... Menu Item
	void OnSaveAs(wxCommandEvent& event);
	/// Event Handler for File/New... Menu Item
	void OnNew(wxCommandEvent& event);
	/// Event Handler for File/Load Menu Item
	void OnLoad(wxCommandEvent& event);
	/// Event Handler for File/Print Preview Menu Item
	void OnPrintPreview(wxCommandEvent& event);
	/// Event Handler for Edit/Turn Timer On/Off Menu Item
	void OnDisableTimer(wxCommandEvent& event);
	/// Event Handler for Disable Timer Menu Item
	void OnGridColour(wxCommandEvent& event);
	/// Event Handler for Grid Colour Menu Item
	void OnSaveScheme(wxCommandEvent& event);
	/// Event Handler for Save Scheme Menu Item
	void OnResetScheme(wxCommandEvent& event);
	/// Event Handler for Net Play Menu Item
	void OnNetPlay(wxCommandEvent& event);
	/// Event Handler for Net Message Menu Item / ctrl+t
	void OnNetMsg(wxCommandEvent& event);
	/// Event Handler for Net Port Menu Item
	void OnNetPort(wxCommandEvent& event);
	/// Event Handler for Net Play socket connect event
	void OnSudClientConnect(wxSocketEvent& WXUNUSED(event));
	/// Event handler for starting a USS netplay server
	void StartSudServer(wxCommandEvent& event);
	/// Handles and interprets keyboard shortcuts
	void ProcessKeyboardShortcut(wxKeyEvent& event);
	/// Processes the 100 character packets put onto the socket
	void ProcessPackets(wxSocketEvent& event);
	/// Event Handler for Background Colour Menu Item
	void OnBgColour(wxCommandEvent& event);
	/// Event Handler for High Scores Menu Item
	void OnHighScores(wxCommandEvent& event);
	/// Event Handler for Rules Menu Item
	void OnRules(wxCommandEvent& event);
	/// Event Handler for Help Menu Item
	void OnHelp(wxCommandEvent& event);
	/// Event Handler for Pressing button 1 on the right panel
	void OnPan1(wxCommandEvent& event);
	/// Event Handler for Pressing button 2 on the right panel
	void OnPan2(wxCommandEvent& event);
	/// Event Handler for Pressing button 3 on the right panel
	void OnPan3(wxCommandEvent& event);
	/// Event Handler for Pressing button 4 on the right panel
	void OnPan4(wxCommandEvent& event);
	/// Event Handler for Donate Menu item
	void OnDonate(wxCommandEvent& event);
	/// Event Handler for Tutorial mode being started
	void OnTutorialMode(wxCommandEvent& event);
	/// Event Handler for Play mode being selected
	void OnPlayMode(wxCommandEvent& event);
	/// Event Handler for Print Menu Item
	void OnPrint(wxCommandEvent& event);
	/// Event Handler for About Menu Item
	void OnAbout(wxCommandEvent& event);
	/// Event Handler for Timer event
	void OnTimer(wxTimerEvent& event);
	/// Event Handler for OnPaint/Refresh()
	void OnPaint(wxPaintEvent& event);
	/// Event Handler for a Left Click mouse
	void OnLeftPress(wxMouseEvent& event);
	/// Event Handler for a Right Click mouse
	void OnRightPress(wxMouseEvent& event);
	/// Event Handler for Default Font Menu Item
	void OnDefaultFont(wxCommandEvent& event);
	/// Event Handler for Default Font Colour Menu Item
	void OnDefaultFontColour(wxCommandEvent& event);
	/// Event Handler for Group Box Colour
	void OnDefaultGBColour(wxCommandEvent& event);
	/// Event Handler for Default Heading Colour
	void OnDefaultHeadingColour(wxCommandEvent& event);
	/// Event Handler for Default Title Colour
	void OnDefaultTitleColour(wxCommandEvent& event);
	/// Declare the event table macro so the given events will be parsed with
	/// the given event IDs to trigger the given functions.
	DECLARE_EVENT_TABLE()
};

/// These are the event macros which handle events within the application
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_TIMER(TIMER_ID, MainFrame::OnTimer)
	EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
	EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
	EVT_MENU(wxID_NEW, MainFrame::OnNew)
	EVT_MENU(wxID_SAVE, MainFrame::OnSave)
	EVT_MENU(wxID_SAVE_AS, MainFrame::OnSaveAs)
	EVT_MENU(wxID_LOAD, MainFrame::OnLoad)
	EVT_MENU(wxID_PRINT_PREVIEW, MainFrame::OnPrintPreview)
	EVT_MENU(wxID_DISABLE_TIMER, MainFrame::OnDisableTimer)
	EVT_MENU(wxID_DEFAULT_FONT, MainFrame::OnDefaultFont)
	EVT_MENU(wxID_DEFAULT_FONT_COLOUR, MainFrame::OnDefaultFontColour)
	EVT_MENU(wxID_GB_COLOUR, MainFrame::OnDefaultGBColour)
	EVT_MENU(wxID_GB_FONT_COLOUR, MainFrame::OnDefaultHeadingColour)
	EVT_MENU(wxID_TITLE_COLOUR, MainFrame::OnDefaultTitleColour)
	EVT_MENU(wxID_SAVE_SCHEME, MainFrame::OnSaveScheme)
	EVT_MENU(wxID_RESET_SCHEME, MainFrame::OnResetScheme)
	EVT_MENU(wxID_NET_PLAY, MainFrame::OnNetPlay)
	EVT_MENU(wxID_NET_MSG, MainFrame::OnNetMsg)
	EVT_MENU(wxID_NET_PORT, MainFrame::OnNetPort)
	EVT_MENU(wxID_GRID_COLOUR, MainFrame::OnGridColour)
	EVT_MENU(wxID_BG_COLOUR, MainFrame::OnBgColour)
	EVT_MENU(wxID_HIGH_SCORES, MainFrame::OnHighScores)
	EVT_MENU(wxID_RULES, MainFrame::OnRules)
	EVT_MENU(wxID_DONATE, MainFrame::OnDonate)
	EVT_BUTTON(ID_PAN_BUTTON_1, MainFrame::OnPan1)
	EVT_BUTTON(ID_PAN_BUTTON_2, MainFrame::OnPan2)
	EVT_BUTTON(ID_PAN_BUTTON_3, MainFrame::OnPan3)
	EVT_BUTTON(ID_PAN_BUTTON_4, MainFrame::OnPan4)
	EVT_MENU(wxID_TUT_MODE, MainFrame::OnTutorialMode)
	EVT_MENU(wxID_PLAY_MODE, MainFrame::OnPlayMode)
	EVT_MENU(wxID_PRINT, MainFrame::OnPrint)
	EVT_MENU(wxID_HELP, MainFrame::OnHelp)
	EVT_LEFT_DOWN(MainFrame::OnLeftPress)
	EVT_RIGHT_DOWN(MainFrame::OnRightPress)
	EVT_PAINT(MainFrame::OnPaint)
	EVT_SOCKET(SERVER_ID, MainFrame::OnSudClientConnect)
	EVT_SOCKET(SOCKET_ID, MainFrame::ProcessPackets)
END_EVENT_TABLE()

IMPLEMENT_APP(BasicApplication)

