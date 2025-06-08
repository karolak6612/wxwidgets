import os

wbs_dir = "enhanced_wbs_yaml_files/"
sample_files = ["task1.yaml", "task2.yaml", "task3.yaml"]

# Ensure the directory exists to avoid errors if it was somehow removed
if not os.path.exists(wbs_dir):
    print(f"Directory not found: {wbs_dir}")
    # If the directory doesn't exist, no files to remove from it.
    # Depending on requirements, could exit or create it.
    # For this cleanup, if no dir, then no files to clean.
    print("Cleanup of sample files complete (directory did not exist).")
else:
    for file_name in sample_files:
        file_path = os.path.join(wbs_dir, file_name)
        if os.path.exists(file_path):
            try:
                os.remove(file_path)
                print(f"Removed sample file: {file_path}")
            except Exception as e:
                print(f"Error removing file {file_path}: {e}")
        else:
            print(f"Sample file not found (already removed or never created): {file_path}")

    print("Cleanup of sample files complete.")
