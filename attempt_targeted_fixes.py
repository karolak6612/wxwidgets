import os
import json
import re

error_patterns_path = "wbs_error_patterns.json"
wbs_dir = "enhanced_wbs_yaml_files/"
targeted_fix_log = []

# --- Fix Functions ---
# Note: Most of these are stubs as per the prompt, only fix_missing_colon_targeted has active logic.

def fix_block_sequence_start(content, filename):
    # Placeholder: Actual logic is complex.
    # This function, as a stub, will not modify content.
    # print(f"Stub: Would attempt fix_block_sequence_start for {filename}")
    fixes_applied = []
    # Example of a *potential* change (currently commented out):
    # if "some specific condition":
    #    content = content.replace("old", "new")
    #    fixes_applied.append("Made_A_Hypothetical_Change_Block_Sequence")
    return content, fixes_applied

def fix_mapping_values_not_allowed(content, filename):
    # Placeholder: Actual logic is complex.
    # This function, as a stub, will not modify content.
    # print(f"Stub: Would attempt fix_mapping_values_not_allowed for {filename}")
    fixes_applied = []
    return content, fixes_applied

def fix_block_end_scalar(content, filename):
    # Placeholder: Actual logic is complex.
    # This function, as a stub, will not modify content.
    # print(f"Stub: Would attempt fix_block_end_scalar for {filename}")
    fixes_applied = []
    return content, fixes_applied

def fix_missing_colon_targeted(content, filename):
    lines = content.split('\n')
    new_lines = []
    made_colon_fix = False
    # print(f"Running fix_missing_colon_targeted for {filename}")
    for line_num, line_content in enumerate(lines):
        stripped_line = line_content.strip()
        original_line = line_content # Keep original for appending if no fix

        # Conditions for attempting fix:
        # - Not a comment, list item, or known block scalar indicator
        # - Does not already contain a colon
        # - Contains at least one space (potential key value separation)
        # - Not an empty line
        if stripped_line and not stripped_line.startswith(('#', '-', '|', '>')) and ':' not in line_content and ' ' in stripped_line:
            # Heuristic: try to match "key value" where key is simple
            # Key: alphanumeric, underscore, hyphen. Value: anything not starting with YAML special chars.
            # Ensure it's not part of a multi-line value from a previous line ending with | or >
            if line_num > 0 and (lines[line_num-1].strip().endswith('|') or lines[line_num-1].strip().endswith('>')):
                new_lines.append(original_line)
                continue

            # Regex: `^(\s*)([a-zA-Z0-9_.-]+)\s+([^#\s].*?)(?:\s*#.*)?$`
            #   (\s*)                     -> indent
            #   ([a-zA-Z0-9_.-]+)         -> key (alphanumeric, _, ., -)
            #   \s+                       -> separator space(s)
            #   ([^#\s].*?)               -> value (starts with non-#, non-space, non-greedy)
            #   (?:\s*#.*)?$              -> optional comment at end of line
            match = re.match(r"^(\s*)([a-zA-Z0-9_.-]+)\s+([^#\s].*?)(?:\s*#.*)?$", line_content)

            if match:
                indent, key, value_part = match.groups()[:3] # We only need these three for reconstruction
                comment_part = ""
                if '#' in line_content:
                    comment_match = re.search(r"(\s*#.*)$", line_content)
                    if comment_match:
                        # Check if the value itself contains a # not indicating a comment
                        # This is tricky. For now, assume # after value is a comment.
                        # If value_part itself ends with text before #, this might be okay.
                        # Let's assume our regex for value_part `([^#\s].*?)` correctly captures value before comment.
                        temp_value_plus_potential_comment = line_content[match.start(3):]
                        value_part_only, sep, comment_text = temp_value_plus_potential_comment.partition('#')
                        if sep: # if '#' was found
                            value_part = value_part_only.rstrip() # remove spaces before comment
                            comment_part = sep + comment_text
                        else: # no comment, value_part is as captured initially
                            value_part = value_part.rstrip()


                # Avoid fixing if value itself looks like a structured element or is empty
                if value_part.strip() and not value_part.strip().startswith(('-', '[', '{', '&', '*', '!', '%', '@', '`')):
                    fixed_line = f"{indent}{key}: {value_part}{comment_part}"
                    new_lines.append(fixed_line)
                    # print(f"  FIXED Line {line_num+1} in {filename}: '{line_content}' -> '{fixed_line}'")
                    made_colon_fix = True
                    continue
        new_lines.append(original_line)

    if made_colon_fix:
        return "\n".join(new_lines), ["Attempted_Targeted_Missing_Colon_Fix"]
    return content, []

# --- Main Script Logic ---

PATTERNS_TO_FIX = {
    "expected <block end>, but found '<block sequence start>'": fix_block_sequence_start,
    "mapping values are not allowed here": fix_mapping_values_not_allowed,
    "expected <block end>, but found '<scalar>'": fix_block_end_scalar,
    "could not find expected ':'": fix_missing_colon_targeted
    # "while scanning an alias (often '*' character)" is not attempted due to its complexity and risk of breaking valid * usage.
}

if not os.path.exists(error_patterns_path):
    print(f"Error: Pattern analysis file {error_patterns_path} not found. Run pattern analysis first.")
else:
    try:
        with open(error_patterns_path, 'r') as f:
            analysis_data = json.load(f)

        detailed_errors_by_pattern = analysis_data.get("detailed_errors_by_pattern", {})
        if not detailed_errors_by_pattern:
            print("No detailed errors by pattern found in the analysis file. Nothing to fix.")
        else:
            # Create a unique list of files to process, applying all relevant fixes per file
            files_and_their_patterns_to_fix = {}
            for pattern_key, error_list in detailed_errors_by_pattern.items():
                if pattern_key in PATTERNS_TO_FIX: # Only consider patterns we have a fix for
                    for error_item in error_list:
                        filename = error_item.get("filename")
                        if filename:
                            if filename not in files_and_their_patterns_to_fix:
                                files_and_their_patterns_to_fix[filename] = []
                            # Add pattern if it's one we try to fix and not already listed for this file
                            if pattern_key not in files_and_their_patterns_to_fix[filename]:
                                files_and_their_patterns_to_fix[filename].append(pattern_key)

            if not files_and_their_patterns_to_fix:
                print("No files found matching the patterns targeted for fixing.")
            else:
                print(f"Attempting targeted fixes for {len(files_and_their_patterns_to_fix)} unique files...")

                for filename, patterns_for_this_file in files_and_their_patterns_to_fix.items():
                    filepath = os.path.join(wbs_dir, filename)
                    if not os.path.exists(filepath):
                        print(f"File {filename} (listed for patterns: {patterns_for_this_file}) not found at {filepath}. Skipping.")
                        targeted_fix_log.append({"filename": filename, "status": "Not_Found_At_Targeted_Fix_Time", "fixes_attempted_by_pattern": {p:PATTERNS_TO_FIX[p].__name__ for p in patterns_for_this_file}})
                        continue

                    try:
                        with open(filepath, 'r', encoding='utf-8-sig') as f:
                            current_file_content = f.read()

                        original_content_for_comparison = current_file_content # For final check if anything changed
                        accumulated_fixes_for_file = []

                        # Sequentially apply all relevant fix functions for this file
                        for pattern_name in patterns_for_this_file:
                            fix_function_to_apply = PATTERNS_TO_FIX[pattern_name]
                            # Pass the latest version of content to each fix function
                            current_file_content, fixes_by_current_func = fix_function_to_apply(current_file_content, filename)
                            if fixes_by_current_func:
                                accumulated_fixes_for_file.extend(fixes_by_current_func)

                        # Remove duplicates from accumulated_fixes_for_file while preserving order (if important, though set is fine here)
                        unique_fixes_applied = sorted(list(set(accumulated_fixes_for_file)))

                        if current_file_content != original_content_for_comparison: # Check if content actually changed
                            with open(filepath, 'w', encoding='utf-8') as f:
                                f.write(current_file_content)
                            log_entry = {"filename": filename, "status": "Targeted_Fix_Applied", "fixes": unique_fixes_applied}
                            targeted_fix_log.append(log_entry)
                            print(f"Applied targeted fixes for {filename}. Fixes: {', '.join(unique_fixes_applied)}")
                        else:
                            targeted_fix_log.append({"filename": filename, "status": "No_Effective_Targeted_Fixes_Applied", "attempted_fix_functions": [PATTERNS_TO_FIX[p].__name__ for p in patterns_for_this_file]})
                    except Exception as e:
                        error_msg_for_log = str(e)
                        print(f"Error during targeted fix attempt for file {filename}: {error_msg_for_log}")
                        targeted_fix_log.append({"filename": filename, "status": "Error_During_Targeted_Fix_Process", "error": error_msg_for_log})

            print(f"Finished attempting targeted fixes for {len(files_and_their_patterns_to_fix)} files.")

    except json.JSONDecodeError:
        print(f"Error: Could not decode JSON from {error_patterns_path}.")
    except Exception as e:
        print(f"An unexpected critical error occurred: {e}")

# Save the log of targeted fixes
targeted_fix_log_path = "wbs_targeted_fix_log.json"
try:
    with open(targeted_fix_log_path, 'w') as f:
        json.dump(targeted_fix_log, f, indent=2)
    print(f"Targeted fix attempt log saved to {targeted_fix_log_path}")
except Exception as e:
    print(f"Error saving targeted fix log: {e}")

print("Targeted fix script execution complete.")
