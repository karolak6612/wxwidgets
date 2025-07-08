# AI Coding Assistant Guide: Fixing C++17/Qt6 Compilation Errors

## Introduction
This document is your instruction manual for fixing compilation and linking errors in a C++17 and Qt6-based 2D tile-based map editor for the game Tibia. Your primary goal is to systematically analyze and resolve errors reported by the Visual Studio compiler, which have been captured in JSON files.

## Project Structure
-   **`src/`**: Contains the C++ source code of the map editor.
-   **`errors/`**: Contains JSON files with compiler and linker errors. This directory is further divided into:
    -   **`compiler_errors/`**: Errors related to the C++ Standard Library and the Qt6 framework headers. The subdirectory names (e.g., `qapplication`, `xlocale`) correspond to the header file where the error occurred.
    -   **`src_errors/`**: Errors related to the application's own source code. The subdirectory names (e.g., `AboutDialog`, `BrushManagerService`) correspond to the class or component where the error occurred.

## 1. Initial Setup and Error Analysis

### 1.1. Locating and Parsing Error Files
-   Your primary source of information is the set of JSON files located in the `errors/` directory and its subdirectories.
-   Each JSON file contains an array of error objects. **Note**: JSON files may contain duplicate errors (same error repeated multiple times), which is normal compiler behavior when a single issue affects multiple compilation units.
-   The structure of each error object is as follows:
    ```json
    {
      "Compile error": "Cannot open include file: '../../assets/AssetManager.h': No such file or directory",
      "File": "Project_QT\\src\\core\\map\\BaseMap.h",
      "Code line error": 7
    }
    ```
    -   `"Compile error"`: A string containing the error message from the compiler.
    -   `"File"`: A string with the path to the file where the error occurred. This path may be relative to the project root (e.g., `Project_QT\\src\\ui\\dialogs\\AboutDialog.h`) or an absolute path to a system/Qt header.
    -   `"Code line error"`: An integer representing the line number of the error.

### 1.2. Error Deduplication
-   **Process duplicates intelligently**: When parsing JSON files, deduplicate identical errors (same file, line, and message) to focus on unique issues.
-   **Recognize error cascades**: Multiple different errors in the same file often stem from a single root cause (e.g., missing header causing multiple "undeclared identifier" errors).

### 1.3. Error Triage and Prioritization
1.  **Start with `src_errors/`**: Begin by analyzing the errors in `errors/src_errors/`. These are errors in the application's own code and are the most critical to fix.
2.  **Prioritize by impact**: Focus on errors that are likely to resolve multiple downstream issues:
    -   Missing include files (high impact - often resolves many cascading errors)
    -   Namespace declaration issues (medium-high impact)
    -   Syntax errors in headers (medium impact)
    -   Individual function/variable errors (low impact)
3.  **Address `compiler_errors/` Later**: Errors in `errors/compiler_errors/` are often side effects of issues in the application code (e.g., incorrect usage of a Qt class). Fixing the `src_errors` will often resolve these.
4.  **Prioritize by File**: Group the errors by the file in which they occur. Focus on fixing all errors in one file before moving to the next.
5.  **Sequential In-File Fixing**: Within a single file, address errors sequentially from the top of the file to the bottom, as earlier errors can cause subsequent phantom errors.

## 2. Error Analysis and Diagnosis Protocol

### 2.1. Common Error Patterns and Solutions

#### 2.1.1. Missing Include Files
**Pattern**: `Cannot open include file: 'path/to/file.h': No such file or directory`

**Analysis Process**:
1. Check if the file exists at the specified relative path
2. Verify the include path is correct for the project structure
3. Look for the file in alternative locations within the project

**Common Solutions**:
- Correct the relative path in the include statement
- Use forward declarations if full definition isn't needed
- Add missing directories to CMakeLists.txt if needed

**Example**:
```cpp
// Error: #include "../../assets/AssetManager.h"
// Fix: #include "../../../assets/AssetManager.h"  // or correct path
```

#### 2.1.2. Namespace Resolution Errors
**Pattern**: `"SomeClass" is not a member of "SomeNamespace"`

**Analysis Process**:
1. Verify the class/type is declared in the expected namespace
2. Check if the containing header is included
3. Ensure proper namespace qualification

**Common Solutions**:
- Add missing `#include` for the type's header
- Use fully qualified names (e.g., `RME::core::SomeClass`)
- Add proper `namespace` declarations

#### 2.1.3. Qt-Specific Syntax Errors
**Pattern**: `syntax error: missing ':' before identifier 'slots'`

**Analysis Process**:
1. Verify `Q_OBJECT` macro is present in the class
2. Check that `slots` is under proper access specifier
3. Ensure MOC processing is enabled for the file

**Common Solutions**:
```cpp
class MyClass : public QObject {
    Q_OBJECT  // Must be present
    
private slots:  // Correct syntax
    void onButtonClicked();
};
```

#### 2.1.4. Template Instantiation Errors
**Pattern**: Complex template error messages, often in Qt containers

**Analysis Process**:
1. Identify the template type causing the issue
2. Check if required headers for template arguments are included
3. Verify template argument types are complete

### 2.2. Root Cause Analysis
-   **Read the Code**: Before attempting a fix, always read the source code around the line number indicated in the error object.
-   **Identify the Pattern**: Look for common error patterns using the guide above.
-   **Trace Dependencies**: For namespace and include errors, trace the dependency chain to find the root cause.

## 3. C++17 & Qt6 Fix Guidelines

### 3.1. Qt6-Specific Issues
-   **Qt Keywords (`slots`, `signals`)**: These must be placed under a `public`, `protected`, or `private` access specifier. For example: `private slots:`.
-   **`Q_OBJECT` Macro**: Any class that declares signals or slots must have the `Q_OBJECT` macro at the beginning of the class definition.
-   **Header Includes**: Qt6 is more modular. An error for an unknown Qt class (e.g., `QListWidget`) means you need to add a specific include (e.g., `#include <QListWidget>`). Don't rely on monolithic includes like `<QtWidgets>`.
-   **`connect` Syntax**: Use the modern, pointer-based `connect` syntax: `connect(sender, &Sender::valueChanged, receiver, &Receiver::updateValue);`.

### 3.2. General C++ Issues
-   **Missing Includes**: "Undeclared identifier" or "incomplete type" errors usually mean a header is missing. Add the appropriate `#include` statement.
-   **Forward Declarations**: If a full class definition isn't needed, use a forward declaration (e.g., `class MyClass;`) to resolve "incomplete type" errors and reduce compilation dependencies.
-   **C++17 Compliance**: Ensure the code uses C++17 standard features correctly. Pay attention to `std::optional`, structured bindings, and `std::filesystem`.

### 3.3. Systematic Namespace Issue Resolution
-   **CRITICAL**: The codebase has severe and widespread namespace issues. You will frequently encounter errors like `"SomeClass" is not a member of "SomeNamespace"`.
-   **Systematic Approach**:
    1. **Document the namespace structure**: As you encounter namespace errors, build a mental map of the intended namespace hierarchy (e.g., `RME::ui::dialogs`, `RME::core`, etc.)
    2. **Consistent qualification**: When adding namespace qualifications, use the same pattern throughout the codebase
    3. **Include strategy**: Create a systematic approach for including headers - prefer specific includes over broad namespace imports
-   **Resolution Strategy**: 
    1. Find the correct header for the type and include it
    2. Add proper namespace prefix (e.g., `RME::core::SomeClass`)
    3. Use fully qualified names where it is unambiguous and safe
    4. **Avoid** adding `using namespace` directives as they can pollute the namespace and cause further issues

## 4. Code Modification Rules

-   **Minimal, Targeted Fixes**: Apply the smallest change necessary to resolve the error. Do not refactor code or change logic.
-   **Maintain Coding Style**: Adhere strictly to the existing coding conventions (naming, indentation, comments).
-   **Verify Includes**: When adding an `#include`, check if it's already present or if a forward declaration would be better.
-   **Impact Assessment**: Before applying a fix, consider if it might resolve multiple related errors (e.g., adding a missing header file).
-   **CMakeLists.txt**: Do not modify the `CMakeLists.txt` file unless you identify a clear problem, such as a source file not being listed or a required Qt module being missing from `find_package` or `target_link_libraries`.

## 5. Workflow

### 5.1. No-Compilation Constraint
-   **CRITICAL**: You are **not permitted** to compile the code. You cannot run a compiler to verify your fixes.
-   **Implication**: You must rely entirely on the provided JSON error logs and your static analysis of the source code. This makes precision essential. Your fixes must be syntactically correct and logically sound based on your analysis alone.

### 5.2. Systematic Process
1.  **Full Error Scan**: Parse all `.json` files in the `errors` directory and deduplicate identical errors.
2.  **Pattern Recognition**: Group errors by type using the common patterns guide above.
3.  **Create a Work Plan**: Generate a prioritized list of fixes, starting with high-impact issues (missing includes, namespace problems).
4.  **Fix by Priority**:
    a.  Address high-impact errors first (missing includes, major syntax errors)
    b.  Apply fixes that resolve multiple downstream errors
    c.  Handle remaining individual errors
5.  **Handle Cascading Effects**: After each significant fix category, reassess which remaining errors might have been resolved.
6.  **Documentation**: Keep track of fixes applied and their expected impact on downstream errors.

### 5.3. Escalation Protocol
**Ask for Clarification** if you encounter:
- Ambiguous errors that could have multiple valid solutions
- Errors that would require significant design changes
- Missing files that seem to be core components
- Inconsistent namespace structures that suggest architectural issues

Present the problem clearly with your analysis and proposed solution(s) for user guidance.

## 6. Expected Outcomes

Following this systematic approach should result in:
- **Reduced error count**: High-impact fixes should resolve multiple cascading errors
- **Consistent codebase**: Systematic namespace resolution and include management
- **Maintainable solutions**: Minimal changes that preserve existing architecture
- **Clear documentation**: Record of what was fixed and why

The key to success is methodical analysis, pattern recognition, and strategic prioritization of fixes based on their downstream impact.