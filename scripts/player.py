import copy
import subprocess
import ctypes
import time

def execute_ai(ai_process, board_state, current_move):
    stack = [0,0,0,0,0,0,0]
    bit_number1 = 0
    bit_number2 = 0
    for row in range(6):
        for col in range(7 - 1, -1, -1):
            # Left-shift the current bit number and set the least significant bit to the cell value
            bit_number1 <<= 1
            bit_number2 <<= 1
            if board_state[row][col] == "1":
                bit_number1 |= 1
            elif board_state[row][col] == "2":
                bit_number2 |= 1
        bit_number1 <<= 1
        bit_number2 <<= 1
    
    for row in range(6 - 1, -1, -1):
        for col in range(7 - 1, -1, -1):
            if board_state[row][col] == "1" or board_state[row][col] == "2":
                stack[col] = 8 * (6 - row)

    bit_number1 >>= 1
    bit_number2 >>= 1

    if current_move%2 == 1:
        aux = bit_number1
        bit_number1 = bit_number2
        bit_number2 = aux

    board1 = str(ctypes.c_uint64(bit_number1).value)
    board2 = str(ctypes.c_uint64(bit_number2).value)

    AIinput = str(current_move) + "\n" + board1 + "\n" + board2 + "\n" + str(stack[0]) + "\n" + str(stack[1]) + "\n" + str(stack[2]) + "\n" + str(stack[3]) + "\n" + str(stack[4]) + "\n" + str(stack[5]) + "\n" + str(stack[6]) + "\n"

    ai_process.stdin.write(AIinput)    
    ai_process.stdin.flush()

    output_lines = []
    while True:
        line = ai_process.stdout.readline().strip()
        if line == "continue":
            break
        output_lines.append(line)
        print(line)

    move = int(output_lines[-1])
    return move

def display_board(board):
    return
    print("Board:")
    for row in board:
        print(" ".join(row))

def check_win(board, value):
    # Implement the logic to check for a win in the board
    rows = 6
    cols = 7

    # Check for horizontal win
    for row in range(rows):
        for col in range(cols - 3):
            if all(board[row][col + i] == value for i in range(4)):
                return True

    # Check for vertical win
    for row in range(rows - 3):
        for col in range(cols):
            if all(board[row + i][col] == value for i in range(4)):
                return True

    # Check for diagonal win (top-left to bottom-right)
    for row in range(rows - 3):
        for col in range(cols - 3):
            if all(board[row + i][col + i] == value for i in range(4)):
                return True

    # Check for diagonal win (bottom-left to top-right)
    for row in range(3, rows):
        for col in range(cols - 3):
            if all(board[row - i][col + i] == value for i in range(4)):
                return True

    return False

def check_tie(board):
    # Implement the logic to check for a tie in the board
    print("Check Tie")

def play_game(ai1_executable, ai2_executable, time_control, starting_pos, current_move):
    # Initialize the game board and other necessary variables
    rows = 6
    cols = 7
    board = starting_pos
    current_player = 1
    ai1_process = subprocess.Popen([ai1_executable], stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
        universal_newlines=True,
        close_fds=False, text=True)
    ai2_process = subprocess.Popen([ai2_executable], stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
        universal_newlines=True,
        close_fds=False, text=True)
    
    ai1_process.stdin.write(str(time_control) + '\n')
    ai1_process.stdin.flush()

    ai2_process.stdin.write(str(time_control) + '\n')    
    ai2_process.stdin.flush()

    while True:
        # Use subprocess to execute the AI executables
        if current_player == 1:
            print(ai1_executable)
            move = execute_ai(ai1_process, board, current_move)
            #move = int(input('Position?\n'))
        else:
            print(ai2_executable)
            move = execute_ai(ai2_process, board, current_move)
            #move = int(input('Which move do you want to play?\n'))

        reached = False
        # Make the move for the current player
        for row in range(rows - 1, -1, -1):
            if move < 7 and move > -1:
                if board[row][move] == "0":
                    board[row][move] = str(current_player)
                    reached = True
                    break
        
        if reached:
            current_move += 1
            display_board(board) 
            
            # Check if the current player wins or if it's a tie
            if check_win(board, "1"):
                print(f"Player {ai1_executable} wins!")
                end_process(ai1_process)
                end_process(ai2_process)
                return 1
            if check_win(board, "2"):
                print(f"Player {ai2_executable} wins!")
                end_process(ai1_process)
                end_process(ai2_process)
                return 2
            elif current_move == 42:
                print(f"Draw!")
                end_process(ai1_process)
                end_process(ai2_process)
                return 0

            # Switch to the other player
            current_player = 3 - current_player

def end_process(ai_process):
    ai_process.stdin.close()
    ai_process.stdout.close()

    ai_process.terminate()

def main():
    rows = 6
    cols = 7

    empty_board = [["0" for _ in range(cols)] for _ in range(rows)]
    starting_boards = []
    current_move = []

    starting_boards.append(empty_board)
    current_move.append(0)
    for col in range(cols):
        board = [row[:] for row in empty_board]
        board[5][col] = "1"  # Starting piece for Player 1 (X)
        starting_boards.append(board)
        current_move.append(1)

    ai1_executable = "./xms"
    ai2_executable = "./src/xms_latest"
    num_iterations = 2 * len(starting_boards)  # Change this to the desired number of iterations
    timeOutMicro = 125000

    # Initialize dictionaries to store the statistics for each AI when starting first and second
    ai1_stats_first = {"Wins": 0, "Losses": 0, "Draws": 0}
    ai1_stats_second = {"Wins": 0, "Losses": 0, "Draws": 0}
    ai2_stats_first = {"Wins": 0, "Losses": 0, "Draws": 0}
    ai2_stats_second = {"Wins": 0, "Losses": 0, "Draws": 0}

    total_boards = 0
    turn = True
    for total_timecontrols in range(4):
        for iteration in range(14):
            time.sleep(1)
            if turn:
                aux = copy.deepcopy(starting_boards)
                aux2 = copy.deepcopy(current_move)
                result = play_game(ai1_executable, ai2_executable, timeOutMicro, aux[total_boards], aux2[total_boards])

                if result == 1:
                    ai1_stats_first["Wins"] += 1
                    ai2_stats_second["Losses"] += 1
                elif result == 2:
                    ai2_stats_second["Wins"] += 1
                    ai1_stats_first["Losses"] += 1
                else:
                    ai1_stats_first["Draws"] += 1
                    ai2_stats_second["Draws"] += 1
            else:
                aux = copy.deepcopy(starting_boards)
                aux2 = copy.deepcopy(current_move)
                result = play_game(ai2_executable, ai1_executable, timeOutMicro, aux[total_boards], aux2[total_boards])

                total_boards += 1

                if result == 1:
                    ai2_stats_first["Wins"] += 1
                    ai1_stats_second["Losses"] += 1
                elif result == 2:
                    ai1_stats_second["Wins"] += 1
                    ai2_stats_first["Losses"] += 1
                else:
                    ai2_stats_first["Draws"] += 1
                    ai1_stats_second["Draws"] += 1
                    
            
            if iteration % 2 == 0:
                turn = not turn

        total_boards = 0
        timeOutMicro *= 2

    print("AI1 Statistics:")
    print("\nNamed " + ai1_executable)
    print("When starting first:")
    print(f"Wins: {ai1_stats_first['Wins']}, Losses: {ai1_stats_first['Losses']}, Draws: {ai1_stats_first['Draws']}")
    print("When starting second:")
    print(f"Wins: {ai1_stats_second['Wins']}, Losses: {ai1_stats_second['Losses']}, Draws: {ai1_stats_second['Draws']}")
    print(f"Elo: {(ai1_stats_second['Wins'] + ai1_stats_first['Wins'] - ai1_stats_second['Losses'] - ai1_stats_first['Losses']) * 3}")

    print("\nAI2 Statistics:")
    print("\nNamed " + ai2_executable)
    print("When starting first:")
    print(f"Wins: {ai2_stats_first['Wins']}, Losses: {ai2_stats_first['Losses']}, Draws: {ai2_stats_first['Draws']}")
    print("When starting second:")
    print(f"Wins: {ai2_stats_second['Wins']}, Losses: {ai2_stats_second['Losses']}, Draws: {ai2_stats_second['Draws']}")
    print(f"Elo: {(ai2_stats_second['Wins'] + ai2_stats_first['Wins'] - ai2_stats_second['Losses'] - ai2_stats_first['Losses']) * 3}")





if __name__ == "__main__":
    main()
