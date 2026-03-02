#!/usr/bin/env python3
"""
generate_maps.py — Generate Algorithmic Arena .map files.

Run from the project root:
    python3 scripts/generate_maps.py

Produces:
    maps/nexus_siege_512.map      Primary gameplay + benchmark map
    maps/benchmark_open_512.map   Open field for collision stress-test
    maps/benchmark_maze_512.map   Dense maze for pathfinding stress-test

Map symbol legend:
    G  Grass      (passable, speed 1.0x)
    M  Mud        (passable, speed 0.6x)
    T  Tree       (impassable, maze wall)
    L  Lava       (impassable, blocker hazard)
    F  Flag       (passable, capture objective)
    D  Deploy     (Grass tile in deployment zone — Minion spawn area)
    C  Commander  (Commander start tile, underlying tile is Grass)
    E  Entrance   (Grass tile at maze entry point, bordering deploy zone)
"""

import os
import random

W = 512
H = 512
DEPLOY_DEPTH = 3   # tile rows/cols reserved as deployment zone on all four sides
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "..", "maps")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def make_grid(w, h, fill="G"):
    return [[fill] * w for _ in range(h)]


def write_map(path, name, time_limit, minion_cap, tile_size, grid):
    w = len(grid[0])
    h = len(grid)
    with open(path, "w") as f:
        f.write(f"# name={name}\n")
        f.write(f"# width={w}\n")
        f.write(f"# height={h}\n")
        f.write(f"# tile_size={tile_size}\n")
        f.write(f"# time_limit={time_limit}\n")
        f.write(f"# minion_cap={minion_cap}\n")
        for row in grid:
            f.write("".join(row) + "\n")
    print(f"Written: {path}  ({w}x{h})")


def place_tree_border(g, w, h):
    """Solid Tree border on all four edges."""
    for x in range(w):
        g[0][x] = "T"
        g[h - 1][x] = "T"
    for y in range(h):
        g[y][0] = "T"
        g[y][w - 1] = "T"


def place_deploy_zone(g, w, h, depth):
    """
    Fill the inner band of the border with D (deploy) tiles.
    Leaves the outermost Tree border intact.
    Returns list of entrance candidate columns/rows.
    """
    # Top and bottom bands
    for y in range(1, depth + 1):
        for x in range(1, w - 1):
            g[y][x] = "D"
            g[h - 1 - y][x] = "D"
    # Left and right bands
    for x in range(1, depth + 1):
        for y in range(1, h - 1):
            g[y][x] = "D"
            g[y][w - 1 - x] = "D"


def place_entrances(g, w, h, depth, entrance_spacing=32):
    """
    Mark E tiles on the inner edge of the deploy zone where it meets the maze.
    Entrances spaced ~entrance_spacing tiles apart on each side.
    Returns list of (x, y) entrance positions.
    """
    entrances = []
    inner = depth + 1  # first maze row/col

    # Top entrances (row = depth, opening downward into maze)
    for x in range(inner, w - inner, entrance_spacing):
        g[depth][x] = "E"
        entrances.append((x, depth))

    # Bottom entrances
    for x in range(inner, w - inner, entrance_spacing):
        g[h - 1 - depth][x] = "E"
        entrances.append((x, h - 1 - depth))

    # Left entrances
    for y in range(inner, h - inner, entrance_spacing):
        g[y][depth] = "E"
        entrances.append((depth, y))

    # Right entrances
    for y in range(inner, h - inner, entrance_spacing):
        g[y][w - 1 - depth] = "E"
        entrances.append((w - 1 - depth, y))

    return entrances


def place_commander_start(g, depth):
    """Place Commander start tile near top-left of deploy zone."""
    cx = depth + 2
    cy = depth + 1
    g[cy][cx] = "C"
    return (cx, cy)


# ---------------------------------------------------------------------------
# Map 1: nexus_siege_512  — grid-of-rooms maze
# ---------------------------------------------------------------------------

def gen_nexus_siege(w, h):
    rng = random.Random(42)
    g = make_grid(w, h, "G")
    place_tree_border(g, w, h)
    place_deploy_zone(g, w, h, DEPLOY_DEPTH)

    interior_x0 = DEPLOY_DEPTH + 1
    interior_y0 = DEPLOY_DEPTH + 1
    interior_x1 = w - DEPLOY_DEPTH - 1
    interior_y1 = h - DEPLOY_DEPTH - 1

    # --- Grid-of-rooms maze ---
    # Rooms are 14 tiles wide, 14 tall, separated by 2-tile Tree walls with doorways.
    ROOM_W = 14
    ROOM_H = 14
    WALL = 2

    # Draw horizontal and vertical Tree dividers across the interior
    # Vertical dividers
    x = interior_x0 + ROOM_W
    while x < interior_x1 - WALL:
        for y in range(interior_y0, interior_y1):
            g[y][x] = "T"
            if x + 1 < interior_x1:
                g[y][x + 1] = "T"
        x += ROOM_W + WALL

    # Horizontal dividers
    y = interior_y0 + ROOM_H
    while y < interior_y1 - WALL:
        for x in range(interior_x0, interior_x1):
            g[y][x] = "T"
            if y + 1 < interior_y1:
                g[y + 1][x] = "T"
        y += ROOM_H + WALL

    # Cut doorways in each wall segment (one random gap per segment)
    DOOR_W = 3

    # Vertical wall doorways
    x = interior_x0 + ROOM_W
    while x < interior_x1 - WALL:
        # Cut doors at random y positions within each horizontal band
        band_y = interior_y0
        while band_y < interior_y1:
            band_end = min(band_y + ROOM_H + WALL, interior_y1)
            door_y = rng.randint(band_y + 1, max(band_y + 2, band_end - DOOR_W - 1))
            for dy in range(DOOR_W):
                if door_y + dy < interior_y1:
                    g[door_y + dy][x] = "G"
                    if x + 1 < interior_x1:
                        g[door_y + dy][x + 1] = "G"
            band_y += ROOM_H + WALL
        x += ROOM_W + WALL

    # Horizontal wall doorways
    y = interior_y0 + ROOM_H
    while y < interior_y1 - WALL:
        band_x = interior_x0
        while band_x < interior_x1:
            band_end = min(band_x + ROOM_W + WALL, interior_x1)
            door_x = rng.randint(band_x + 1, max(band_x + 2, band_end - DOOR_W - 1))
            for dx in range(DOOR_W):
                if door_x + dx < interior_x1:
                    g[y][door_x + dx] = "G"
                    if y + 1 < interior_y1:
                        g[y + 1][door_x + dx] = "G"
            band_x += ROOM_W + WALL
        y += ROOM_H + WALL

    # --- Mud patches in corridors (every 3rd room interior) ---
    room_col = 0
    cx = interior_x0
    while cx + ROOM_W < interior_x1:
        cy_row = 0
        cy = interior_y0
        while cy + ROOM_H < interior_y1:
            if (room_col + cy_row) % 3 == 2:
                # Fill most of this room with Mud
                for my in range(cy + 2, min(cy + ROOM_H - 2, interior_y1)):
                    for mx in range(cx + 2, min(cx + ROOM_W - 2, interior_x1)):
                        if g[my][mx] == "G":
                            g[my][mx] = "M"
            cy += ROOM_H + WALL
            cy_row += 1
        cx += ROOM_W + WALL
        room_col += 1

    # --- Lava in dead-end corners of some rooms ---
    lava_positions = []
    cx = interior_x0
    while cx + ROOM_W < interior_x1:
        cy = interior_y0
        while cy + ROOM_H < interior_y1:
            if rng.random() < 0.15:
                # 2x2 lava patch in corner
                lx = cx + rng.randint(2, ROOM_W - 4)
                ly = cy + rng.randint(2, ROOM_H - 4)
                for oy in range(2):
                    for ox in range(2):
                        if g[ly + oy][lx + ox] in ("G", "M"):
                            g[ly + oy][lx + ox] = "L"
                lava_positions.append((lx, ly))
            cy += ROOM_H + WALL
        cx += ROOM_W + WALL

    # --- Flag tiles: 6 locations spread across the map interior ---
    flag_positions = [
        (interior_x0 + (interior_x1 - interior_x0) * fx // 7,
         interior_y0 + (interior_y1 - interior_y0) * fy // 7)
        for (fx, fy) in [(1, 1), (3, 2), (5, 1), (2, 4), (4, 4), (6, 5)]
    ]
    for (fx, fy) in flag_positions:
        # Place flag on a Grass tile (not inside a wall)
        placed = False
        for dy in range(-2, 3):
            for dx in range(-2, 3):
                nx, ny = fx + dx, fy + dy
                if interior_x0 <= nx < interior_x1 and interior_y0 <= ny < interior_y1:
                    if g[ny][nx] in ("G", "M"):
                        g[ny][nx] = "F"
                        placed = True
                        break
            if placed:
                break

    place_entrances(g, w, h, DEPLOY_DEPTH, entrance_spacing=32)
    place_commander_start(g, DEPLOY_DEPTH)
    return g


# ---------------------------------------------------------------------------
# Map 2: benchmark_open_512  — open field, few obstacles
# ---------------------------------------------------------------------------

def gen_benchmark_open(w, h):
    rng = random.Random(7)
    g = make_grid(w, h, "G")
    place_tree_border(g, w, h)
    place_deploy_zone(g, w, h, DEPLOY_DEPTH)

    interior_x0 = DEPLOY_DEPTH + 1
    interior_y0 = DEPLOY_DEPTH + 1
    interior_x1 = w - DEPLOY_DEPTH - 1
    interior_y1 = h - DEPLOY_DEPTH - 1

    # Sparse Lava pillars (3x3) as obstacles
    for _ in range(80):
        px = rng.randint(interior_x0 + 5, interior_x1 - 8)
        py = rng.randint(interior_y0 + 5, interior_y1 - 8)
        for oy in range(3):
            for ox in range(3):
                g[py + oy][px + ox] = "L"

    # 2 Flag tiles near centre
    mid_x = w // 2
    mid_y = h // 2
    g[mid_y][mid_x - 20] = "F"
    g[mid_y][mid_x + 20] = "F"

    place_entrances(g, w, h, DEPLOY_DEPTH, entrance_spacing=48)
    place_commander_start(g, DEPLOY_DEPTH)
    return g


# ---------------------------------------------------------------------------
# Map 3: benchmark_maze_512  — dense maze, stress pathfinding
# ---------------------------------------------------------------------------

def gen_benchmark_maze(w, h):
    """
    Dense corridor maze using a recursive-division style layout.
    All corridors are 1 tile wide to maximise tree density and path complexity.
    """
    rng = random.Random(99)
    g = make_grid(w, h, "T")  # start all Tree
    place_tree_border(g, w, h)
    place_deploy_zone(g, w, h, DEPLOY_DEPTH)  # overwrites some Trees with D

    interior_x0 = DEPLOY_DEPTH + 1
    interior_y0 = DEPLOY_DEPTH + 1
    interior_x1 = w - DEPLOY_DEPTH - 1
    interior_y1 = h - DEPLOY_DEPTH - 1

    # Carve corridors on odd tile positions (standard maze carving grid)
    # Each "maze cell" is at (x*2+interior_x0, y*2+interior_y0)
    maze_cols = (interior_x1 - interior_x0) // 2
    maze_rows = (interior_y1 - interior_y0) // 2

    visited = [[False] * maze_cols for _ in range(maze_rows)]

    def carve(mx, my):
        visited[my][mx] = True
        wx = interior_x0 + mx * 2
        wy = interior_y0 + my * 2
        g[wy][wx] = "G"

        directions = [(0, -1), (0, 1), (-1, 0), (1, 0)]
        rng.shuffle(directions)
        for dx, dy in directions:
            nmx, nmy = mx + dx, my + dy
            if 0 <= nmx < maze_cols and 0 <= nmy < maze_rows and not visited[nmy][nmx]:
                # Carve wall between current and next
                g[wy + dy][wx + dx] = "G"
                carve(nmx, nmy)

    # Use iterative carving (recursive would hit Python stack limit for 256x256 maze cells)
    stack = [(0, 0)]
    visited[0][0] = True
    wx0 = interior_x0
    wy0 = interior_y0
    g[wy0][wx0] = "G"

    while stack:
        mx, my = stack[-1]
        wx = interior_x0 + mx * 2
        wy = interior_y0 + my * 2
        dirs = [(0, -1), (0, 1), (-1, 0), (1, 0)]
        rng.shuffle(dirs)
        moved = False
        for dx, dy in dirs:
            nmx, nmy = mx + dx, my + dy
            if 0 <= nmx < maze_cols and 0 <= nmy < maze_rows and not visited[nmy][nmx]:
                visited[nmy][nmx] = True
                g[wy + dy][wx + dx] = "G"  # carve wall
                nwx = interior_x0 + nmx * 2
                nwy = interior_y0 + nmy * 2
                g[nwy][nwx] = "G"          # carve cell
                stack.append((nmx, nmy))
                moved = True
                break
        if not moved:
            stack.pop()

    # Scatter Mud throughout (replace some Grass in corridors)
    for y in range(interior_y0, interior_y1):
        for x in range(interior_x0, interior_x1):
            if g[y][x] == "G" and rng.random() < 0.25:
                g[y][x] = "M"

    # 4 Flag tiles at far interior corners (find nearest Grass/Mud tile)
    targets = [
        (interior_x0 + (interior_x1 - interior_x0) // 4,
         interior_y0 + (interior_y1 - interior_y0) // 4),
        (interior_x1 - (interior_x1 - interior_x0) // 4,
         interior_y0 + (interior_y1 - interior_y0) // 4),
        (interior_x0 + (interior_x1 - interior_x0) // 4,
         interior_y1 - (interior_y1 - interior_y0) // 4),
        (interior_x1 - (interior_x1 - interior_x0) // 4,
         interior_y1 - (interior_y1 - interior_y0) // 4),
    ]
    for (fx, fy) in targets:
        placed = False
        for r in range(1, 10):
            for dy in range(-r, r + 1):
                for dx in range(-r, r + 1):
                    nx, ny = fx + dx, fy + dy
                    if interior_x0 <= nx < interior_x1 and interior_y0 <= ny < interior_y1:
                        if g[ny][nx] in ("G", "M"):
                            g[ny][nx] = "F"
                            placed = True
                            break
                if placed:
                    break
            if placed:
                break

    place_entrances(g, w, h, DEPLOY_DEPTH, entrance_spacing=32)
    place_commander_start(g, DEPLOY_DEPTH)
    return g


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    print("Generating nexus_siege_512.map ...")
    write_map(
        os.path.join(OUTPUT_DIR, "nexus_siege_512.map"),
        name="Nexus Siege",
        time_limit=180,
        minion_cap=200,
        tile_size=32,
        grid=gen_nexus_siege(W, H),
    )

    print("Generating benchmark_open_512.map ...")
    write_map(
        os.path.join(OUTPUT_DIR, "benchmark_open_512.map"),
        name="Open Field Benchmark",
        time_limit=120,
        minion_cap=500,
        tile_size=32,
        grid=gen_benchmark_open(W, H),
    )

    print("Generating benchmark_maze_512.map ...")
    write_map(
        os.path.join(OUTPUT_DIR, "benchmark_maze_512.map"),
        name="Maze Benchmark",
        time_limit=300,
        minion_cap=100,
        tile_size=32,
        grid=gen_benchmark_maze(W, H),
    )

    print("Done.")
