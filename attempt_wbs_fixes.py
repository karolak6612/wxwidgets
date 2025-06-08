import os
import json
import re

error_summary_path = "wbs_error_summary.json"
wbs_dir = "enhanced_wbs_yaml_files/"
fix_log = []

def attempt_fixes(content, filename):
    original_content_for_comparison = content # Keep a pristine copy for comparison after all changes
    modified_content_in_step = content
    applied_fixes = []

    # Fix 1: Replace unescaped backticks with single quotes
    # Avoids replacing within existing quoted strings or code blocks that might use backticks intentionally.
    # This simplified regex looks for backticks that are likely delimiters.
    # It specifically targets backticks surrounded by whitespace or at the start/end of a line,
    # or adjacent to non-word characters, to avoid breaking valid YAML/code constructs.
    # A positive lookbehind and lookahead can help make this more specific.
    # (?<=\s|^) ensures start of line or whitespace before. (?=\s|$) ensures end of line or whitespace after.
    # This is still heuristic. A common error pattern is `key`: `value`
    # Let's try to replace backticks that appear to be used as quotes for simple values or keys
    # Example: `some_key`: `some_value` -> 'some_key': 'some_value'
    # Simpler: just replace all backticks not inside existing single or double quotes.

    # Iteratively replace to handle cases like ```key``` -> '''key'''
    # This is still very broad. Let's refine.
    # Targeting `key` or `value` type structures, often seen with path-like strings or informal text.
    # A very common error from the logs is `some text` as a value.
    if '`' in modified_content_in_step:
        # This regex attempts to find backticks that are likely used as quotes
        # It avoids replacing if the backtick is adjacent to another backtick (like in ```code blocks```)
        # or if it's part of a word (e.g. variable_name_`suffix`)
        # This is difficult to get perfect without a full parser.
        # For now, let's assume backticks are problematic if they are delimiters for a "word".
        # Example: description: `This is an error`

        # Safer approach: only replace backticks if the error message specifically indicates it.
        # Given the generic nature of current error messages, this is hard.
        # Let's use a simpler regex that was in the original prompt, but be aware of its limitations.
        temp_content = re.sub(r'(?<![\'"`])`(?![`\'"])', "'", modified_content_in_step) # Avoid `` and ````
        if temp_content != modified_content_in_step:
            modified_content_in_step = temp_content
            applied_fixes.append("Replaced_Backticks_Simple")
            # print(f"Applied 'Replaced_Backticks_Simple' to {filename}")


    # Fix 2: Handle multiple YAML documents (---)
    if "\n---" in modified_content_in_step: # Ensure it's a document separator, not part of a string
        parts = modified_content_in_step.split("\n---")
        if len(parts) > 1:
            modified_content_in_step = parts[0]
            if modified_content_in_step.strip(): # if content is not just whitespace
                 modified_content_in_step = modified_content_in_step.rstrip() + "\n"
            else: # if content becomes empty, ensure it's just an empty string or a single newline
                 modified_content_in_step = "\n"
            applied_fixes.append("Removed_Multiple_YAML_Documents")
            # print(f"Applied 'Removed_Multiple_YAML_Documents' to {filename}")

    # Fix 3: Attempt to fix missing colons for simple key-value pairs
    # This regex looks for lines like "key value" (no colon, not a list item)
    # and changes them to "key: value". It's very heuristic.
    # It tries to ensure the "key" looks like a typical YAML key (alphanumeric, underscores, hyphens).
    lines = modified_content_in_step.split('\n')
    new_lines = []
    made_colon_fix_this_iteration = False
    for i, line in enumerate(lines):
        stripped_line = line.strip()
        # Avoid empty lines, comments, list items, existing colons, document separators, flow style
        if not stripped_line or stripped_line.startswith('#') or stripped_line.startswith('-') or \
           ':' in line or stripped_line == "---" or stripped_line.startswith('{') or stripped_line.startswith('['):
            new_lines.append(line)
            continue

        # Heuristic: "word possibly_another_word" -> "word: possibly_another_word"
        # The key should not contain spaces.
        match = re.match(r'^(\s*)([a-zA-Z0-9_-]+)(\s+)([^#\s].*?)(\s*#.*)?$', line)
        if match:
            indent, key, space_after_key, value, comment = match.groups()
            # Ensure that this line isn't part of a multi-line string from the previous line
            # This is a very basic check; proper multiline string detection is complex
            if i > 0 and (new_lines[-1].strip().endswith('|') or new_lines[-1].strip().endswith('>')):
                 new_lines.append(line) # Likely part of multiline string, don't change
                 continue

            new_lines.append(f"{indent}{key}:{space_after_key}{value}{(comment if comment else '')}")
            made_colon_fix_this_iteration = True
        else:
            new_lines.append(line)

    if made_colon_fix_this_iteration:
        modified_content_in_step = "\n".join(new_lines)
        applied_fixes.append("Attempted_Missing_Colon_Fix")
        # print(f"Applied 'Attempted_Missing_Colon_Fix' to {filename}")

    # If content changed from original, return new content and list of fixes
    if modified_content_in_step != original_content_for_comparison:
        return modified_content_in_step, applied_fixes
    else:
        return original_content_for_comparison, [] # No changes made

if not os.path.exists(error_summary_path):
    print(f"Error: Summary file {error_summary_path} not found. Run analysis script first.")
else:
    try:
        with open(error_summary_path, 'r') as f:
            summary_data = json.load(f)

        error_file_entries = summary_data.get("error_files", [])
        if not error_file_entries:
            print("No error files listed in the summary to fix.")
        else:
            print(f"Found {len(error_file_entries)} error entries. Attempting to fix files...")
            files_to_fix_filenames = {entry['filename'] for entry in error_file_entries if entry.get('filename')} # Unique filenames

            for filename in files_to_fix_filenames:
                filepath = os.path.join(wbs_dir, filename)

                if not os.path.exists(filepath):
                    print(f"File {filename} listed in summary not found at {filepath}. Skipping.")
                    fix_log.append({"filename": filename, "status": "Not_Found_At_Fix_Time", "fixes": []})
                    continue

                try:
                    # Read with 'utf-8-sig' to handle potential BOM, then write as 'utf-8'
                    with open(filepath, 'r', encoding='utf-8-sig') as f:
                        original_file_content = f.read()

                    modified_content, fixes_applied_list = attempt_fixes(original_file_content, filename)

                    if fixes_applied_list:
                        with open(filepath, 'w', encoding='utf-8') as f: # Write back as standard UTF-8
                            f.write(modified_content)
                        fix_log.append({"filename": filename, "status": "Fix_Attempted", "fixes": fixes_applied_list})
                        print(f"Applied fixes for {filename}. Fixes: {', '.join(fixes_applied_list)}")
                    else:
                        fix_log.append({"filename": filename, "status": "No_Fixes_Applicable_Or_Needed", "fixes": []})
                        # print(f"No applicable automatic fixes for {filename} or file already conformant to fixes.")
                except Exception as e:
                    error_message = str(e)
                    print(f"Error processing file {filename}: {error_message}")
                    fix_log.append({"filename": filename, "status": "Error_During_Fix_Attempt", "error": error_message, "fixes": []})

            print(f"Finished attempting fixes for {len(files_to_fix_filenames)} unique files listed in error summary.")

    except json.JSONDecodeError:
        print(f"Error: Could not decode JSON from {error_summary_path}.")
    except Exception as e:
        print(f"An unexpected error occurred during the fix process: {e}")

# Save the log of fixes
fix_log_path = "wbs_fix_attempt_log.json"
try:
    with open(fix_log_path, 'w') as f:
        json.dump(fix_log, f, indent=2)
    print(f"Fix attempt log saved to {fix_log_path}")
except Exception as e:
    print(f"Error saving fix log: {e}")

print("Fix script execution complete.")
