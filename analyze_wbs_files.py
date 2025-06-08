import os
import yaml
import json

wbs_dir = "enhanced_wbs_yaml_files/"
error_files_summary = []
processed_files_count = 0
yaml_files_count = 0 # Initialize to handle case where dir might not exist or is empty

print(f"Starting analysis. Target directory: {wbs_dir}")

# Clean up any test files that might have been created in previous runs
test_files_to_remove = [
    "valid_task.yaml", "task_no_id.yaml", "task_no_title.yaml",
    "task_non_dict.yaml", "task_deps_string.yaml", "task_deps_int.yaml",
    "task_deps_list_mixed.yaml", "malformed.yaml",
    "task1.yaml", "task2.yaml", "task3.yaml" # from even earlier runs
]

if not os.path.exists(wbs_dir):
    print(f"Warning: Directory {wbs_dir} not found. Skipping cleanup and analysis.")
else:
    print("Attempting to remove known test files...")
    for test_file in test_files_to_remove:
        file_path = os.path.join(wbs_dir, test_file)
        if os.path.exists(file_path):
            try:
                os.remove(file_path)
                print(f"Successfully removed test file: {file_path}")
            except Exception as e:
                print(f"Error removing test file {file_path}: {e}")
        # else:
            # print(f"Test file not found (already removed or never created): {file_path}")
    print("Test file cleanup attempt complete.")

try:
    if not os.path.exists(wbs_dir) or not os.path.isdir(wbs_dir):
        print(f"Error: Directory '{wbs_dir}' not found or is not a directory. Analysis cannot proceed.")
        # Initialize yaml_files to empty list for consistent JSON output structure
        yaml_files = []
    else:
        all_files = os.listdir(wbs_dir)
        yaml_files = [f for f in all_files if f.endswith(".yaml") and f != "START.yaml"]
        yaml_files_count = len(yaml_files)

    if not yaml_files: # Checks if list is empty
        print("No YAML files found in the directory to analyze (excluding START.yaml).")
    else:
        print(f"Found {yaml_files_count} YAML files to analyze (excluding START.yaml).")
        for filename in yaml_files:
            filepath = os.path.join(wbs_dir, filename)
            try:
                with open(filepath, 'r') as f:
                    content = yaml.safe_load(f)

                if not isinstance(content, dict):
                    error_files_summary.append({"filename": filename, "error": "Content is not a dictionary"})
                    continue

                if content.get("id") is None:
                    error_files_summary.append({"filename": filename, "error": "'id' is missing"})
                    continue

                processed_files_count += 1

            except yaml.YAMLError as e:
                # Simplify YAMLError message for summary if it's too long
                error_msg = str(e).split('\\n')[0] # Take first line of error
                error_files_summary.append({"filename": filename, "error": f"YAML parsing error: {error_msg}"})
            except Exception as e:
                error_files_summary.append({"filename": filename, "error": f"General error reading file: {e}"})

        print(f"Analysis complete. {processed_files_count} out of {yaml_files_count} files appear processable.")
        if error_files_summary:
            print(f"Found issues in {len(error_files_summary)} files:")
            # Limit printing details if too many errors for console
            for i, err_file in enumerate(error_files_summary):
                if i < 10: # Print details for first 10 errors
                    print(f"  - Filename: {err_file['filename']}, Issue: {err_file['error']}")
                elif i == 10:
                    print(f"  ... and {len(error_files_summary) - 10} more error(s) not detailed here.")
                    break
        else:
            print("No issues found in any remaining WBS YAML files.")

except Exception as e:
    print(f"An critical error occurred during the analysis: {e}")
    # Ensure error_files_summary list is available
    if 'error_files_summary' not in locals():
        error_files_summary = []
    error_files_summary.append({"filename": "N/A", "error": f"Critical script error: {e}"})


# Store the summary of problematic files
output_data = {
    "error_files": error_files_summary,
    "processed_count": processed_files_count,
    "total_analyzed": yaml_files_count
}

with open('wbs_error_summary.json', 'w') as f:
    json.dump(output_data, f, indent=2)

if error_files_summary:
    print(f"Error summary (with {len(error_files_summary)} issues) saved to wbs_error_summary.json")
else:
    print(f"No errors detected in {yaml_files_count} files analyzed. Summary file saved to wbs_error_summary.json")
