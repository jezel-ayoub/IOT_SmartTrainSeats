"""
Smart Train Seat Guide - journey simulation
===========================================

QUESTION THIS ANSWERS
    How much time does a train journey save when passengers on the platform
    can see which carriage has free seats, instead of boarding blind?

HOW IT WORKS, IN ONE PARAGRAPH
    A train runs along a line and stops at every station. At each stop some
    passengers get off and some get on. How the boarders spread across the
    carriages decides how long the doors must stay open: the train cannot
    leave until the SLOWEST carriage has finished, so the busiest carriage
    sets the dwell time for the whole train. We run the same journey twice -
    once with passengers boarding blind, once with them guided by a platform
    display - and compare the total time spent standing at stations.

WHERE THE NUMBERS COME FROM
    Boarding distribution, measured in the Stockholm metro:
        55.3% / 25.4% / 18.8% across three carriages.
        Zhang, Jenelius & Kottenhoff (2017), Public Transport 9(3), 483-499.

    Effect of a platform crowding display, measured in the same study:
        boardings into the most crowded car fell by 4.3 percentage points,
        boardings into the emptier car rose by 4.1 percentage points.

    Dwell time as a function of passengers, measured for light rail:
        DT = 9.07 + 1.15 x boarding + 0.63 x alighting   (seconds)
        Lin, T. & Wilson, N.H.M. "Dwell Time Relationships for Light Rail
        Systems", Transportation Research Record 1361, 287-295, model A1.

ASSUMPTIONS WE MAKE, AND THEY ARE ASSUMPTIONS
    1. The train's dwell equals the dwell of its busiest carriage. This is the
       mechanism the whole result rests on. It is reasonable - doors close
       together - but it is our modelling choice, not a measured fact.
    2. Lin & Wilson measured light rail, not heavy commuter rail. The shape of
       the relationship transfers; the exact coefficients may not.
    3. Alighting passengers leave each carriage in proportion to how full it
       is, which is the simplest honest assumption.
    4. The guided scenario assumes passengers respond exactly as the Stockholm
       pilot measured. PERFECT_INFO is the theoretical ceiling if every
       passenger followed the display.

TO RUN
    python3 train_boarding_simulation.py

    It prints a summary and writes four charts next to itself.
"""

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

# ---------------------------------------------------------------------------
# PARAMETERS - change these and run again
# ---------------------------------------------------------------------------

CARS = 3
STATION_NAMES = [f"T{i}" for i in range(1, 13)]
STATIONS = len(STATION_NAMES)

# A commuter line into a city: heavy boarding early, heavy alighting late.
BOARDINGS = [84, 72, 65, 58, 46, 34, 22, 16, 10, 6, 4, 0]
ALIGHTINGS = [0, 4, 6, 10, 18, 28, 38, 48, 56, 64, 72, 71]

# How boarders spread across the carriages, in each scenario.
DIST_BLIND = [0.553, 0.254, 0.188]          # measured, no information

SHIFT_OFF_BUSIEST = 0.043                   # measured effect of a display
SHIFT_ONTO_EMPTIEST = 0.041
DIST_GUIDED = [
    DIST_BLIND[0] - SHIFT_OFF_BUSIEST,
    DIST_BLIND[1] + (SHIFT_OFF_BUSIEST - SHIFT_ONTO_EMPTIEST),
    DIST_BLIND[2] + SHIFT_ONTO_EMPTIEST,
]
DIST_PERFECT = [1 / CARS] * CARS            # theoretical ceiling

# Lin & Wilson (1992), model A1, one-car trains. Seconds.
DWELL_CONST = 9.07
DWELL_PER_BOARDING = 1.15
DWELL_PER_ALIGHTING = 0.63

SCENARIOS = {
    "blind": ("Boarding blind", DIST_BLIND),
    "guided": ("Guided by the display", DIST_GUIDED),
    "perfect": ("Perfect information", DIST_PERFECT),
}

# ---------------------------------------------------------------------------
# THE SIMULATION
# ---------------------------------------------------------------------------


def car_dwell(boarding, alighting):
    """Seconds this carriage needs at the platform."""
    return DWELL_CONST + DWELL_PER_BOARDING * boarding + DWELL_PER_ALIGHTING * alighting


def run_journey(distribution, boardings, alightings):
    """
    Run one train along the whole line.

    Returns the dwell time at each station, how many people boarded each
    carriage in total, and how full each carriage was as the journey went on.
    """
    occupancy = [0.0] * CARS
    dwells = []
    boarded_per_car = [0.0] * CARS
    occupancy_history = []

    for stop in range(STATIONS):
        # --- people get off, in proportion to how full each carriage is ---
        on_board = sum(occupancy)
        if on_board > 0:
            leaving = [alightings[stop] * c / on_board for c in occupancy]
        else:
            leaving = [0.0] * CARS
        leaving = [min(l, c) for l, c in zip(leaving, occupancy)]
        occupancy = [c - l for c, l in zip(occupancy, leaving)]

        # --- people get on, spread by the scenario's distribution ---
        joining = [boardings[stop] * share for share in distribution]
        occupancy = [c + j for c, j in zip(occupancy, joining)]
        boarded_per_car = [t + j for t, j in zip(boarded_per_car, joining)]

        # --- the train waits for its slowest carriage ---
        dwells.append(max(car_dwell(j, l) for j, l in zip(joining, leaving)))
        occupancy_history.append(list(occupancy))

    return dwells, boarded_per_car, occupancy_history


def total_dwell(distribution, scale=1.0):
    """Total seconds spent at platforms, with demand scaled by `scale`."""
    b = [x * scale for x in BOARDINGS]
    a = [x * scale for x in ALIGHTINGS]
    return sum(run_journey(distribution, b, a)[0])


# ---------------------------------------------------------------------------
# CHART STYLE
# ---------------------------------------------------------------------------

SURF, INK, DIM, GRID = "#fcfcfb", "#1b1f1d", "#676e6b", "#e6e4de"
BLUE, ORANGE, TEAL = "#2D6DB4", "#D4761A", "#13A085"
COLOUR = {"blind": BLUE, "guided": ORANGE, "perfect": TEAL}


def frame(ax, title, sub=None, xlab=None, ylab=None):
    ax.set_facecolor(SURF)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    for s in ("left", "bottom"):
        ax.spines[s].set_color(GRID)
        ax.spines[s].set_linewidth(1.1)
    ax.grid(True, color=GRID, linewidth=0.9)
    ax.set_axisbelow(True)
    ax.tick_params(colors=DIM, labelsize=10, length=0)
    if xlab:
        ax.set_xlabel(xlab, color=DIM, fontsize=10.5, labelpad=8)
    if ylab:
        ax.set_ylabel(ylab, color=DIM, fontsize=10.5, labelpad=8)
    ax.set_title(title, color=INK, fontsize=14.5, fontweight="bold",
                 loc="left", pad=34 if sub else 10)
    if sub:
        ax.text(0, 1.012, sub, transform=ax.transAxes, color=DIM,
                fontsize=11, va="bottom")


def save(fig, name):
    fig.tight_layout()
    fig.savefig(name, facecolor=SURF, dpi=160)
    plt.close(fig)
    print(f"  wrote {name}")


# ---------------------------------------------------------------------------
# RUN AND REPORT
# ---------------------------------------------------------------------------

def main():
    results = {}
    for key, (_, dist) in SCENARIOS.items():
        dwells, boarded, occ = run_journey(dist, BOARDINGS, ALIGHTINGS)
        results[key] = dict(dwells=dwells, boarded=boarded, occupancy=occ,
                            total=sum(dwells))

    blind_total = results["blind"]["total"]
    guided_total = results["guided"]["total"]
    perfect_total = results["perfect"]["total"]
    saved = blind_total - guided_total
    saved_perfect = blind_total - perfect_total

    print()
    print("=" * 62)
    print("  SMART TRAIN SEAT GUIDE - JOURNEY SIMULATION")
    print("=" * 62)
    print(f"  Line: {STATIONS} stations, {CARS} carriages")
    print(f"  Passengers boarding over the journey: {sum(BOARDINGS)}")
    print()
    print(f"  Boarding blind          {blind_total:7.1f} s "
          f"({blind_total/60:.1f} min at platforms)")
    print(f"  Guided by the display   {guided_total:7.1f} s "
          f"({guided_total/60:.1f} min)")
    print(f"  Perfect information     {perfect_total:7.1f} s "
          f"({perfect_total/60:.1f} min)")
    print()
    print(f"  TIME SAVED PER JOURNEY  {saved:7.1f} s "
          f"({saved/blind_total*100:.1f}% of platform time)")
    print(f"  Theoretical ceiling     {saved_perfect:7.1f} s "
          f"({saved_perfect/blind_total*100:.1f}%)")
    print()
    print(f"  Average per station:    {saved/STATIONS:.1f} s")
    print("=" * 62)
    print()

    # --- 1. where passengers board -----------------------------------------
    fig, ax = plt.subplots(figsize=(9.6, 5.2), dpi=160, facecolor=SURF)
    frame(ax, "Where passengers board",
          "Share of all boarders entering each carriage",
          ylab="Share of boarders (%)")
    x = np.arange(CARS)
    w = 0.26
    for i, key in enumerate(("blind", "guided", "perfect")):
        label, dist = SCENARIOS[key]
        vals = [d * 100 for d in dist]
        bars = ax.bar(x + (i - 1) * w, vals, w * 0.88, color=COLOUR[key],
                      label=label, zorder=3)
        for b, v in zip(bars, vals):
            ax.text(b.get_x() + b.get_width() / 2, v + 1.0, f"{v:.1f}",
                    ha="center", color=DIM, fontsize=9.5)
    ax.set_xticks(x)
    ax.set_xticklabels([f"Carriage {i+1}" for i in range(CARS)])
    ax.set_ylim(0, 62)
    ax.legend(frameon=False, labelcolor=DIM, fontsize=10.5, loc="upper right")
    save(fig, "sim_1_boarding_distribution.png")

    # --- 2. dwell time at each station -------------------------------------
    fig, ax = plt.subplots(figsize=(9.6, 5.2), dpi=160, facecolor=SURF)
    frame(ax, "Time spent at each station",
          "The train waits for its slowest carriage",
          xlab="Station", ylab="Dwell time (seconds)")
    xs = np.arange(STATIONS)
    for key in ("blind", "guided"):
        ax.plot(xs, results[key]["dwells"], color=COLOUR[key], lw=2.2,
                marker="o", markersize=5, label=SCENARIOS[key][0])
    ax.fill_between(xs, results["guided"]["dwells"], results["blind"]["dwells"],
                    color=ORANGE, alpha=0.10, zorder=1)
    ax.set_xticks(xs)
    ax.set_xticklabels(STATION_NAMES)
    ax.legend(frameon=False, labelcolor=DIM, fontsize=10.5)
    save(fig, "sim_2_dwell_per_station.png")

    # --- 3. cumulative saving ----------------------------------------------
    fig, ax = plt.subplots(figsize=(9.6, 5.2), dpi=160, facecolor=SURF)
    frame(ax, "Time saved, adding up along the line",
          "Difference from boarding blind, accumulated station by station",
          xlab="Station", ylab="Seconds saved so far")
    for key in ("guided", "perfect"):
        diff = np.cumsum(np.array(results["blind"]["dwells"])
                         - np.array(results[key]["dwells"]))
        ax.plot(xs, diff, color=COLOUR[key], lw=2.3, marker="o", markersize=5,
                label=SCENARIOS[key][0])
        ax.annotate(f"{diff[-1]:.0f} s", xy=(xs[-1], diff[-1]),
                    xytext=(-6, 10), textcoords="offset points",
                    color=COLOUR[key], fontweight="bold", fontsize=11.5,
                    ha="right")
    ax.set_xticks(xs)
    ax.set_xticklabels(STATION_NAMES)
    ax.legend(frameon=False, labelcolor=DIM, fontsize=10.5, loc="upper left")
    save(fig, "sim_3_cumulative_saving.png")

    # --- 4. does it still help on a busier train? --------------------------
    fig, ax = plt.subplots(figsize=(9.6, 5.2), dpi=160, facecolor=SURF)
    frame(ax, "Does it still help when the train is busier?",
          "Demand scaled up and down from the baseline journey",
          xlab="Passenger demand, relative to the baseline",
          ylab="Seconds saved per journey")
    scales = np.linspace(0.3, 2.2, 40)
    for key in ("guided", "perfect"):
        ys = [total_dwell(DIST_BLIND, s) - total_dwell(SCENARIOS[key][1], s)
              for s in scales]
        ax.plot(scales, ys, color=COLOUR[key], lw=2.3,
                label=SCENARIOS[key][0])
    ax.axvline(1.0, color=DIM, lw=1.1, ls=(0, (5, 4)))
    ax.text(1.03, ax.get_ylim()[1] * 0.93, "baseline", color=DIM, fontsize=10)
    ax.legend(frameon=False, labelcolor=DIM, fontsize=10.5, loc="upper left")
    save(fig, "sim_4_sensitivity.png")

    print("Done.\n")


if __name__ == "__main__":
    main()
