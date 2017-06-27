

class SudokuPuzzle
{
public:
	// SudokuPuzzle Constructors
	int PuzzleSolution[9][9];
	int grid_cache[9][9][10];
	int PuzzleDifficulty;
	int GeneratedPuzzle[9][9];
	/// This will contain the x1, y1, x2 and y2 locations for drawing highlighted
	/// squares for hints, up to a maximum of 3 values. HintSquares[x][4] is set
	/// to 1 if that value is currently set (so 0, 0, 0, 0 doesn't show up)
	int HintSquares[3][5];
	/// This will return a string for a hint when in tutorial mode. If there are no
	/// new hints available with the given information, wxEmptyString will be returned
	wxString ProvideHint(void);
	/// This is a complex function which tries to determine if the sudoku puzzle is
	/// solvable by known methods. This may generate an array which may then be
	/// referenced to determine if a user's entries are correct, or if the puzzle
	/// they loaded is invalid. The function calls SetupMagicLine() to generate
	/// the *magic_line[27][9][10] array of pointers, then cyphens through the
	/// entries using the EliminateValues() function to eliminate all possibilities
	/// and thus determine the only possible solution. Returns 0 if solved, 1 if
	/// unsolved and 2 if determined impossible
	int SolvePuzzle(char comb);
	/// This generates a puzzle based on repeatedly picking random numbers,
	/// calling SolvePuzzle(0x0f), sees if it's solvable and of a specific
	/// difficulty, and only leaves the function when it is both.
	int GeneratePuzzle(int difficulty);
	SudokuPuzzle(void);
	SudokuPuzzle(const SudokuPuzzle &);
	SudokuPuzzle(const int InputArray[9][9][10]);
	SudokuPuzzle(const int InputArray[9][9][10], const int InputModes[9][9]);
	~SudokuPuzzle();
private:
	int HintCount;
	/// Columns A-I make up 0-8 in the magic_line array
	/// <br>Rows 1-9 make up 9-17 in the magic_line array
	/// <br>Squares 3x3 make up 18-26 in the magic_line array.
	/// <br>magic_line is a 3 dimensional array of pointers to corresponding
	/// values in the suButModes array (where suButModes[i][j][0] is the 
	/// value of suValues[i][j]) so that when their values are changed, 
	/// it directly effects the entire sudoku grid, so modifications
	/// are updated in real-time
	/// *magic_line[row/col/square][offset][0=value, 1-9=Button Modes]
	int *magic_line[27][9][10];
	/// SolveIterations[0]: the number of times standard elimination are used
	/// Worth 1
	/// SolveIterations[1]: the number of times lone rangers are used
	/// Worth 4
	/// SolveIterations[2]: The number of times set theory is used
	/// Worth 16
	int SolveIterations[3];
	/// Whether or not a hint is currently being requested
	bool suHint;
	/// Fuction to zero-fill the magic_line array between calls
	/// to EliminateValues().
	void ResetElimArray(void);
	/// This returns a pseudo random number between lower and upper
	int GenerateRandomNumber(int lower, int upper);
	/// Function to return a difficulty rating from 0-6
	int RateDifficulty(void);
	/// This generates a pseudo random number between 1 and 10
	int RandNum( void );
	/// This resets all array elements that correspond to the sudoku grid
	void ResetGrid(void);
	/// This function goes through the suButModes in any square and checks
	/// if there is only one value remaining, if true, it returns this value,
	/// if not, it returns 0.
	int CheckElim(int col, int row);
	/// This is the function which generates a complete hint string based on the best
	/// possible next step in solving the puzzle. It skips straight to the step in the
	/// puzzle which yields the greatest results.
	wxString BuildHintString(void);
	void SetHighlightValues(int x1, int y1, int x2, int y2);
	/// When EliminateValues is called, this wll be populated with a mask
	/// representing the methods used. 
	char suMethod;
	/// This is a copy of suValues[9][9] at the time which a game entered play
	/// mode so that any entries can be undone at any stage.
	int suOriginalValues[9][9];
	/// Just a wxString to save it being re-declared in every function -
	/// this wxString is only ever user for temporary values.
	int elim_array[10];
	/// This will eliminate values for the square based on standard elimination
	void EliminateValuesByElim(int entity, int row, int col);
	/// This will eliminate values for the square based on line ranger
	void EliminateValuesByLone(int entity, int row, int col);
	/// This will eliminate values for the ENTIRE ENTITY based on set theory
	void EliminateValuesBySet(int entity, int row, int col);
	/// Function to Eliminate possibilities based on the
	/// *magic_line[][][] array
	void EliminateValues(int entity, int row, int col, char comb);
	int suValues[9][9];
	/// Char representations of the three difficulties
	char comb_arr[3];
	/// The last row which yielded the best results in providing a hint
	int cache_row;
	/// The last col which yielded the best results in providing a hint
	int cache_col;
	/// This is the net total of elements eliminated in a square by the
	/// various methods during providing a hint
	int elim_list[9];
	/// This is a hexidecimal representation of the solving methods used in the last
	/// pass while trying to provide a hint.
	char cache_method;
	/// Represents the entities in magic line which have just been referenced for solving
	int suDim;
	void Init(void);
	int suButModes[9][9][9];
	/// Function to set up the magic_line[][][] array - bare=true means
	/// only original values will be used
	bool SetupMagicLine(bool bare);
	/// This wxString array contains the values of the entries of the rows (1-9) so
	/// they can be referenced and re-named to anything we want at a later point.
	wxString row_name[9];
	/// This wxString array contains the values of the entries on the column (A-I)
	/// they can be referenced and re-named to anything we want at a later point.
	wxString col_name[9];
	/// wxMemoryDC to act as a bitmap cache for relaying wxDC objects between
	/// different classes
};
