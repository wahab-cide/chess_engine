"""
Simple CLI interface to play against the Gotham chess engine.
"""

import subprocess

class ChessGame:
    def __init__(self):
        self.board = [
            ['r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'],
            ['p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'],
            ['.', '.', '.', '.', '.', '.', '.', '.'],
            ['.', '.', '.', '.', '.', '.', '.', '.'],
            ['.', '.', '.', '.', '.', '.', '.', '.'],
            ['.', '.', '.', '.', '.', '.', '.', '.'],
            ['P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'],
            ['R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'],
        ]
        self.moves = []
        self.engine = None
        self.white_to_move = True

    def start_engine(self):
        """Start the chess engine subprocess."""
        self.engine = subprocess.Popen(
            ['./chess_engine'],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        # Initialize UCI
        self.send_command("uci")
        self.wait_for("uciok")
        self.send_command("isready")
        self.wait_for("readyok")
        self.send_command("ucinewgame")

    def send_command(self, command):
        """Send a command to the engine."""
        if self.engine:
            self.engine.stdin.write(command + "\n")
            self.engine.stdin.flush()

    def wait_for(self, text):
        """Wait for specific text from engine."""
        while True:
            line = self.engine.stdout.readline().strip()
            if text in line:
                return line
            # Print info lines
            if line.startswith("info"):
                print(f"  Engine: {line}")

    def get_engine_move(self, movetime_ms=2000):
        """Get a move from the engine."""
        position_cmd = "position startpos"
        if self.moves:
            position_cmd += " moves " + " ".join(self.moves)

        self.send_command(position_cmd)
        self.send_command(f"go movetime {movetime_ms}")

        # Wait for bestmove
        while True:
            line = self.engine.stdout.readline().strip()
            if line.startswith("bestmove"):
                move = line.split()[1]
                return move
            elif line.startswith("info"):
                print(f"  {line}")

    def display_board(self):
        """Display the current board position."""
        # File labels (a-h) 
        files = "    " + "   ".join("abcdefgh") + "    "
        print(f"\n{files}")

        # Top border
        print("  +" + "---+" * 8)

        # Board rows
        for i, row in enumerate(self.board):
            rank = 8 - i
            pieces = " | ".join(piece if piece != '.' else ' ' for piece in row)
            print(f"{rank} | {pieces} | {rank}")
            print("  +" + "---+" * 8)

        # Bottom file labels
        print(f"{files}\n")

    def apply_move(self, move_uci):
        """Apply a move in UCI format to the board."""
        from_col = ord(move_uci[0]) - ord('a')
        from_row = 8 - int(move_uci[1])
        to_col = ord(move_uci[2]) - ord('a')
        to_row = 8 - int(move_uci[3])

        # Get the piece
        piece = self.board[from_row][from_col]

        # Validate source square has a piece
        if piece == '.':
            raise ValueError(f"No piece on {move_uci[0]}{move_uci[1]}")

        # Validate piece belongs to the current player
        if self.white_to_move and piece.islower():
            raise ValueError(f"It's White's turn, but {move_uci[0]}{move_uci[1]} has a black piece ({piece})")
        if not self.white_to_move and piece.isupper():
            raise ValueError(f"It's Black's turn, but {move_uci[0]}{move_uci[1]} has a white piece ({piece})")

        # Handle castling
        if piece.upper() == 'K' and abs(from_col - to_col) == 2:
            # King-side castling
            if to_col == 6:
                self.board[from_row][7] = '.'
                self.board[from_row][5] = 'R' if piece.isupper() else 'r'
            # Queen-side castling
            elif to_col == 2:
                self.board[from_row][0] = '.'
                self.board[from_row][3] = 'R' if piece.isupper() else 'r'

        # Handle en passant
        if piece.upper() == 'P' and from_col != to_col and self.board[to_row][to_col] == '.':
            # En passant capture
            captured_row = from_row
            self.board[captured_row][to_col] = '.'

        # Move the piece
        self.board[to_row][to_col] = piece
        self.board[from_row][from_col] = '.'

        # Handle promotion
        if len(move_uci) == 5:
            promo = move_uci[4]
            if piece.isupper():
                self.board[to_row][to_col] = promo.upper()
            else:
                self.board[to_row][to_col] = promo.lower()

        self.moves.append(move_uci)
        self.white_to_move = not self.white_to_move

    def is_valid_move_format(self, move):
        """Check if move is in valid UCI format."""
        if len(move) < 4 or len(move) > 5:
            return False
        if move[0] not in 'abcdefgh' or move[2] not in 'abcdefgh':
            return False
        if move[1] not in '12345678' or move[3] not in '12345678':
            return False
        if len(move) == 5 and move[4] not in 'qrbn':
            return False
        return True

    def play(self):
        """Main game loop."""
        print("=== Gotham Chess Engine ===")
        print("You are White. Enter moves in UCI format (e.g., e2e4)")
        print("Type 'quit' to exit, 'moves' to see move history")
        print()

        self.start_engine()

        while True:
            self.display_board()

            if self.white_to_move:
                # Human's turn (White)
                print("Your move (White): ", end="", flush=True)
                move = input().strip().lower()

                if move == 'quit':
                    self.send_command("quit")
                    break
                elif move == 'moves':
                    print("Move history:", " ".join(self.moves))
                    continue
                elif not self.is_valid_move_format(move):
                    print("Invalid move format! Use UCI notation (e.g., e2e4)")
                    continue

                try:
                    self.apply_move(move)
                except Exception as e:
                    print(f"Error applying move: {e}")
                    continue
            else:
                # Engine's turn (Black)
                print("Engine is thinking...")
                move = self.get_engine_move(movetime_ms=3000)
                print(f"Engine plays: {move}\n")

                if move == "0000":
                    print("Game Over! Checkmate or Stalemate")
                    break

                self.apply_move(move)

        if self.engine:
            self.engine.terminate()
        print("\nThanks for playing!")

if __name__ == "__main__":
    game = ChessGame()
    try:
        game.play()
    except KeyboardInterrupt:
        print("\n\nGame interrupted. Thanks for playing!")
        if game.engine:
            game.engine.terminate()
