import os

dirs_to_create = ["valid_wbs_files", "error_wbs_files"]
for dir_name in dirs_to_create:
    if not os.path.exists(dir_name):
        try:
            os.makedirs(dir_name)
            print(f"Directory '{dir_name}' created successfully.")
        except Exception as e:
            print(f"Error creating directory '{dir_name}': {e}")
    else:
        print(f"Directory '{dir_name}' already exists.")

print("Directory creation script finished.")
