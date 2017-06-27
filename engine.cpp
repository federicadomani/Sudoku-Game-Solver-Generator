#include "common.h"
#include "engine.h"


SudokuPuzzle::SudokuPuzzle(void)
{
	Init();
}

SudokuPuzzle::SudokuPuzzle(const SudokuPuzzle &)
{
	Init();
}


SudokuPuzzle::SudokuPuzzle(const int InputArray[9][9][10])
{
	int i, j, k;
	Init();

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<9; k++)
			{
				if (k == 0)
					suOriginalValues[i][j] = InputArray[i][j][k];
				grid_cache[i][j][k] = InputArray[i][j][k];
			}
}

SudokuPuzzle::SudokuPuzzle(const int InputArray[9][9][10], const int InputModes[9][9])
{
	int i, j, k;
	Init();

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<9; k++)
			{
				if (k == 0)
					suOriginalValues[i][j] = InputArray[i][j][k];
				grid_cache[i][j][k] = InputArray[i][j][k];
			}
}

SudokuPuzzle::~SudokuPuzzle()
{
	Init();
}

void SudokuPuzzle::Init(void)
{
	wxString str;
	int i;

	for (i=0; i<9; i++)
	{
		str.Printf(wxT("%c"), (65+i));
		row_name[i] = str;
		str.Printf(wxT("%c"), (49+i));
		col_name[i] = str;
	}
}

wxString SudokuPuzzle::ProvideHint(void)
{
	// Run through solving algorithm until either a complete value could be determined,
	// or the most number of possibilities could be eliminated in any one square.
	int acc, i, j, k, m, entity, cache_entity, change_peak, suDim_cache;
	int elim_list_c[9];
	bool row_used=false, col_used=false, square_used=false;
	bool row_used_c=false, col_used_c=false, square_used_c=false;
	char comb, cache_method_c;
	wxString str;
	str = wxEmptyString;

	// Should return a wxString with [0] = x-offset, [1] = y_offset, 
	// [2+] = hint string

	comb_arr[0] = 0x02;
	comb_arr[1] = 0x01;
	comb_arr[2] = 0x04;
	cache_row=99;
	cache_col=99;

	
	SolvePuzzle(0xff);

	HintCount = 0;
	for(i=0; i<3; i++)
		for(j=0; j<5; j++)
		{
			HintSquares[i][j] = 0;
		}

	// Go through each solving method
	//for (acc=0; acc<3; acc++)
	//{
		//comb=comb_arr[acc];
		comb = 0xff;
		change_peak = 0;
		
		for (i=0; i<9; i++)
		{
			// treat i as the col, j as the row
			for (j=0; j<9; j++)
			{
				for (k=0; k<9; k++)
					elim_list[k] = 0;

				suDim = 0;
				row_used = false;
				col_used = false;
				square_used = false;
				// EliminateValuesPopulates elim_array[10]
				// row entity is equal to j
				entity=j;
				cache_method = 0x00;
				ResetGrid();
				SetupMagicLine(false);

				// Going through all the rows:
				EliminateValues(entity, j, i, comb);
				for (k=0; k<10; k++)
				{
					if (k == 0 && elim_array[k] > 0 && suOriginalValues[i][j] == 0)
					{
						grid_cache[i][j][0] = elim_array[k];
						SetHighlightValues(0, j, 8, j);
						cache_row = j;
						cache_col = i;

						cache_method = (cache_method | suMethod);
						return BuildHintString();
					}
					else if (elim_array[k] == 2 && k > 0 && grid_cache[i][j][k] < 2)
					{
						grid_cache[i][j][k] = elim_array[k];
						row_used=true;
					}
					else if (suDim == entity)
					{
						col_used=true;
					}
				}
				
				if ((suMethod & 0x04) == 0x04)
				{
					SetHighlightValues(0, j, 8, j);
					cache_row = j;
					cache_col = i;
					
					cache_method = (cache_method | suMethod);
					return BuildHintString();
				}

				// m in this instance is the number of "x" entries
				m = 0;
				for (k=1; k<10; k++)
					if (elim_array[k] == 2 && suButModes[i][j][k] == 0)
					{
						elim_list[k-1] = elim_array[k];
						m++;
					}

				if (change_peak < m)
				{
					cache_row = j;
					cache_col = i;
					change_peak = m;
					cache_method = (cache_method | suMethod);
					cache_method_c = cache_method;
					suDim_cache = suDim;
					row_used_c = row_used;
					col_used_c = col_used;
					square_used_c = square_used;
					
					for (k=0; k<9; k++)
						elim_list_c[k] = elim_list[k];
				}

				// col entity is equal to i
				entity = i + 9;
				EliminateValues(entity, j, i, comb);
				// Going through all the columns:
				for (k=0; k<10; k++)
				{
					if (k == 0 && elim_array[k] > 0 && suOriginalValues[i][j] == 0)
					{
						grid_cache[i][j][0] = elim_array[k];
						SetHighlightValues(i, 0, i, 8);
						cache_row = j;
						cache_col = i;
						cache_method = (cache_method | suMethod);
						
						return BuildHintString();
					}
					else if (elim_array[k] == 2 && k > 0 && grid_cache[i][j][k] < 2)
					{
						grid_cache[i][j][k] = elim_array[k];
						col_used=true;
					}
					else if (suDim == entity)
					{
						col_used=true;
					}
				}
				if ((suMethod & 0x04) == 0x04)
				{
					SetHighlightValues(i, 0, i, 8);
					cache_row = j;
					cache_col = i;
					
					cache_method = (cache_method | suMethod);
					return BuildHintString();
				}

				// m in this instance is the number of "x" entries
				m = 0;
				for (k=1; k<10; k++)
					if (elim_array[k] == 2 && suButModes[i][j][k] == 0)
					{
						elim_list[k-1] = elim_array[k];
						m++;
					}

				if (change_peak < m)
				{
					cache_row = j;
					cache_col = i;
					change_peak = m;
					cache_method = (cache_method | suMethod);
					suDim_cache = suDim;
					row_used_c = row_used;
					col_used_c = col_used;
					square_used_c = square_used;
					cache_method_c = cache_method;
					
					for (k=0; k<9; k++)
						elim_list_c[k] = elim_list[k];
				}

				entity = ((int)(j/3))*3 + (int)(i/3) + 18;

				// Going through all the squares:
				EliminateValues(entity, j, i, comb);

				for (k=0; k<10; k++)
				{
					if (k == 0 && elim_array[k] > 0 && suOriginalValues[i][j] == 0)
					{
						grid_cache[i][j][0] = elim_array[k];

						SetHighlightValues(((int)(i/3)*3), ((int)(j/3)*3), 
							(((int)(i/3)*3) + 2), (((int)(j/3)*3) + 2));
						cache_row = j;
						cache_col = i;
						cache_method = (cache_method | suMethod);
						
						return BuildHintString();
					}
					else if (elim_array[k] == 2 && k > 0 && grid_cache[i][j][k] < 2)
					{
						grid_cache[i][j][k] = elim_array[k];
						square_used = true;
					}
					else if (suDim == entity)
					{
						col_used=true;
					}
				}
				if ((suMethod & 0x04) == 0x04)
				{
					SetHighlightValues(((int)(i/3)*3), ((int)(j/3)*3), 
							(((int)(i/3)*3) + 2), (((int)(j/3)*3) + 2));
					cache_row = j;
					cache_col = i;
					
					cache_method = (cache_method | suMethod);
					return BuildHintString();
				}

				if (CheckElim(i, j) > 0 && suOriginalValues[i][j] == 0)
				{
					grid_cache[i][j][0] = CheckElim(i, j);
					if (square_used == true)
						SetHighlightValues(((int)(i/3)*3), ((int)(j/3)*3), 
							(((int)(i/3)*3) + 2), (((int)(j/3)*3) + 2));
					if (row_used == true)
						SetHighlightValues(0, j, 8, j);
					if (col_used == true)
						SetHighlightValues(i, 0, i, 8);

					if (row_used == false && col_used == false && square_used == false)
					{
						SetHighlightValues(i, j, i, j);
						cache_row = j;
						cache_col = i;
						cache_method = (cache_method | suMethod);
						return BuildHintString();
					}
					else
					{
						cache_row = j;
						cache_col = i;
						cache_method = (cache_method | suMethod);
						return BuildHintString();
					}

				}

				// m in this instance is the number of "x" entries
				m = 0;
				for (k=1; k<10; k++)
					if (elim_array[k] == 2 && suButModes[i][j][k] == 0)
					{
						elim_list[k-1] = elim_array[k];
						m++;
					}


				if (change_peak < m)
				{
					cache_row = j;
					cache_col = i;
					change_peak = m;
					cache_method = (cache_method | suMethod);
					cache_method_c = cache_method;
					suDim_cache = suDim;
					row_used_c = row_used;
					col_used_c = col_used;
					square_used_c = square_used;
					for (k=0; k<9; k++)
						elim_list_c[k] = elim_list[k];
				}
			}
		}
		
		for (k=0; k<9; k++)
			elim_list[k] = elim_list_c[k];
		
		cache_method = cache_method_c;

		if ( cache_row != 99 && cache_col != 99 && change_peak > 0)
		{

			if (square_used_c == true)
			{
				SetHighlightValues(((int)(cache_col/3)*3), ((int)(cache_row/3)*3), 
					(((int)(cache_col/3)*3) + 2), (((int)(cache_row/3)*3) + 2));
			}
			if (row_used_c == true)
			{
				SetHighlightValues(0, cache_row, 8, cache_row);
			}
			if (col_used_c == true)
			{
				SetHighlightValues(cache_col, 0, cache_col, 8);
			}

			return BuildHintString();
		}
	return wxEmptyString;
}

void SudokuPuzzle::SetHighlightValues(int x1, int y1, int x2, int y2)
{
	HintSquares[HintCount][0] = x1;
	HintSquares[HintCount][1] = y1;
	HintSquares[HintCount][2] = x2;
	HintSquares[HintCount][3] = y2;
	HintSquares[HintCount][4] = 1;

	HintCount++;
}
int SudokuPuzzle::GenerateRandomNumber(int lower, int upper)
{
	// Generate Random Number between lower and upper inclusive
	int limit, initial;
	// get the floor
	limit = upper - lower + 1;
	initial = rand() % limit;
	return (initial + lower);
}

wxString SudokuPuzzle::BuildHintString(void)
{
	
	int i, acc;
	wxString desc_string[5];
	wxString str;
	wxString HintString;
	bool LoneRanger = false;
	desc_string[1].Printf("<Standard Elimination>");
	desc_string[2].Printf("<Lone Ranger>");
	desc_string[4].Printf("<Set Elimination>");
	

	if (cache_col != 99 && cache_row != 99)
	{
		HintString.Printf("Methods [ ");
		for (acc=0; acc<3; acc++)
			if ( (comb_arr[acc] & cache_method) == comb_arr[acc] )
			{
				str.Clear();
				if ((cache_method & 0x03) != 0x03)
				{
					str.append(desc_string[(int)(comb_arr[acc])]);
					str.append(" ");
				}
				else if (!LoneRanger)
				{
					str.append(desc_string[2]);
					str.append(" ");
					LoneRanger = true;
				}

				HintString.Append(str);
			}

		if (grid_cache[cache_col][cache_row][0] > 0)
		{
			HintString.append("] conclude the value can only be [ ");
			str.Printf("%d ", grid_cache[cache_col][cache_row][0]);
			HintString.append(str);
		}
		else if ((cache_method & 0x04) == 0x04)
		{
			HintString.Printf("Look Closer - there's potential for a set in this section...");
			//update_text = true;
			return HintString;
		}
		else
		{
			HintString.append(" ] eliminate the values [ ");
			for (acc=0; acc<9; acc++)
				if (elim_list[acc] == 2)
				{
					str.Printf("%d ", (acc + 1));
					HintString.append(str);
				}
		}

		HintString.append("] in row ");
		HintString.append(col_name[cache_row]);
		HintString.append(", col ");
		HintString.append(row_name[cache_col]);
		//update_text = true;
	}
	else
	{
		HintString = wxEmptyString;
	}

	return HintString;
}

int SudokuPuzzle::RandNum( void )
{
	return (GenerateRandomNumber(1, 9));
}

int SudokuPuzzle::GeneratePuzzle(int difficulty)
{
	int i, j, last, m=0, value, k, l, i_off=99, j_off=99, puz_return = 99;
	int i_prev, j_prev, iter, new_diff = 99, diff, change, old_val=0;
	int i_tmp1, j_tmp1, i_tmp2, j_tmp2;
	int debug_count;
	wxString str;


	while(puz_return > 0)
	{
		new_diff=99;
		i_off = RandNum() - 1;
		j_off = RandNum() - 1;
		while ( suOriginalValues[i_off][j_off] > 0)
		{
			// generate random offsets between 0 and 9
			i_off = RandNum() - 1;
			j_off = RandNum() - 1;
		}

		// generate random value between 0 and 9
		value = RandNum();
		suOriginalValues[i_off][j_off] = value;

		puz_return = SolvePuzzle(0xff);
		// If puzzle is now completely impossible, backtrack
		if (puz_return == 2)
		{
			for (i=0; i<9; i++)
				for (j=0; j<9; j++)
				{
					suOriginalValues[i_off][j_off] = 0;
				}
		}
		// If puzzle is now solvable
		else if (puz_return == 0)
		{
			while (new_diff != -1)
			{
				i_off = RandNum() - 1;
				j_off = RandNum() - 1;

				SolvePuzzle(0xff);

				while (suOriginalValues[i_off][j_off] > 0)
				{
					// generate random offsets between 0 and 9
					i_off = RandNum() - 1;
					j_off = RandNum() - 1;
				}

				diff = RateDifficulty();

				// If the puzzle is currently too hard
				if (diff > difficulty)
				{
					// Add a new value
					new_diff = 1;
					suOriginalValues[i_off][j_off] = PuzzleSolution[i_off][j_off];
				}
				// If this has gotten too easy, undo the last change
				else if (diff < difficulty && new_diff != 99)
				{
					new_diff = 1;
					if (suOriginalValues[i_prev][j_prev] != 0 
						&& suOriginalValues[i_prev][j_prev] != 0)
					{
						suOriginalValues[i_prev][j_prev] = 0;
					}
					else
					{
						new_diff = -1;
						puz_return = 99;
						//ResetAllArrays();
					}
				}
				// If it's too easy and it's the first pass, start again
				else if (diff < difficulty && new_diff == 99)
				{
					change = 0;
					for (i=0; i<9; i++)
						for (j=0; j<9; j++)
						{
							old_val = suOriginalValues[i][j];
							// Unset the value
							if (suOriginalValues[i][j] > 0)
							{
								suOriginalValues[i][j] = 0;
							}

							// If the puzzle is no longer solvable:
							if (SolvePuzzle(0xff) > 0)
							{
								// Check if Swapping this value with any other helps
								suOriginalValues[i][j] = PuzzleSolution[i][j];
							}
							else if (old_val > 0 && suOriginalValues[i][j] == 0)
							{
								change = 1;
							}
						}

					if (change == 0)
					{
						new_diff = -1;
						puz_return = 99;
					}
				}
				else if (diff == difficulty)
				{
					new_diff = 1;
					for (i=0; i<9; i++)
						for (j=0; j<9; j++)
						{
				    		GeneratedPuzzle[i][j] = suOriginalValues[i][j];
						}
					return 0;
				}

				i_prev = i_off;
				j_prev = j_off;
			}
		}
		if ( (m % 1000) == 0 && m > 0 )
		{
			// If it's not solved by now, best just try again
			for (i=0; i<9; i++)
				for (j=0; j<9; j++)
				{
					// Reset the entire puzzle
					suOriginalValues[i][j] = 0;
				}
		}
		m++;
	}

	return 1;
}


int SudokuPuzzle::RateDifficulty( void )
{
	int Difficulty, i;

	Difficulty = SolveIterations[0] + SolveIterations[1]*4
		+ SolveIterations[2]*16;

	if (Difficulty > 0 && Difficulty <= 1000)
		PuzzleDifficulty = 0;
	else if (Difficulty > 1000 && Difficulty <= 2000)
		PuzzleDifficulty = 1;
	else if (Difficulty > 2000 && Difficulty <= 3000)
		PuzzleDifficulty = 2;
	else if (Difficulty > 3000 && Difficulty <= 4000)
		PuzzleDifficulty = 3;
	else if (Difficulty > 5000 && Difficulty <= 5500)
		PuzzleDifficulty = 4;
	else if (Difficulty > 5500 && Difficulty <= 6000)
		PuzzleDifficulty = 5;
	else if (Difficulty > 6000)
		PuzzleDifficulty = 6;
	else
		PuzzleDifficulty = -1;

	return PuzzleDifficulty;
}

int SudokuPuzzle::SolvePuzzle(char comb)
{
	int num_solved=0, impos, count, iter, i, row, col, j, k, entity, entries=0;
	
	for (i=0; i<3; i++)
		SolveIterations[i] = 0;
	// This sets up *magic_line[row/col/square][offset][0=value, 1-9=Button Modes]
	SetupMagicLine(true);
	
	for (iter=0; iter<10; iter++)
	for (i=0; i<9; i++)
	{
		// treat i as the col
		for (j=0; j<9; j++)
		{
			// EliminateValuesPopulates elim_array[10]
			// row entity is equal to j
			entity=j;
			
			EliminateValues(entity, j, i, comb);
			for (k=0; k<10; k++)
			{
				if (k == 0 && elim_array[k] > 0)
				{
					grid_cache[i][j][0] = elim_array[k];
					
				}
				else if (elim_array[k] == 2 && k > 0)
				{
					grid_cache[i][j][k] = elim_array[k];
				}
			}

			// col entity is equal to i
			entity = i + 9;
			
			EliminateValues(entity, j, i, comb);
			
			for (k=0; k<10; k++)
			{
				if (k == 0 && elim_array[k] > 0)
				{
					grid_cache[i][j][0] = elim_array[k];
				}
				else if (elim_array[k] == 2 && k > 0)
				{
					grid_cache[i][j][k] = elim_array[k];
				}
			}

			entity = ((int)(j/3))*3 + (int)(i/3) + 18;
			
			EliminateValues(entity, j, i, comb);
			for (k=0; k<10; k++)
			{
				if (k == 0 && elim_array[k] > 0)
				{
					grid_cache[i][j][0] = elim_array[k];
				}
				else if (elim_array[k] == 2 && k > 0)
				{
					grid_cache[i][j][k] = elim_array[k];
				}
			}

			if (CheckElim(i, j) > 0)
			{
				grid_cache[i][j][0] = CheckElim(i, j);
			}
		}

	}


	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			if (grid_cache[i][j][0] > 0)
				num_solved++;

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
		{
			count = 0;
			for (k=1; k<10; k++)
				if (grid_cache[i][j][k] == 2)
					count++;
			if (count == 9)
				return 2;
		}
	for (i=0; i<27; i++)
		for (j=0; j<9; j++)
			for (k=0; k<9; k++)
				if (j != k && *magic_line[i][j][0] == *magic_line[i][k][0]
				&& *magic_line[i][j][0] > 0)
				{
					return 2;
				}
	for (i=0; i<27; i++)
		for (j=0; j<9; j++)
			if (*magic_line[i][j][0] > 0)
				entries++;

	if (num_solved == 81)
	{
		for (i=0; i<9; i++)
			for (j=0; j<9; j++)
			{
				PuzzleSolution[i][j] = grid_cache[i][j][0];
			}
			PuzzleDifficulty = RateDifficulty();
		return 0;
	}
	else if (entries < 10)
		return 3;
	else
	{
		return 1;
	}
}

void SudokuPuzzle::ResetGrid()
{
	int i, j, k;

	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (k=0; k<10; k++)
				grid_cache[i][j][k] = 0;
}

int SudokuPuzzle::CheckElim(int col, int row)
{
	int i, curr=0, pos=0;
	for (i=1; i<10; i++)
	{
		if (grid_cache[col][row][i] < 2)
		{
			pos++;
			curr=i;
		}
	}
	if (pos == 1 && curr > 0)
		return curr;
	else
		return 0;
}

bool SudokuPuzzle::SetupMagicLine(bool bare)
{
	// Columns A-I make up 0-8 in the magic_line array
	// Rows 1-9 make up 9-17 in the magic_line array
	// Squares 3x3 make up 18-26 in the magic_line array
	// magic_line is a 3 dimensional array of pointers to corresponding
	// values in the suButModes array (where suButModes[i][j][0] is the 
	// value of suValues[i][j]) so that when their values are changed, 
	// it directly effects the entire sudoku grid
	// *magic_line[row/col/square][offset][0=value, 1-9=Button Modes]

	int i, j, g, k, h, m, row, col;


	// This part is designed to generate the magic_line array of pointers

	// Set up grid_cache[9][9][10] - copy of suValues[9][9] and suButModes[9][9][9]
	// This for loop deals with the rows
	for (i=0; i<9; i++)
	{
		for (j=0; j<9; j++)
		{
			// If the square in question is entered by user and is correct
			if (suOriginalValues[j][i] == 0 && 
				suValues[j][i] == PuzzleSolution[j][i] &&
				!bare)
					grid_cache[j][i][0] = suValues[j][i];

			// if the result was incorrect
			else
				grid_cache[j][i][0] = suOriginalValues[j][i];

			if (bare)
				grid_cache[j][i][0] = suOriginalValues[j][i];

			magic_line[i][j][0] = &grid_cache[j][i][0];

			for (k=1; k<10; k++)
			{
				if (suButModes[j][i][k] == 2 && PuzzleSolution[j][i] != k)
					grid_cache[j][i][k] = suButModes[j][i][k];
				else
					grid_cache[j][i][k] = 0;

				if (bare)
					grid_cache[j][i][k] = 0;

				magic_line[i][j][k] = &grid_cache[j][i][k];
			}

		}
	}

	// This for loop deals with the columns
	for (i=9; i<18; i++)
	{
		for (j=0; j<9; j++)
		{
			magic_line[i][j][0] = &grid_cache[i-9][j][0];
			for (k=1; k<10; k++)
			{
				magic_line[i][j][k] = &grid_cache[i-9][j][k];
			}
		}
	}

	// This for loop deals with the squares
	for (i=18; i<27; i++)
	{
		j = 0;
		m = i-18;
		row = (m%3)*3;
		col = (int)(m/3)*3;
		for (g=0; g<3; g++)
		{
			for (h=0; h<3; h++)
			{
				magic_line[i][j][0] = &grid_cache[row+h][col+g][0];
				for (k=1; k<10; k++)
				{
					magic_line[i][j][k] = &grid_cache[row+h][col+g][k];
				}
				j++;
			}
		}

	}

	return true;
}

void SudokuPuzzle::ResetElimArray()
{
	int i;
	for (i=0; i<10; i++)
	{
		elim_array[i] = 0;
	}
}


void SudokuPuzzle::EliminateValuesByElim(int entity, int row, int col)
{

	int i, j;

	// For each of the possible values of the square, cross out
	// if they are already in the entity elsewhere
	for (i=1; i<10; i++)
	{
		// magic_line[entity][*][0] represents the entire row's values
		// return_array is the array of possibilities for the square
		// with [0] being the value if applicable


		// for each of the elements in the row
		for (j=0; j<9; j++)
		{
			// Check if their value is the value currently being examined
			if (*magic_line[entity][j][0] == i && elim_array[i] < 2)
			{
				// Mark that index as being impossible
				elim_array[i] = 2;
				SolveIterations[0]++;
				suDim = entity;
				suMethod = (suMethod | 0x01);
			}
		}

	}
}

void SudokuPuzzle::EliminateValuesByLone(int entity, int row, int col)
{
	int curr, impos, k, i, j;
	// For each of the possible values of the square, check to see if
	// it's the only remaining value of it's kind in the entity
	for (i=1; i<10; i++)
	{
		// search the magic_line array and count how many of the
		// other members of the entity DEFINITELY have the i value
		// eliminated
		curr = i;
		impos = 0;

		for (j=0; j<9; j++)
		{
			if(*magic_line[entity][j][0] != 0)
				impos++;
			else if(*magic_line[entity][j][0] == i)
				impos = 9;
		}
		
		if (elim_array[i] < 2 && grid_cache[col][row][i] < 2)
		{

			// for each of the elements in the entity
			for (j=0; j<9; j++)
			{
				// If this value has not already been crossed out and number is not specified
				if (*magic_line[entity][j][i] == 2 && *magic_line[entity][j][0] == 0)
				{
					impos++;
				}
			}
			
			if (impos == 8 && elim_array[i] != 2)
			{
				elim_array[0] = i;
				for (k=1; k<10; k++)
					if (k != i)
					{
						elim_array[k] = 2;
						SolveIterations[1]++;
						suDim = entity;
						suMethod = (suMethod | 0x02);
					}
			}
		}
	}
}

void SudokuPuzzle::EliminateValuesBySet(int entity, int row, int col)
{
	int i, j, k, m, n, o, p, match_off[9], match_off_c[9], count, count_max=0, count_array[9];
	int match_count, match, row_c, col_c, suSet[10];
	wxString all_str;
	// For each of the entries in the entity, check if there are N
	// groups of N possibilitiesm e.g. 3 entries in a row are either
	// 7, 8, or 9 (perhaps their possibilities are 7,8,9/7,9/7,8), no
	// other entries in the row could possibly be these values
	// which contain 

	for (i=0; i<10; i++)
		suSet[i] = 0;

	// going through each offset of the entity
	for (j=0; j<9; j++)
	{
		match_off[j] = 0;
		match_off_c[j] = 0;
		count = 0;
		// Count how many possibilities are in each entry
		for (k=1; k<10; k++)
		{
			if(*magic_line[entity][j][k] == 0)
				count++;
		}
		count_array[j] = count;
		if (*magic_line[entity][j][0] == 0)
			count_max++;
	}


	// for each of the useful quantities of possible entries
	for (k=2; k<(count_max-1); k++)
	{
		// For each of the offsets in the array (also the entry which
		// everything else is compared against)
		for (j=0; j<9; j++)
		{
			for (n=0; n<9; n++)
			{
				// reset match_off[] array - holds matching offsets with a 1,
				// sets unmatching offsets within a set with a 0
				match_off[n] = 0;
			}
			match_count = 1;
			// Go through each of the other entities and compare
			// their entries
			for (m=0; m<9; m++)
			{
				// If the number of possibilities in the square is useful
				if (count_array[j] == k && count_array[m] <= k && 
					*magic_line[entity][m][0] == 0 && j != m)
				{
					match = 1;
					// Go through each of the entries in this entity
					for (i=1; i<10; i++)
					{
						if(	*magic_line[entity][j][i] == 2 && *magic_line[entity][m][i] == 0)
						{
							// match will show how many sharing x's they have.
							match = 0;
						}
						else if (*magic_line[entity][j][i] == 0)
						{
							if (match != 0)
								match++;
						}
					}
					if (match > 0)
						match--;

					// Checking for current match necessary count
					if (match == k && match > 0)
					{
						match_count++;
						match_off[m] = 1;
						match_off[j] = 1;
						// If the required number of matching sets has been met
						if (match_count >= k)
						{
							// For each of the other entries in this entity, which we have
							// now established can be taken down further with set theory
							for (n=0; n<9; n++)
							{
								// If this if not a member of the maching entries within the set
								if (match_off[n] == 0)
								{
									// for each of the possible values for this set
									for (i=1; i<10; i++)
									{
										// if the number is currently not already eliminated, and is already
										// excluded thanks to it being a member of the set being examined
										if(*magic_line[entity][n][i] < 2 && *magic_line[entity][j][i] == 0)
										{
											// Mark this compared value as being impossible
											*magic_line[entity][n][i] = 2;
											suDim = entity;
											SolveIterations[2]++;
										}
										if ( *magic_line[entity][j][i] == 0
											 && suButModes[col][row][i] < 2 && suHint == true)
										{
											for (o=0; o<9; o++)
												match_off_c[o] = match_off[o];
												suSet[i]= 2;
										}
									}
								}
							}

							// Reset offset-holding array
							for (n=0; n<9; n++)
							{
								match_off[n] = 0;
							}

						}
					}
				}
			}
		}

	}

	p = 0;

	for (i=0; i<9; i++)
		if (match_off_c[i] > 0)
			p++;

	o=0;
	for (i=0; i<9; i++)
		if (suSet[i] > 1)
			o++;

	// if we need to provide a hint, go through established set and see if
	// it has already been eiliminated with suButModes and if it has,
	// unset the suMethod set mode bit
	if ( suHint == true && p > 1 && o > 1)
	{
		// For each of the offsets in this entity
		for (i=0; i<9; i++)
		{
			// if this offset isn't part of the set
			if (match_off_c[i] != 1)
			{
				// check each possible value to see if it has a member 
				// of the set un-eliminated
				for (j=1; j<10; j++)
				{
					// if this entry is part of set and not currently eliminated
					if (suSet[j] == 2)
					{
						// Find out what row and column this offset and entity
						// refers to
						if (entity <= 9)
						{
							col_c = i;
							row_c = entity;
						}
						else if (entity > 9 && entity <= 18)
						{
							col_c = entity - 9;
							row_c = i;
						}
						else
						{
							col_c = (((entity - 18) % 3) * 3) + (i % 3);
							row_c = ((int)((entity - 18)/3)*3) + (int)(i / 3);
						}

						if (suButModes[col_c][row_c][j] != 2 && suValues[col_c][row_c] == 0)
						{
							suMethod = (suMethod | 0x04);
						}
					}
				}
			}
		}
	}
}

void SudokuPuzzle::EliminateValues(int entity, int row, int col, char comb)
{
	// comb is a bit mask representing which methods to attempt with solving
	int i, j, change_e = 0, change_g1 = 0, change_g2 = 0;
	int diff_c[10];
	int grid_cache_c[10];

	ResetElimArray();
	suMethod = 0x00;


	// Count no of elements already eliminated
	for (i=1; i<10; i++)
		if (grid_cache[col][row][i] > 0)
		{
			change_g1++;
			grid_cache_c[i] = grid_cache[col][row][i];
		}
		else
			grid_cache_c[i] = 0;

	if (grid_cache[col][row][0] == 0)
	{	
		if ((comb & 0x01) == 0x01)
			EliminateValuesByElim(entity, row, col);
		if ((comb & 0x02) == 0x02)
			EliminateValuesByLone(entity, row, col);
		if ((comb & 0x04) == 0x04)
			EliminateValuesBySet(entity, row, col);
		//DebugString(wxString(wxT("Finished Elim:")));
	}
	else
	{
		//if (col == 0 && row == 0)
		//	DebugInt(suValues[col][row]);
		elim_array[0] = grid_cache[col][row][0];
		for (i=1; i<10; i++)
		{
			elim_array[i] = grid_cache[col][row][i];
		}
	}
}