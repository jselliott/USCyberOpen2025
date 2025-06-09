import random
import secrets
import json
import sys

BOARD_SIZE = 1_000_000
SHIP_SIZES = [5, 4, 3, 3, 2]
MAX_TURNS = 500

seed = secrets.randbits(64)
rng = random.Random(seed)
FLAG = open("/flag.txt","r").read()

def place_ship(board, start_row, start_col, size, direction):
    positions = []
    for i in range(size):
        r = start_row + i if direction == 'V' else start_row
        c = start_col + i if direction == 'H' else start_col
        if not (0 <= r < BOARD_SIZE and 0 <= c < BOARD_SIZE):
            return False
        pos = (r, c)
        if pos in board:
            return False
        positions.append(pos)
    for pos in positions:
        board.add(pos)
    return True

def place_computer_ships():
    board = set()
    for size in SHIP_SIZES:
        placed = False
        r = rng.randint(0, BOARD_SIZE - 1)
        c = rng.randint(0, BOARD_SIZE - 1)
        d = 'H' if rng.random() < 0.5 else 'V'
        placed = place_ship(board, r, c, size, d)
    return board

def parse_ship_input(data):
    try:
        ships = json.loads(data)
        if not isinstance(ships, list) or len(ships) != len(SHIP_SIZES):
            return None
        parsed = []
        for entry in ships:
            if (not isinstance(entry, list) or len(entry) != 3 or
                not isinstance(entry[0], int) or not isinstance(entry[1], int) or
                entry[2] not in ("H", "V")):
                return None
            parsed.append((entry[0], entry[1], entry[2]))
        return parsed
    except:
        return None

def start_game():
    print("Starting new game.")
    print("Enter your ship placements as JSON list (e.g. [[100,200,\"V\"], [300,400,\"H\"], ...])")
    print(f"You must sink all computer ships in under {MAX_TURNS} moves.")
    print()

    # Get player ship placements
    while True:
        print("Ship placements:")
        line = sys.stdin.readline()
        if not line:
            return
        ships = parse_ship_input(line.strip())
        if not ships:
            print("Invalid input. Must be a JSON list like [[r,c,\"H\"]].")
            continue
        break

    # Place player ships
    player_board = set()
    for idx, (r, c, d) in enumerate(ships):
        if not place_ship(player_board, r, c, SHIP_SIZES[idx], d):
            print(f"Invalid placement for ship {idx+1}.")
            return

    # Place computer ships
    computer_board = place_computer_ships()
    player_shots = set()

    for turn in range(0, MAX_TURNS):
        print(f"Turn {turn+1} - Enter your shot as row,col (e.g. 12345,67890):")
        line = sys.stdin.readline()
        if not line:
            return
        try:
            sr, sc = map(int, line.strip().split(","))
        except:
            print("Invalid input. Must be row,col format.")
            continue

        if (sr, sc) in player_shots:
            print("Already shot there.")
            continue

        player_shots.add((sr, sc))
        if (sr, sc) in computer_board:
            print("HIT")
        else:
            print("MISS")

        if computer_board.issubset(player_shots):
            print("You sank all the computer's ships! You win.")
            print(f"Flag: {FLAG}")
            return

        # Computer fires (output must be observable!)
        cr = rng.randint(0, BOARD_SIZE - 1)
        cc = rng.randint(0, BOARD_SIZE - 1)
        result = "HIT" if (cr, cc) in player_board else "MISS"
        print(f"Computer fires at {cr},{cc} - {result}")

    print("You ran out of moves. Game over.")


print("== BATTLESHIP CTF ==")
print("Board size: 1,000,000 x 1,000,000")
print("Sink all computer ships in under 1,000 moves.")
print("Good luck.\n")

while True:
    start_game()
    print("Press enter to try again or type exit to quit")
    cmd = input("> ")
    if cmd.strip().lower() == "exit":
        break