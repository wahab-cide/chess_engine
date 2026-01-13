A UCI-compatible chess engine with alpha-beta search, quiescence search, transposition tables, and advanced move ordering.

## Building the Engine

### Prerequisites
- C++ compiler (g++ or clang)
- Make utility

### Compilation
```bash
cd chess_engine
make
```

To clean and rebuild:
```bash
make clean
make
```

The compiled executable will be: `chess_engine`

### Compilation Flags
For maximum performance on your specific CPU:
```bash
# Edit Makefile and change CXXFLAGS to:
CXXFLAGS = -std=c++11 -O3 -march=native -flto -Wall -Wextra
```

## Running the Engine (Command Line)

### Interactive Mode
```bash
./chess_engine
```

Then type UCI commands:
```
uci                    # Initialize engine
isready                # Check if ready
position startpos      # Set starting position
go movetime 3000       # Search for 3 seconds
quit                   # Exit
```

### Test with Pipe
```bash
echo -e "uci\nisready\nposition startpos\ngo movetime 1000\nquit" | ./chess_engine
```

### Example UCI Session
```bash
cat << 'EOF' | ./chess_engine
uci
isready
position startpos moves e2e4 e7e5 g1f3
go movetime 2000
quit
EOF
```

## Setting Up a Chess GUI

### Recommended: En Croissant (Modern, Cross-Platform)

**Download & Install:**
- Visit: https://github.com/franciscoBSalgueiro/en-croissant/releases
- Download the latest release for your OS:
  - **macOS:** `.dmg` file
  - **Windows:** `.exe` installer
  - **Linux:** `.AppImage` or `.deb`

**Configure Engine:**
1. Open En Croissant
2. Navigate to: `Settings` → `Engines`
3. Click `Add Engine`
4. Browse to your compiled `chess_engine` executable
5. Name it `Gotham`
6. Protocol: `UCI` (auto-detected)
7. Save




### Alternative GUIs


#### Tarrasch Chess GUI (Simple, macOS)

**Platform:** macOS
**Download:** https://www.triplehappy.com/

**Setup:**
1. Install from DMG
2. `File` → `Engines` → `+`
3. Browse to engine executable

**Best for:** macOS users who want a simple, native interface.

#### ChessX (Open Source)

**Platform:** macOS, Windows, Linux
**Download:** https://chessx.sourceforge.io/

**Setup:**
1. Install ChessX
2. `Engine` → `Add Engine`
3. Browse to engine path


#### Arena Chess GUI (Classic)

**Platform:** Windows (Mac/Linux via Wine)
**Download:** http://www.playwitharena.de/

**Best for:** Windows users, classic interface.


## Playing Against Other Engines

Test your engine's strength against established chess engines.

### Popular Free Engines

#### Stockfish (Strongest Open Source Engine)
**Rating:** ~3635 ELO

**Installation:**
```bash
# macOS (Homebrew)
brew install stockfish

# Linux (Debian/Ubuntu)
sudo apt-get install stockfish

# Windows: Download from https://stockfishchess.org/download/
```

**Usage:** Stockfish has adjustable skill levels (0-20) for testing.

