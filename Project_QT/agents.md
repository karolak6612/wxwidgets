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
-   Each JSON file contains an array of error objects. You must parse all `.json` files to build a complete list of errors.
-   The structure of each error object is as follows:
    -   `"Compile error"`: A string containing the error message from the compiler.
    -   `"File"`: A string with the path to the file where the error occurred. This path may be relative to the project root (e.g., `Project_QT\\src\\ui\\dialogs\\AboutDialog.h`) or an absolute path to a system/Qt header.
    -   `"Code line error"`: An integer representing the line number of the error.

### 1.2. Error Triage and Prioritization
1.  **Start with `src_errors/`**: Begin by analyzing the errors in `errors/src_errors/`. These are errors in the application's own code and are the most critical to fix.
2.  **Address `compiler_errors/` Later**: Errors in `errors/compiler_errors/` are often side effects of issues in the application code (e.g., incorrect usage of a Qt class). Fixing the `src_errors` will often resolve these.
3.  **Prioritize by File**: Group the errors by the file in which they occur. Focus on fixing all errors in one file before moving to the next.
4.  **Sequential In-File Fixing**: Within a single file, address errors sequentially from the top of the file to the bottom, as earlier errors can cause subsequent phantom errors.

## 2. Error Analysis and Diagnosis Protocol

### 2.1. Root Cause Analysis
-   **Read the Code**: Before attempting a fix, always read the source code around the line number indicated in the error object.
-   **Identify the Pattern**: Look for common error patterns. For example, an "undeclared identifier" error often points to a missing `#include` statement. A "syntax error" near a `Q_OBJECT` macro might indicate a missing `Q_OBJECT` or an incorrect access specifier for `slots`.
-   **Example: `AboutDialog.h` Errors**
    -   **Errors**: `syntax error: missing ':' before identifier 'slots'` and `unexpected token(s) preceding ':'; skipping apparent function body` at line 32.
    -   **Analysis**: Looking at `src/ui/dialogs/AboutDialog.h`, line 32 is `private slots:`. The error indicates a syntax problem. In Qt, `slots` is a keyword that must be under an access specifier. The code is `private slots:`, which is correct. However, if it were just `slots:`, that would be an error. The error message suggests the compiler is not recognizing `slots` as a Qt keyword. This often happens if the `Q_OBJECT` macro is missing or if the file is not being processed by the Meta-Object Compiler (MOC). In this specific case, the code looks correct, which might indicate a build system (CMake) issue where the file is not being correctly processed by `AUTOMOC`. However, the most direct fix is to ensure the syntax is `private slots:`.

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

### 3.3. Widespread Namespace Issues
-   **CRITICAL**: The codebase has severe and widespread namespace issues. You will frequently encounter errors like `"SomeClass" is not a member of "SomeNamespace"`.
-   **Root Cause**: While the overall namespace structure (e.g., `RME::ui::dialogs`) is intentional, the implementation is flawed. This is often due to missing includes or incorrect/missing namespace qualifications for types.
-   **Your Task**: When you encounter a namespace-related error, you must resolve it by finding the correct header for the type and including it, or by adding the proper namespace prefix (e.g., `RME::core::SomeClass`). Use fully qualified names where it is unambiguous and safe. Avoid adding `using namespace` directives as they can pollute the namespace and cause further issues.

## 4. Code Modification Rules

-   **Minimal, Targeted Fixes**: Apply the smallest change necessary to resolve the error. Do not refactor code or change logic.
-   **Maintain Coding Style**: Adhere strictly to the existing coding conventions (naming, indentation, comments).
-   **Verify Includes**: When adding an `#include`, check if it's already present or if a forward declaration would be better.
-   **CMakeLists.txt**: Do not modify the `CMakeLists.txt` file unless you identify a clear problem, such as a source file not being listed or a required Qt module being missing from `find_package` or `target_link_libraries`.

## 5. Workflow

### 5.1. No-Compilation Constraint
-   **CRITICAL**: You are **not permitted** to compile the code. You cannot run a compiler to verify your fixes.
-   **Implication**: You must rely entirely on the provided JSON error logs and your static analysis of the source code. This makes precision essential. Your fixes must be syntactically correct and logically sound based on your analysis alone. The "Minimal, Targeted Fixes" rule is therefore extremely important.

### 5.2. Systematic Process
1.  **Full Error Scan**: First, parse all `.json` files in the `errors` directory.
2.  **Create a Work Plan**: Generate a prioritized list of files to fix, starting with those in `src_errors`.
3.  **Fix File by File**:
    a.  Open a source file (e.g., `src/ui/dialogs/AboutDialog.h`).
    b.  Read the corresponding error file (`errors/src_errors/AboutDialog/AboutDialog.json`).
    c.  Apply fixes for the errors in that file.
    d.  Move to the next file in your work plan.
4.  **Handle Cascading Errors**: Be aware that fixing one error (like a missing header) may resolve many subsequent errors. After each significant fix, it would be ideal to re-evaluate the remaining errors, but in this workflow, you will proceed with your initial plan and assume fixes will resolve downstream issues.
5.  **Ask for Clarification**: If an error is ambiguous or requires a significant design change, stop and present the problem and your proposed solution to the user.