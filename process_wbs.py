import os
import yaml
import json

wbs_dir = "enhanced_wbs_yaml_files/"
wbs_files_data = []

try:
    all_files = os.listdir(wbs_dir)
    # Filter out START.yaml and ensure only .yaml files are processed
    yaml_files = [f for f in all_files if f.endswith(".yaml") and f != "START.yaml"]

    if not yaml_files:
        print("No YAML files found in the directory (excluding START.yaml).")
    else:
        for filename in yaml_files:
            filepath = os.path.join(wbs_dir, filename)
            try:
                with open(filepath, 'r') as f:
                    content = yaml.safe_load(f)
                    # Ensure dependencies is a list, even if null or missing
                    dependencies = content.get('dependencies')
                    if dependencies is None:
                        dependencies = []
                    elif not isinstance(dependencies, list):
                        # If dependencies is not a list (e.g. a string), make it a list
                        dependencies = [str(dependencies)]

                    wbs_files_data.append({
                        "id": content.get("id"),
                        "title": content.get("title"),
                        "dependencies": dependencies,
                        "filename": filename  # Keep filename for error reporting
                    })
            except yaml.YAMLError as e:
                print(f"Error parsing YAML file {filename}: {e}")
            except Exception as e:
                print(f"Error reading file {filename}: {e}")

        # Print the extracted data for verification
        for item in wbs_files_data:
            print(f"ID: {item['id']}, Title: {item['title']}, Dependencies: {item['dependencies']}, Filename: {item['filename']}")
        print(f"Successfully processed {len(wbs_files_data)} WBS files.")

except Exception as e:
    print(f"An error occurred while listing or processing files: {e}")

# Store the result in a file to be read by the next step
with open('wbs_data.json', 'w') as f:
    json.dump(wbs_files_data, f)
print("WBS data saved to wbs_data.json")
