import os
import shutil

source_file = "wbs_error_summary.json"
destination_dir = "error_wbs_files/"

# Ensure the destination directory name is just the directory, not part of the filename yet
if not os.path.isdir(destination_dir):
    print(f"Error: Destination directory '{destination_dir}' does not exist. Please create it first.")
    # Optionally, create it if it's missing, though the problem implies it should exist
    # os.makedirs(destination_dir)
    # print(f"Created destination directory '{destination_dir}'.")
else:
    destination_file_path = os.path.join(destination_dir, source_file) # Construct full dest path

    if os.path.exists(source_file):
        try:
            shutil.copy2(source_file, destination_file_path)
            print(f"Successfully copied '{source_file}' to '{destination_file_path}'.")

            # As per instructions, not deleting the original from the root.
            # If deletion were required:
            # try:
            #     os.remove(source_file)
            #     print(f"Successfully deleted original '{source_file}' from the root.")
            # except Exception as e:
            #     print(f"Error deleting original '{source_file}' from root: {e}")

        except Exception as e:
            print(f"Error copying '{source_file}' to '{destination_file_path}': {e}")
    else:
        print(f"Error: Source file '{source_file}' does not exist at the repository root.")

print("Copy script finished.")
