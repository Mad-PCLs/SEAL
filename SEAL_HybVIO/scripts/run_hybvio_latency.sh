#!/bin/bash

# Check if the --restrict argument is provided
RESTRICT=false
if [[ "$1" == "--restrict" ]]; then
    RESTRICT=true
fi

NUM_RUNS=5

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
OUT_ROOT_FOLDER="../output_latency"

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
            -gpu=false \
            -timer=true \
            -displayVideo=false"

        # Prepend taskset -c 0 if --restrict is provided
        if $RESTRICT; then
            COMMAND="taskset -c 0 $COMMAND"
        fi

        # Remove newlines and extra spaces from the command
        CLEANED_COMMAND=$(echo "$COMMAND" | tr -d '\n' | sed 's/  */ /g')

        # Output the cleaned command to the stdout file, followed by a newline
        echo "Command: $CLEANED_COMMAND" > "$STDOUT_FILE"
        echo "" >> "$STDOUT_FILE"  # Add a newline after the command

        # Execute the command and append stdout and stderr to the file
        eval "$COMMAND" >> "$STDOUT_FILE" 2>&1

        sleep 5
    done
    echo ""
done

# Parse the raw outputs offline using python
python3 -m latency.parse_output_latency
