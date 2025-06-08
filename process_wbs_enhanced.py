import os
import yaml
import json

wbs_dir = "enhanced_wbs_yaml_files/"
wbs_files_data = []
error_files = []

try:
    # Ensure the directory exists
    if not os.path.isdir(wbs_dir):
        print(f"Error: Directory '{wbs_dir}' not found.")
        # Store empty results if dir not found, or handle as critical error
        with open('wbs_data.json', 'w') as f:
            json.dump({"processed_files": [], "error_files": [{"filename": wbs_dir, "error": "Directory not found"}]}, f)
        exit() # Exit if the directory is crucial and not found

    all_files = os.listdir(wbs_dir)
    yaml_files = [f for f in all_files if f.endswith(".yaml") and f != "START.yaml"]

    if not yaml_files:
        print("No YAML files found in the directory (excluding START.yaml).")
    else:
        print(f"Found {len(yaml_files)} YAML files to process (excluding START.yaml).")
        for filename in yaml_files:
            filepath = os.path.join(wbs_dir, filename)
            try:
                with open(filepath, 'r') as f:
                    content = yaml.safe_load(f)

                    if not isinstance(content, dict):
                        print(f"Error: Content of {filename} is not a dictionary. Skipping.")
                        error_files.append({"filename": filename, "error": "Content is not a dictionary"})
                        continue

                    file_id = content.get("id")
                    title = content.get("title")
                    dependencies = content.get('dependencies')

                    if file_id is None:
                        print(f"Error: 'id' is missing in {filename}. Skipping.")
                        error_files.append({"filename": filename, "error": "'id' is missing"})
                        continue

                    # Ensure id is stored as string
                    file_id = str(file_id)

                    if title is None:
                        print(f"Warning: 'title' is missing in {filename}. Using empty string for title.")
                        title = ""
                    else:
                        title = str(title) # Ensure title is string

                    if dependencies is None:
                        dependencies = []
                    elif not isinstance(dependencies, list):
                        # Convert to list if it's a single item. Also ensure it's a string.
                        dependencies = [str(dependencies)]
                    else:
                        # Ensure all elements in the list are strings
                        dependencies = [str(dep) for dep in dependencies]

                    wbs_files_data.append({
                        "id": file_id,
                        "title": title,
                        "dependencies": dependencies,
                        "filename": filename
                    })
            except yaml.YAMLError as e:
                print(f"Error parsing YAML file {filename}: {e}")
                error_files.append({"filename": filename, "error": f"YAML parsing error: {e}"})
            except Exception as e:
                print(f"Error reading or processing file {filename}: {e}")
                error_files.append({"filename": filename, "error": f"General error: {e}"})

        print(f"Successfully processed {len(wbs_files_data)} WBS files out of {len(yaml_files)} found.")
        if error_files:
            print(f"Encountered errors or validation issues in {len(error_files)} files:")
            for err_file in error_files:
                print(f"  - {err_file['filename']}: {err_file['error']}")

except Exception as e:
    print(f"An critical error occurred outside of file processing: {e}")
    # Ensure error_files list is available for dumping if this top-level try fails early
    if 'error_files' not in locals():
        error_files = [] # Initialize if not defined due to early critical error
    error_files.append({"filename": "N/A", "error": f"Critical error: {e}"})


# Store the result (even if there were errors, so we can inspect)
with open('wbs_data.json', 'w') as f:
    json.dump({"processed_files": wbs_files_data, "error_files": error_files}, f)

if error_files:
    print("Processing complete with errors/warnings. Check wbs_data.json and console output.")
else:
    print("Processing complete. All targeted files processed successfully.")
