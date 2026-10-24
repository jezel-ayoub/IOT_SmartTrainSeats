# Journey simulation

A theoretical simulation that answers one question: **how much time does a train
journey save when passengers on the platform can see which carriage has free
seats, instead of boarding blind?**

This is the stage 3 deliverable. The simulated carriages on the live display
page are a separate thing and belong to stage 2.

## The result

A 12-station line with 417 passengers boarding over the journey:

| Scenario | Time at platforms | Saved |
|---|---|---|
| Boarding blind | 519 s | — |
| Guided by the display | 487 s | 32 s |
| Perfect information | 356 s | 164 s |

The interesting number is not the 32 seconds — it is the gap between 32 and 164.
The behaviour change measured in a real pilot captures only about a fifth of the
benefit that is theoretically available. The bottleneck is not the sensing; it is
how many passengers notice the display and act on it.

## How it works

A train runs along a line and stops at every station. At each stop some
passengers get off and some get on. How the boarders spread across the carriages
decides how long the doors must stay open, because the train cannot leave until
the slowest carriage has finished — so the busiest carriage sets the dwell time
for the whole train. The same journey is run three times and the total time spent
standing at platforms is compared.

## Where the numbers come from

| Input | Value | Source |
|---|---|---|
| Boarding distribution with no information | 55.3% / 25.4% / 18.8% | Zhang, Jenelius & Kottenhoff (2017) |
| Effect of a platform crowding display | −4.3 pp off the busiest car, +4.1 pp onto the emptiest | same study |
| Dwell time | `DT = 9.07 + 1.15 × boarding + 0.63 × alighting` seconds | Lin & Wilson, TRR 1361, model A1 |

- Zhang, Y., Jenelius, E. & Kottenhoff, K. (2017). Impact of real-time crowding
  information: a Stockholm metro pilot study. *Public Transport*, 9(3), 483–499.
- Lin, T. & Wilson, N. H. M. Dwell Time Relationships for Light Rail Systems.
  *Transportation Research Record*, 1361, 287–295.

## Running it

**macOS** — double-click `run_simulation.command`. The first run builds a Python
environment and takes a minute; every run after that is instant. One-time setup:

```
chmod +x run_simulation.command
```

**Any platform, manually:**

```
pip install matplotlib numpy
python3 train_boarding_simulation.py
```

**No installation at all** — paste the contents of the `.py` file into a Google
Colab notebook and run the cell.

## What it produces

A summary table printed to the terminal, and four charts:

| File | What it shows |
|---|---|
| `sim_1_boarding_distribution.png` | Where passengers board, in all three scenarios |
| `sim_2_dwell_per_station.png` | Dwell time at each station, with and without the system |
| `sim_3_cumulative_saving.png` | Seconds saved, accumulating along the line |
| `sim_4_sensitivity.png` | How the saving changes as the train gets busier |

## Changing it

Every parameter is at the top of the file: the number of carriages, the station
names, how many passengers board and alight at each stop, and the three dwell
coefficients. Edit, save, run again.

To add a scenario, add an entry to `SCENARIOS` with a name and a distribution
that sums to 1. The table and all four charts pick it up automatically.

## What this model does not capture

1. **The train's dwell equals its busiest carriage's dwell.** This is the
   mechanism the whole result rests on. It is reasonable, since doors close
   together, but it is a modelling choice rather than a measured fact.
2. **Lin & Wilson measured light rail**, not heavy commuter rail. The shape of
   the relationship transfers; the exact coefficients may not.
3. **The dwell model is linear in passenger count**, so the saving scales
   proportionally with demand. In reality there is a saturation point beyond
   which switching carriages stops helping — Peftitsi, Jenelius & Cats (2022)
   find exactly that. This is why `sim_4_sensitivity.png` is a straight line,
   and it would not be in the real world.
4. **Alighting passengers leave each carriage in proportion to how full it is**,
   which is the simplest honest assumption in the absence of data.
