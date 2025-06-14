# generate_graph.py
import os
import re
import argparse
from collections import defaultdict

# Regular expression to find #include "local_header.h"
# This ignores system headers like #include <vector>
INCLUDE_REGEX = re.compile(r'#include\s+"([^"]+)"')

def find_dependencies(src_path):
    """
    Scans the source path and builds a dictionary of dependencies.
    Returns a dict: { "file_path": set("included_file_1", "included_file_2") }
    """
    dependencies = defaultdict(set)
    print(f"Scanning for .cpp and .h files in: {src_path}")

    for root, _, files in os.walk(src_path):
        for file in files:
            if file.endswith(('.cpp', '.h', '.hpp')):
                file_path = os.path.join(root, file)
                
                # Use a relative path for cleaner node names in the graph
                relative_path = os.path.relpath(file_path, src_path).replace('\\', '/')
                
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        for line in f:
                            match = INCLUDE_REGEX.search(line)
                            if match:
                                # Normalize the included path (e.g., ..\foo\bar.h -> foo/bar.h)
                                included_file = match.group(1).replace('\\', '/')
                                dependencies[relative_path].add(included_file)
                except Exception as e:
                    print(f"Warning: Could not read file {file_path}: {e}")

    print(f"Scan complete. Found dependencies in {len(dependencies)} files.")
    return dependencies

def generate_dot_file(dependencies, output_file):
    """
    Generates a DOT graph description file from the dependencies.
    """
    print(f"Generating DOT file: {output_file}")
    with open(output_file, 'w') as f:
        f.write('digraph "Source Code Dependencies" {\n')
        f.write('  rankdir="LR";\n')  # Layout from Left to Right
        f.write('  node [shape=box, style="rounded,filled", fillcolor="#EFEFEF"];\n')
        f.write('  edge [color="#444444"];\n\n')

        for file, includes in dependencies.items():
            if not includes:
                continue
            for include in includes:
                f.write(f'  "{file}" -> "{include}";\n')

        f.write('}\n')
    print("DOT file generation complete.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate a C++ include dependency graph.")
    parser.add_argument("src_dir", help="The root directory of the source code.")
    parser.add_argument("-o", "--output", default="dependencies.dot", help="The name of the output DOT file.")
    args = parser.parse_args()
    
    if not os.path.isdir(args.src_dir):
        print(f"Error: Directory not found at '{args.src_dir}'")
        exit(1)
        
    deps = find_dependencies(args.src_dir)
    generate_dot_file(deps, args.output)