import pandas as pd
import numpy as np
import sys

# Load CSV
csv_path = '../output/epe_errors.csv'
df = pd.read_csv(csv_path)

# Validation
expected_thresholds = [str(i) for i in range(20, 46)]
expected_sequence_count = 36

# Check format
missing_columns = [col for col in expected_thresholds if col not in df.columns]
wrong_row_count = df.shape[0] != expected_sequence_count
missing_data = df[expected_thresholds].isnull().all(axis=1).sum() > 0

if missing_columns or wrong_row_count or missing_data:
    print("Error: CSV file is not well formatted.")
    if missing_columns:
        print(f"Missing threshold columns: {missing_columns}")
    if wrong_row_count:
        print(f"Expected {expected_sequence_count} sequences but found {df.shape[0]}")
    if missing_data:
        print("One or more sequences are missing all threshold values.")
    print("Please run the SEAL optical flow simulator with no flags to generate results for all sequences and thresholds.")
    sys.exit(1)

baseline_column = "Baseline"

# Find the threshold with lowest mean EPE across all sequences
mean_epe_per_threshold = df[expected_thresholds].mean()
best_threshold = mean_epe_per_threshold.idxmin()
print(f"Best threshold (lowest mean EPE across all sequences): N = {best_threshold}")

# Select sequences where |baseline - fixed EPE| >= 2
selected_mask = (df[baseline_column] - df[best_threshold]).abs() >= 2
selected_sequences = df[selected_mask].index.tolist()

print(f"Selected sequences (|baseline - EPE @ N={best_threshold}| ≥ 2): {selected_sequences}")


# Step 3: Compute rounded values first
df_rounded = df.copy()
df_rounded[expected_thresholds + [baseline_column]] = df_rounded[expected_thresholds + [baseline_column]].round(2)
flexible_epe_rounded = df_rounded[expected_thresholds].min(axis=1)
best_threshold_per_seq = df_rounded[expected_thresholds].idxmin(axis=1)

# Build selected sequence result table (rounded values)
results = []
for seq in selected_sequences:
    baseline = df_rounded.loc[seq, baseline_column]
    fixed = df_rounded.loc[seq, best_threshold]
    flexible = flexible_epe_rounded[seq]
    best_N = best_threshold_per_seq[seq]
    results.append((seq, baseline, fixed, flexible, best_N))

res_df = pd.DataFrame(results, columns=["Sequence", "Base", f"Fixed (N = {best_threshold})", "Flexible (min)", "Best N"])

# Compute stats on rounded values
mean_base = round(res_df["Base"].mean(), 2)
mean_fixed = round(res_df[f"Fixed (N = {best_threshold})"].mean(), 2)
mean_flex = round(res_df["Flexible (min)"].mean(), 2)

std_base = round(res_df["Base"].std(ddof=0), 2)
std_fixed = round(res_df[f"Fixed (N = {best_threshold})"].std(ddof=0), 2)
std_flex = round(res_df["Flexible (min)"].std(ddof=0), 2)

# Global stats (on rounded values)
global_base = df_rounded[baseline_column]
global_fixed = df_rounded[best_threshold]
global_flexible = flexible_epe_rounded

global_mean_base = round(global_base.mean(), 2)
global_mean_fixed = round(global_fixed.mean(), 2)
global_mean_flex = round(global_flexible.mean(), 2)

global_std_base = round(global_base.std(ddof=0), 2)
global_std_fixed = round(global_fixed.std(ddof=0), 2)
global_std_flex = round(global_flexible.std(ddof=0), 2)

# Print final report
print("\nSelected Sequences Report:")
print(res_df[["Sequence", "Base", f"Fixed (N = {best_threshold})", "Flexible (min)", "Best N"]])

print("\nMean (selected):")
print(f"Base: {mean_base}, Fixed: {mean_fixed}, Flexible: {mean_flex}")
print("Std Dev (selected):")
print(f"Base: {std_base}, Fixed: {std_fixed}, Flexible: {std_flex}")

print("\nMean (all sequences):")
print(f"Base: {global_mean_base}, Fixed: {global_mean_fixed}, Flexible: {global_mean_flex}")
print("Std Dev (all sequences):")
print(f"Base: {global_std_base}, Fixed: {global_std_fixed}, Flexible: {global_std_flex}")