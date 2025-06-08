import os
import json
import shutil

source_dir = "enhanced_wbs_yaml_files/"
valid_dir = "valid_wbs_files/"
error_dir = "error_wbs_files/"
error_summary_path = "wbs_error_summary.json" # Assuming this is at the repo root

actions_log = {
    "copied_to_valid": [],
    "copied_to_error": [],
    "deleted_from_source": [],
    "skipped_in_source": [], # For files not matching .yaml or being START.yaml
    "not_processed_from_source_list": [], # For files that were in source_files_to_process but not copied
    "errors": []
}

print("Starting file segregation process...")

# 1. Ensure destination directories exist
for dir_path in [valid_dir, error_dir]:
    if not os.path.exists(dir_path):
        try:
            os.makedirs(dir_path)
            print(f"Created missing destination directory: {dir_path}")
        except Exception as e:
            error_msg = f"Error creating destination directory {dir_path}: {e}. Aborting."
            actions_log["errors"].append(error_msg)
            print(error_msg)
            # Exit if critical directories can't be made
            with open("file_segregation_log.json", 'w') as f: json.dump(actions_log, f, indent=2)
            exit()

# 2. Read wbs_error_summary.json
error_filenames = set()
if os.path.exists(error_summary_path):
    try:
        with open(error_summary_path, 'r') as f:
            summary_data = json.load(f)
            for error_entry in summary_data.get("error_files", []):
                if error_entry.get("filename"):
                    error_filenames.add(error_entry["filename"])
        print(f"Successfully loaded {len(error_filenames)} unique error filenames from {error_summary_path}")
    except Exception as e:
        actions_log["errors"].append(f"Error reading or parsing {error_summary_path}: {e}")
        print(f"WARNING: Error reading or parsing {error_summary_path}: {e}. Segregation might be inaccurate.")
else:
    actions_log["errors"].append(f"{error_summary_path} not found. Cannot accurately determine error files.")
    print(f"WARNING: {error_summary_path} not found. Segregation might be inaccurate.")

# 3. List all relevant YAML files in source_dir
source_files_to_process = []
if not os.path.isdir(source_dir):
    actions_log["errors"].append(f"Source directory {source_dir} not found.")
    print(f"ERROR: Source directory {source_dir} not found. Aborting.")
else:
    try:
        all_source_items = os.listdir(source_dir)
        for item_name in all_source_items:
            if item_name.endswith(".yaml") and item_name.lower() != "start.yaml":
                 source_files_to_process.append(item_name)
            elif item_name.lower() == "start.yaml" or not item_name.endswith(".yaml"):
                 actions_log["skipped_in_source"].append(item_name) # Log non-processed files like START.yaml

        print(f"Found {len(source_files_to_process)} YAML files to process in {source_dir} (excluding START.yaml and non-YAMLs).")
        if actions_log["skipped_in_source"]:
            print(f"Skipped files in source (e.g. START.yaml, non-YAMLs): {actions_log['skipped_in_source']}")

    except Exception as e:
        actions_log["errors"].append(f"Error listing files in {source_dir}: {e}")
        print(f"ERROR: Error listing files in {source_dir}: {e}. Aborting.")
        source_files_to_process = [] # Ensure we don't proceed

# Proceed only if source listing was successful
if "Error listing files" not in "".join(str(actions_log["errors"])) and os.path.isdir(source_dir) :
    # 4. Copy files to valid_dir or error_dir
    successfully_copied_filenames = set()
    for filename in source_files_to_process:
        source_filepath = os.path.join(source_dir, filename)
        try:
            if filename in error_filenames:
                dest_filepath = os.path.join(error_dir, filename)
                shutil.copy2(source_filepath, dest_filepath)
                actions_log["copied_to_error"].append(filename)
                successfully_copied_filenames.add(filename)
            else:
                dest_filepath = os.path.join(valid_dir, filename)
                shutil.copy2(source_filepath, dest_filepath)
                actions_log["copied_to_valid"].append(filename)
                successfully_copied_filenames.add(filename)
        except Exception as e:
            error_msg = f"Error copying file {filename}: {e}"
            actions_log["errors"].append(error_msg)
            print(f"ERROR: {error_msg}")
            # If copy failed, it won't be added to successfully_copied_filenames, so won't be deleted.

    print(f"Successfully copied {len(actions_log['copied_to_valid'])} files to {valid_dir}")
    print(f"Successfully copied {len(actions_log['copied_to_error'])} files to {error_dir}")

    # 5. Delete original files from source_dir that were successfully copied
    deleted_count = 0
    for filename in successfully_copied_filenames:
        original_filepath = os.path.join(source_dir, filename)
        try:
            os.remove(original_filepath)
            actions_log["deleted_from_source"].append(filename)
            deleted_count +=1
        except Exception as e:
            error_msg = f"Error deleting original file {filename} from {source_dir}: {e}"
            actions_log["errors"].append(error_msg)
            print(f"ERROR: {error_msg}")
    print(f"Successfully deleted {deleted_count} files from {source_dir}.")

    # Final check for files that were meant to be processed but weren't copied
    for filename in source_files_to_process:
        if filename not in successfully_copied_filenames:
            actions_log["not_processed_from_source_list"].append(filename)
    if actions_log["not_processed_from_source_list"]:
        print(f"WARNING: Files listed for processing but not successfully copied (and thus not deleted): {actions_log['not_processed_from_source_list']}")


# Save the log
log_path = "file_segregation_log.json"
try:
    with open(log_path, 'w') as f:
        json.dump(actions_log, f, indent=2)
    print(f"File segregation log saved to {log_path}")
except Exception as e:
    # This is a fallback print, as primary logging is to actions_log itself
    print(f"Critical error saving segregation log to {log_path}: {e}")


if actions_log["errors"]:
    print("\n--- Segregation Completed with Errors/Warnings ---")
    for err_msg in actions_log["errors"]:
        print(f"- {err_msg}")
    print("Review the log file and console output for details.")
else:
    print("\nFile segregation completed successfully.")

print("Segregation script finished.")
