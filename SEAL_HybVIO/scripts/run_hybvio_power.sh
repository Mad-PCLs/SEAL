#!/bin/bash

# Check if the --vanilla flag is provided
VANILLA=false
if [[ "$1" == "--vanilla" ]]; then
    VANILLA=true
fi

NUM_RUNS=1

DATASET_ROOT_PATH="../data/benchmark/"
DATASET_SEQUENCE_NAMES=(
    "euroc-mh-01-easy"
    "euroc-mh-02-easy"
    "euroc-mh-03-medium"
    "euroc-mh-04-difficult"
    "euroc-mh-05-difficult"
    "euroc-v1-01-easy"
    "euroc-v1-02-medium"
    "euroc-v1-03-difficult"
    "euroc-v2-01-easy"
    "euroc-v2-02-medium"
    "euroc-v2-03-difficult"
)

HYBVIO_BINARY="../HybVIO/build/main"
ORB_VOCAB_PATH="../HybVIO/data/orb_vocab.dbow2"
SERIALIZED_FOLDERS_PATH="../HybVIO/data/seal_serial_storage/optical_flow"
OUT_ROOT_FOLDER="../output_power"

mkdir -p "$OUT_ROOT_FOLDER"

# Function to clean up files on interruption
cleanup() {
    echo "Script interrupted. Cleaning up files for the current iteration..."
    if [[ -f "$STDOUT_FILE" ]]; then
        rm -f "$STDOUT_FILE"
        echo "Deleted $STDOUT_FILE"
    fi
    if [[ -f "$HYBVIO_OUT_FILE" ]]; then
        rm -f "$HYBVIO_OUT_FILE"
        echo "Deleted $HYBVIO_OUT_FILE"
    fi
    exit 1
}

# Set up trap to call cleanup function on SIGINT (Ctrl+C) or SIGTERM
trap cleanup SIGINT SIGTERM

# Inform the user about the power measurements
echo -e "\e[33mNote: The power measurements reported in the paper were measured using an external power meter and a RPi4."
echo -e "This script does not perform or parse any measurements.\e[0m"

# Ensure that the optical flow results have been pre-computed.
if [[ "$VANILLA" == false ]]; then
    python3 -m power.precompute_optical_flow
    OUT_ROOT_FOLDER="$OUT_ROOT_FOLDER/load"
else
    OUT_ROOT_FOLDER="$OUT_ROOT_FOLDER/vanilla"
fi

echo ""
# Source: https://unix.stackexchange.com/questions/293940/how-can-i-make-press-any-key-to-continue
read -n 1 -s -r -p $'\e[1mPress any key to continue with the power experiments.\e[0m'
echo ""
echo ""

for ((i=1; i<=NUM_RUNS; i++)); do
    echo "Iteration number: $i"
    echo "-----------------------"
    # Iterate over all the datasets and run the experiment
    for DATASET_SEQUENCE in "${DATASET_SEQUENCE_NAMES[@]}"; do
        echo "Dataset Running: $DATASET_SEQUENCE"

        # Create the output folder to store the raw outputs
        STDOUT_DIR="$OUT_ROOT_FOLDER/$DATASET_SEQUENCE/stdout"
        ESTIM_TRAJ_DIR="$OUT_ROOT_FOLDER/$DATASET_SEQUENCE/estim_traj"

        # Ensure the directories exist
        mkdir -p "$STDOUT_DIR"
        mkdir -p "$ESTIM_TRAJ_DIR"

        # Define the .txt file to store experiment results in
        STDOUT_FILE="$STDOUT_DIR/$i.txt"
        HYBVIO_OUT_FILE="$ESTIM_TRAJ_DIR/$i.txt"

        # Check if both files exist; if not, run the experiment
        if [[ -f "$STDOUT_FILE" && -f "$HYBVIO_OUT_FILE" ]]; then
            echo "Files $STDOUT_FILE and $HYBVIO_OUT_FILE exist. Continuing..."
            continue
        fi

        # Construct the command to execute the hybvio binary
        COMMAND="$HYBVIO_BINARY \
            -vocabularyPath=\"$ORB_VOCAB_PATH\" \
            -i=\"$DATASET_ROOT_PATH/$DATASET_SEQUENCE\" \
            -o=\"$HYBVIO_OUT_FILE\" \
            -displayVideo=false"

        # Add optical flow arguments if not running in vanilla mode
        if [[ "$VANILLA" == false ]]; then
            COMMAND="$COMMAND \
                -sealPowerEmulatorStoragePath=\"$SERIALIZED_FOLDERS_PATH\" \
                -sealPowerEmulatorMode=\"load\""
        else
            COMMAND="$COMMAND \
                -sealPowerEmulatorMode=\"vanilla\""
        fi

        CLEANED_COMMAND=$(echo "$COMMAND" | tr -d '\n' | sed 's/  */ /g')

        echo "Command: $CLEANED_COMMAND" > "$STDOUT_FILE"
        echo "" >> "$STDOUT_FILE"

        # Execute the command and append stdout and stderr to the file
        eval "$COMMAND" >> "$STDOUT_FILE" 2>&1

        read -n 1 -s -r -p $'\e[1mPress any key to continue to the next sequence.\e[0m'
        echo ""
        echo ""
    done
done
