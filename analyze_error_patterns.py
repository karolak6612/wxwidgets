import os
import json
from collections import Counter

error_summary_path = "wbs_error_summary.json"
error_patterns_path = "wbs_error_patterns.json"
error_patterns = Counter()
detailed_errors = {} # Using a dict to store lists of errors for each pattern

def extract_pattern(error_message):
    # Order of checks can be important if messages are complex or overlap
    if "mapping values are not allowed here" in error_message:
        return "mapping values are not allowed here"
    if "while scanning an alias" in error_message: # Often related to '*' not being a valid alias start
        return "while scanning an alias (often '*' character)"
    if "expected <block end>, but found '<block sequence start>'" in error_message:
        return "expected <block end>, but found '<block sequence start>'"
    if "expected <block end>, but found '<scalar>'" in error_message: # Often related to indentation or unexpected text
        return "expected <block end>, but found '<scalar>'"
    if "could not find expected ':'" in error_message:
        return "could not find expected ':'"
    # "while parsing a block mapping" can be too general if other specific cases below it also match
    # Test if this is too greedy or if it correctly categorizes.
    # if "while parsing a block mapping" in error_message:
    #     return "while parsing a block mapping (general)"

    # More specific parsing issues often give context in the first line
    # Fallback for less common or more specific messages
    parts = error_message.split('\n')
    first_line = parts[0].strip()

    # Further refinement based on observed error messages from previous steps
    if "while parsing a block collection" in first_line: # Catches various collection parsing issues
        return "general block collection parsing issue"
    if "while parsing a block mapping" in first_line: # This might catch some already handled, but also others
        return "general block mapping parsing issue"
    if "while scanning a simple key" in first_line: # e.g. if ':' is missing after a key
        return "issue scanning simple key (often missing colon)"

    return first_line # Return the first line of the error as a general pattern if no specific match


if not os.path.exists(error_summary_path):
    print(f"Error: Source file {error_summary_path} not found. Run analysis after fixes first.")
else:
    try:
        with open(error_summary_path, 'r') as f:
            summary = json.load(f)

        error_file_entries = summary.get("error_files", [])
        if not error_file_entries:
            print("No error files listed in the summary to analyze for patterns.")
        else:
            print(f"Analyzing {len(error_file_entries)} error entries from {error_summary_path}...")
            for error_entry in error_file_entries:
                error_msg = error_entry.get("error", "Unknown error (no message)")
                filename = error_entry.get("filename", "N/A")

                pattern = extract_pattern(error_msg)
                error_patterns[pattern] += 1

                if pattern not in detailed_errors:
                    detailed_errors[pattern] = []
                # Store filename and the full original error message for that pattern
                detailed_errors[pattern].append({
                    "filename": filename,
                    "full_error": error_msg
                })

            print("\n--- Common Error Patterns ---")
            if error_patterns:
                for pattern, count in error_patterns.most_common():
                    print(f"\nPattern: \"{pattern}\"")
                    print(f"  Count: {count}")
                    # Show first 2 example filenames for brevity in console
                    example_filenames = [e['filename'] for e in detailed_errors[pattern][:2]]
                    print(f"  Example Filename(s): {example_filenames}")
            else:
                print("No error patterns identified (this is unexpected if there were errors).")

            # Save the analysis
            output_data = {
                "error_pattern_counts": dict(error_patterns.most_common()),
                "detailed_errors_by_pattern": detailed_errors
            }
            with open(error_patterns_path, 'w') as f:
                json.dump(output_data, f, indent=2)
            print(f"\nError pattern analysis saved to {error_patterns_path}")

    except json.JSONDecodeError:
        print(f"Error: Could not decode JSON from {error_summary_path}.")
    except Exception as e:
        print(f"An unexpected error occurred during pattern analysis: {e}")

print("Error pattern analysis script complete.")
