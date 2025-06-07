import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_data = {
    "wbs_item_id": "TEST-10",
    "name": "UI Test Cross-Platform Compatibility",
    "description": "Perform (primarily manual) UI tests on Windows, macOS, and Linux to identify and address platform-specific UI glitches, layout issues, and behavioral inconsistencies. Automated UI tests from TEST-09 should also be run on each platform.",
    "dependencies": [
        "TEST-09", # Automated UI test suite to be run on different platforms.
        # Implicitly depends on all UI-XX tasks and the overall application being in a testable state.
    ],
    "input_files": [], # This is a testing execution and reporting task.
    "analyzed_input_files": [], # N/A
    "documentation_references": [
        "Qt Platform Notes: https://doc.qt.io/qt-6/platformspecific.html",
        "Qt High DPI Support: https://doc.qt.io/qt-6/highdpi.html",
        "Testing for Cross-Platform Issues (General Software Engineering Principles)"
    ],
    "current_functionality_summary": """\
This WBS item outlines the process for testing the Qt6 Remere's Map Editor application across its target operating systems: Windows, macOS, and Linux. The primary goal is to ensure a consistent and correct user experience on each platform. This involves manual exploratory testing based on a checklist and, where possible, running the automated UI test suite developed in TEST-09.\
""",
    "definition_of_done": [
        "A comprehensive UI testing checklist, covering key features, dialogs, and user workflows, has been systematically executed on recent versions of Windows, macOS, and at least one common Linux distribution/desktop environment.",
        "The automated UI test suite (from `TEST-09`) has been executed on each target platform, and any platform-specific functional regressions identified are documented.",
        "All significant platform-specific UI issues (e.g., layout breakages, incorrect widget scaling, font rendering problems affecting readability, non-functional controls, critical deviations from native look-and-feel where expected) are documented with detailed reproduction steps, screenshots, and severity assessment.",
        "A report summarizing the findings for each platform is produced.",
        "Actionable bug reports for critical or major platform-specific issues are created for developers.",
        "The application achieves a state of acceptable visual and functional consistency across the tested platforms, with any remaining minor discrepancies acknowledged and documented if not fixed."
    ],
    "boilerplate_coder_ai_prompt": """\
This WBS item, `TEST-10`, is focused on the **execution and documentation of cross-platform UI compatibility testing** for the Qt6 Remere's Map Editor. This is largely a manual testing effort, potentially supplemented by running automated UI tests from `TEST-09` on different OS environments.

**Primary Goal:**
Identify, document, and report any UI/UX issues that are specific to Windows, macOS, or Linux platforms.

**Testing Scope & Environments:**
-   **Platforms:**
    -   Windows (e.g., Windows 10/11).
    -   macOS (e.g., latest available version).
    -   Linux (e.g., Ubuntu LTS with GNOME; consider testing on another DE like KDE if feasible).
-   **Key Areas for Manual Testing Checklist (for each platform):**
    1.  **Installation & Startup:** Application installs and launches correctly.
    2.  **Main Window:**
        -   Correct layout, no overlapping/truncated elements.
        -   Window decorations, title bar, minimize/maximize/close behavior are native.
        -   Menu bar: Native integration on macOS (global menu bar), correct appearance on Windows/Linux.
        -   Toolbars & Palettes: Correctly docked/undocked, icons visible, tooltips appear.
    3.  **Dialogs:**
        -   Standard dialogs (`QFileDialog`, `QMessageBox`, `QInputDialog`, `QFontDialog`, `QColorDialog`) have native look and feel.
        -   Custom dialogs render correctly, all controls are accessible and functional.
    4.  **Map Canvas Rendering:** While `TEST-07` covers detailed rendering logic, perform a smoke test to ensure the map canvas displays without obvious platform-specific rendering artifacts (e.g., severe flickering, missing chunks, completely incorrect colors not caught by automated tests).
    5.  **Controls & Widgets:**
        -   Buttons, checkboxes, radio buttons, sliders, input fields, tabs, scrollbars, etc., are visually correct and function as expected.
        -   Font rendering is clear and legible across all UI text.
    6.  **User Interactions:**
        -   Mouse clicks, drags, wheel scrolling on map and UI elements.
        -   Keyboard input and shortcuts (check for conflicts with OS shortcuts, standard platform equivalents like Cmd+C vs. Ctrl+C).
        -   Drag-and-drop operations (e.g., for docking palettes).
    7.  **High DPI Scaling:** If testing on High DPI displays, ensure UI elements scale correctly and are not blurry or misaligned.
    8.  **Performance/Responsiveness:** Note any significant platform-specific sluggishness in UI interactions (though detailed performance profiling is `REFACTOR-02`).
    9.  **Feature Workflows:** Execute key workflows from `TEST-09` manually (e.g., File > New, basic editing, File > Save) to catch platform-specific behavioral deviations.

**Automated UI Test Execution (from `TEST-09`):**
-   Set up environments to run the automated UI test suite (developed in `TEST-09`) on Windows, macOS, and Linux.
-   Execute the suite and record pass/fail results for each platform.
-   Analyze any failures to determine if they are due to platform-specific issues.

**Deliverables for this task (manual generation by tester/QA):**
1.  A **Cross-Platform Test Report** detailing:
    -   Environments tested (OS versions, Desktop Environments).
    -   Summary of manual checklist execution.
    -   Results of automated UI test suite execution on each platform.
    -   A list of all identified platform-specific bugs/issues, including:
        -   Description, severity, steps to reproduce, platform(s) affected.
        -   Screenshots/videos illustrating the issues.
2.  (If applicable) Bug reports filed in an issue tracking system for the identified problems.

This task does not typically involve writing new application code, but rather the systematic testing of the application developed from other WBS items across different operating systems. If the AI is later tasked to *fix* issues found, this YAML's context will be relevant.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/TEST-10.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

del yaml_data
