import os
import yaml # Ensure PyYAML is installed: pip install PyYAML
import argparse

def validate_yaml_files(directory_path, check_keys=True):
    """
    Validates YAML files in a given directory.

    Args:
        directory_path (str): The path to the directory containing YAML files.
        check_keys (bool): Whether to check for 'id' and 'dependencies' keys.
    """
    print(f"Scanning directory: {directory_path}\n")
    found_files = False
    error_count = 0
    success_count = 0

    for filename in os.listdir(directory_path):
        if filename.endswith(".yaml") or filename.endswith(".yml"):
            found_files = True
            filepath = os.path.join(directory_path, filename)
            print(f"--- Validating: {filename} ---")
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = yaml.safe_load(f)
                
                if not isinstance(content, dict):
                    print("Status: ERROR - Content is not a dictionary (root should be a mapping).")
                    error_count += 1
                    continue

                print("Status: Parsed successfully.")
                
                if check_keys:
                    missing_keys = []
                    if 'id' not in content:
                        missing_keys.append('id')
                    # 'title' is not strictly needed for START.yaml structure, but good to have.
                    # 'dependencies' is crucial, should be a list, even if empty.
                    if 'dependencies' not in content: 
                        missing_keys.append('dependencies (even if empty list [])')
                    elif not isinstance(content['dependencies'], list):
                        missing_keys.append('dependencies (must be a list, e.g., [] or [dep1, dep2])')

                    if missing_keys:
                        print(f"Warning: Missing or invalid crucial keys: {', '.join(missing_keys)}")
                        # Consider if this should be an error for your workflow
                    else:
                        print("Found required keys: 'id', and 'dependencies' is a list.")
                
                success_count +=1

            except yaml.YAMLError as e:
                print(f"Status: ERROR - YAML parsing error:")
                print(e)
                error_count += 1
            except Exception as e:
                print(f"Status: ERROR - General error reading or processing file:")
                print(e)
                error_count += 1
            print("--- End of validation for this file ---\n")

    if not found_files:
        print(f"No YAML files (.yaml or .yml) found in '{directory_path}'.")
        return

    print("--- Validation Summary ---")
    print(f"Total YAML files checked: {success_count + error_count}")
    print(f"Successfully parsed files: {success_count}")
    print(f"Files with errors: {error_count}")
    if error_count > 0:
        print("\nPlease fix the files listed with ERROR status.")
    else:
        print("\nAll checked YAML files seem to be well-formed!")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Validate YAML files in a directory.")
    parser.add_argument("directory", help="The path to the directory containing YAML files.")
    parser.add_argument(
        "--no-key-check", 
        action="store_false", 
        dest="check_keys",
        help="Disable checking for 'id' and 'dependencies' keys."
    )
    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        print(f"Error: Provided path '{args.directory}' is not a valid directory.")
    else:
        validate_yaml_files(args.directory, check_keys=args.check_keys)