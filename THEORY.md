# Chess Engine Evaluation Theory

## Understanding Chess Evaluation

A chess engine's evaluation function tries to answer: **"Who is better and by how much?"**

The score is in **centipawns** (1/100th of a pawn):
- +100 = White is ahead by one pawn
- -50 = Black is ahead by half a pawn
- +300 = White is ahead by about 3 pawns (usually winning)
- ±1000 = Overwhelming advantage
- ±100000 = Checkmate

---

## The Three Pillars of Evaluation

### 1. Material (40-50% of evaluation)

Basic piece values:
- Pawn: 100 cp (baseline)
- Knight: 320 cp (~3 pawns)
- Bishop: 330 cp (~3.3 pawns)
- Rook: 500 cp (~5 pawns)
- Queen: 900 cp (~9 pawns)
- King: 20000 cp (infinite value)

### 2. Positional Factors (30-40% of evaluation)

This is what we're adding now:
- Pawn structure
- Piece placement (PST)
- King safety
- Piece mobility
- Control of center
- Piece coordination

### 3. Tactical Factors (10-20% of evaluation)
**Future Implementation:**

- Threats
- Pins and forks
- Discovered attacks
- X-ray attacks
- Static Exchange Evaluation (SEE)

---


### Why Pawns Are Special

Pawns are the "soul of chess" (Philidor). They:
1. **Cannot move backward** - Weaknesses are permanent
2. **Define the position** - Determine piece placement
3. **Create targets** - Weak pawns become attack points
4. **Control space** - Restrict opponent pieces
5. **Transform** - Promote to pieces

**Key principle:** Pawn moves are commitments. You can't undo them.

---

## Pawn Structure Concepts

### 1. Doubled Pawns

```
Bad:                    Good:
. . . .                 . . . .
. P . .                 . . . .
. P . .                 . P . .
. . . .                 P . . .
```

**Why doubled pawns are bad:**
- Both pawns on same file = less control
- Front pawn blocks the rear one
- Rear pawn is hard to defend
- Creates holes on adjacent files
- Less flexible pawn majority

**Exceptions (when not so bad):**
- Open file with rook behind
- Trading doubled pawn for bishop pair
- Creating passed pawn opportunity
- Controlling key squares

**Examples from master games:**
- Doubled c-pawns in certain Nimzo-Indian structures
- Doubled f-pawns after Bxf6 (common tradeoff)
- Doubled d-pawns after cxd3 in Jobava London

---

### 2. Isolated Pawns

```
Isolated d-pawn (IQP):

. . . . . . . .
. . . . . . . .
. . P . P . . .  <- isolated
. P . P . P . .
. . . . . . . .
```

**Why isolated pawns are weak:**
- Cannot be defended by other pawns
- Square in front (d5) becomes outpost
- Becomes target in endgame
- Restricts piece mobility

**Compensations:**
- Open files for rooks
- Active piece play
- Central control
- Attacking chances

**The Isolated Queen Pawn (IQP):**
Most common isolated pawn position:
- On d4 or d5
- Common from many openings
- Plan: Attack in middlegame, avoid endgame
- If pushed to d5: can become strength or weakness

**Famous example:** Tarrasch Defense, Caro-Kann Panov

---

### 3. Passed Pawns

```
Unstoppable passed pawn:

. . . . . . . K
. . . . . . . .
. . P . . . . .  <- Passed!
. . . . . . . .
k . . . . . . .
```

**Tarrasch quote:** "The passed pawn is a criminal that must be kept under lock and key."

**Why passed pawns are strong:**
- Advance toward promotion
- Tie down opponent pieces
- Create threats
- Force defensive concessions
- Win endgames

**Value progression:**
```
Rank 7: +120 cp (one step from queening!)
Rank 6: +90 cp  (very dangerous)
Rank 5: +60 cp  (threatening advance)
Rank 4: +40 cp  (starting to be serious)
Rank 3: +30 cp  (long-term asset)
Rank 2: +30 cp  (far from promotion)
```

**Types of passed pawns:**

**Outside passed pawn:** Far from kings, wins endgames
```
k . . . . . . K
. . . . . . . P  <- Outside
```

**Protected passed pawn:** Defended by another pawn
```
. . . . . . . .
. . . P . . . .  <- Protected
. . P . . . . .
```

**Connected passed pawns:** Multiple passed pawns, very strong
```
. . . . . . . .
. . P P . . . .  <- Connected
```

**Blockaded passed pawn:** Piece in front, less dangerous
```
. . . . . . . .
. . N . . . . .  <- Blockader
. . P . . . . .
```

---

### 4. Pawn Chains

```
French Defense structure:

. . . . . . . .
. . . . . . . .
. . . P . . . .  <- Chain
. . P . P . . .     head
. P . . . P . .     (e5)
P . . . . . P .
```

**Nimzowitsch quote:** "Attack the base of the pawn chain"

**Chain structure:**
- **Base:** Bottom of chain (c3 in example)
- **Head:** Top of chain (e5 in example)

**Strategic principles:**
- Base is weak point
- Head controls space
- Pieces operate on same color as chain
- Opposite colored bishops favor attacker

**Classic example:** French Defense
- White: e4-e5-d4 chain
- Black attacks with c5, f6
- White attacks with f4-f5

---

### 5. Connected Pawns

```
Healthy pawn structure:

. . . . . . . .
. . . . . . . .
P P P . . . . .  <- Connected
. . . . . . . .
```

**Why connected pawns are strong:**
- Can defend each other
- Harder to attack
- Control adjacent squares
- Form solid structure
- Flexible advance options

**Comparison:**
```
Connected:          Disconnected:
P . P               P . . . P
  ^                   ^ Isolated
Can defend          Cannot defend
```

---

### 6. Backward Pawns

```
Backward d-pawn:

. . . . . . . .
. . . . . . . .
. . . . . . . .
. P . P . . . .  <- These advanced
. . P . . . . .     <- This backward
```

**Why backward pawns are weak:**
- Cannot advance safely
- Square in front is weak (hole)
- Target for attack
- Hard to defend
- Restricts piece play

**Common patterns:**
- Sicilian Dragon: d6 backward
- King's Indian: d6 backward
- Often on semi-open file

---

## Pawn Evaluation Heuristics

### Rule of Square (Passed Pawn Races)
```
If king can enter "square" = can catch pawn
. . . . . . . K
. . x x x x x .
. . x P x x x .  <- Square
. . x x x x x .
k . x x x x x .
```

### Pawn Majorities
```
Queenside majority (White):
P P P . . . p p
```
**Value:** +20-30 cp (can create passed pawn)

### Principle of Two Weaknesses
- Attacking one weakness: defendable
- Attacking two weaknesses: one falls

---

## Integration with Other Factors

### Pawn Structure + King Safety
```
Weakened kingside pawns:
. . . . . k P .  <- Weak pawns
. . . . . p . p     near king
```
**Extra penalty:** -30 cp

### Pawn Structure + Piece Placement
```
Knight outpost on hole:
. . . . . . . .
. . . p . p . .
. . p . . . p .  <- Holes
. . . N . . . .     at d5, f5
```
**Knight on outpost:** +30 cp bonus

### Pawn Structure + Endgames
- Pawn weaknesses matter more in endgames
- Scale factor: multiply by 1.5 in endgame
- Passed pawns even more valuable

---

## Evaluation Balance

A good evaluation balances:

1. **Speed:** Must be fast (called millions of times)
2. **Accuracy:** Must guide search correctly
3. **Complexity:** Not too complex (diminishing returns)

**Typical breakdown:**
```
Material:           60%
Pawn structure:     15%
Piece placement:    10%
King safety:        10%
Mobility:           5%
```

---

## Common Evaluation Mistakes


## Testing Evaluation Improvements

### Position Test Suite
Use these to verify evaluation:

**1. Lucena Position (Passed Pawn Win)**
```
FEN: 1K6/1P6/8/8/8/8/r7/2k5 w - - 0 1
Should: Heavily favor passed pawn
```

**2. Philidor Position (Passive Defense)**
```
FEN: 3k4/R7/8/8/8/8/r7/3K4 b - - 0 1
Should: Recognize drawing structure
```

**3. Doubled Pawns**
```
FEN: 8/8/8/8/8/p7/p7/8 w - - 0 1
Should: Penalize doubled pawns
```

---

## References

### Essential Resources

**Websites:**
- Chess Programming Wiki: https://www.chessprogramming.org/
- Computer Chess Club: http://talkchess.com/

**Books:**
- "Pawn Structure Chess" by Andrew Soltis
- "Pawn Power in Chess" by Hans Kmoch
- "My System" by Aron Nimzowitsch

---

Last Updated: December 2, 2025
