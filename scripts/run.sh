#!/bin/bash

# input handling is AI-generated & tested
# Best-friend: man <command>

# ========================RUN IDENTIFIER: easier tracking=================================

# -e enables interpreation of '\' character in '\n'
echo -e "\n--- Experiment Configuration ---"
read -p "Enter run name (Press Enter for timestamp default): " RUN_NAME

if [ -z "$RUN_NAME" ]; then
    # Generates a clean filename-safe timestamp (YYYYMMDD_HHMMSS)
    RUN_NAME=$(date +"%Y%m%d_%H%M%S")
    echo "Using default timestamp name: $RUN_NAME"
else
    # Optional: sanitize input to remove spaces/special characters if needed
    RUN_NAME=$(echo "$RUN_NAME" | tr ' ' '-')
    echo "Using run name: $RUN_NAME"
fi

# Decide on the cores with constraints: non-sibling
MAX_CORES_LIMIT=5

CHOSEN_CORES=()
DISABLED_CORES=()

contains_element() {
  local e match="$1"
  shift
  for e; do [[ "$e" == "$match" ]] && return 0; done
  return 1
}

while true; do
    read -p "Enter number of cores required (Maximum ${MAX_CORES_LIMIT}): " REQ_COUNT
    if [[ "$REQ_COUNT" =~ ^[0-9]+$ ]] && [ "$REQ_COUNT" -le "$MAX_CORES_LIMIT" ] && [ "$REQ_COUNT" -ge 0 ]; then
        break
    else
        echo "Invalid input. Please enter a number between 0 and ${MAX_CORES_LIMIT}."
    fi
done

for (( iteration=1; iteration<=REQ_COUNT; iteration++ )); do
    while true; do
        echo -e "\n--- Core Selection ($iteration of $REQ_COUNT) ---"
        read -p "Enter CPU core ID to allocate: " CORE_ID
        
        # Validate that input is a valid number
        # TODO : fix this 11
        if [[ "$REQ_COUNT" =~ ^[0-9]+$ ]] && [ "$REQ_COUNT" -gt 11 ] && [ "$REQ_COUNT" -lt 0 ]; then
            echo "Error: Please enter a valid numerical core ID."
            continue
        fi

        # Check if already chosen
        if contains_element "$CORE_ID" "${CHOSEN_CORES[@]}"; then
            echo "Error: Core $CORE_ID is already selected."
            continue
        fi

        # Check if disabled because its sibling pair was chosen
        if contains_element "$CORE_ID" "${DISABLED_CORES[@]}"; then
            echo "Error: Core $CORE_ID is disabled because its hyperthread sibling is active."
            continue
        fi

        # Core is valid, save it
        CHOSEN_CORES+=("$CORE_ID")

        # Calculate sibling core to disable: if even -> i+1, if odd -> i-1
        if (( CORE_ID % 2 == 0 )); then
            SIBLING=$(( CORE_ID + 1 ))
        else
            SIBLING=$(( CORE_ID - 1 ))
        fi
        
        DISABLED_CORES+=("$SIBLING")
        echo "Selected Core: $CORE_ID. (Sibling Core $SIBLING has been blacklisted)."
        break
    done
done

# Convert choices array to a comma-separated string for cpupower
CPU_STRING=$(IFS=,; echo "${CHOSEN_CORES[*]}")
echo -e "\nTarget execution cores: $CPU_STRING"

if [ -z "$CPU_STRING" ]; then
    echo "Error: Cores is empty / corrupted. Terminating script right-away."
    exit 1
fi

# -----------------------Core Orchestration-----------------------------

echo "Setting performance mode..."
sudo cpupower -c "$CPU_STRING" frequency-set -g performance

# Build the executable
cmake --build ./build

# thread priority change: privileged operation
sudo setcap cap_sys_nice=eip ./build/bin/main

# Run the executable: Provide run/temp directory as argument
# Or make it fixed

./build/bin/main "${CHOSEN_CORES[@]}"
EXIT_CODE=$?

echo "Restoring powersave mode..."
sudo cpupower -c "$CPU_STRING" frequency-set -g powersave

echo -e "\n--- Processing Output Data ($RUN_NAME) ---"

TEMP_DIR="run/temp"
TARGET_BASE_DIR="run/$RUN_NAME"

# Check if any CSV files were generated
if [ ! -d "$TEMP_DIR" ] || [ -z "$(ls -A $TEMP_DIR/*.csv 2>/dev/null)" ]; then
    echo "Warning: No CSV data found in $TEMP_DIR to process."
    exit $EXIT_CODE
fi

# Create target directory for this specific run
mkdir -p "$TARGET_BASE_DIR"

# Iterate over every generated experiment CSV file
for csv_path in "$TEMP_DIR"/*.csv; do
    # Extract filename without path and extension (e.g., "expt_1")
    filename=$(basename "$csv_path" .csv)
    
    # Create an isolation folder for this specific experiment
    expt_folder="$TARGET_BASE_DIR/$filename"
    mkdir -p "$expt_folder"
    
    # Move and rename the CSV file
    mv "$csv_path" "$expt_folder/data.csv"
    echo "Structured: $expt_folder/data.csv"
done
exit $EXIT_CODE
