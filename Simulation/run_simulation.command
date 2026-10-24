#!/bin/bash
# ---------------------------------------------------------------------------
# Smart Train Seat Guide - run the journey simulation
#
# Double-click this file to run the simulation. It handles the Python
# environment for you, so there is nothing to remember.
#
# The first run sets Python up and takes a minute. Every run after that
# is instant.
# ---------------------------------------------------------------------------

# Work in the folder this file is sitting in, wherever that may be.
cd "$(dirname "$0")" || exit 1

echo ""
echo "============================================================"
echo "  Smart Train Seat Guide - journey simulation"
echo "============================================================"
echo ""

# Set the Python environment up the first time, then reuse it.
if [ ! -d "$HOME/venv" ]; then
    echo "First run: setting Python up. This takes a minute, please wait."
    echo ""
    python3 -m venv "$HOME/venv" || {
        echo "Could not create the Python environment."
        echo "Check that Python is installed: type  python3 --version"
        read -r -p "Press Enter to close. "
        exit 1
    }
    "$HOME/venv/bin/pip" install --quiet --upgrade pip
    "$HOME/venv/bin/pip" install --quiet matplotlib numpy || {
        echo "Could not install matplotlib and numpy."
        read -r -p "Press Enter to close. "
        exit 1
    }
    echo "Python is ready."
    echo ""
fi

# Switch the environment on.
# shellcheck disable=SC1091
source "$HOME/venv/bin/activate"

# Run it.
python3 train_boarding_simulation.py || {
    echo ""
    echo "The simulation did not finish. The error is above."
    read -r -p "Press Enter to close. "
    exit 1
}

echo "The four charts have been saved into this same folder."
echo ""
read -r -p "Press Enter to close this window. "
