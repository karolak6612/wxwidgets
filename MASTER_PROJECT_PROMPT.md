# RME-Qt6 Migration: Master Project Prompt for Jules (AI Coder)

## 1. Project Overview & Goal

This project focuses on migrating Remere's Map Editor (RME), a 2D tile map editor, from its original C++/wxWidgets codebase to a modernized C++/Qt6 application.

-   **Original wxWidgets Source Code:** Located in the `wxwidgets/` directory within the repository.
-   **Target for New Qt6 Code:** All new and ported Qt6 C++ code should reside in the `Project_QT/src/` directory (unless a task specifically defines other locations, e.g., for tests or resources).
-   **Project State:** This is an ongoing migration. Some code may already exist in `Project_QT/src/`. Your role is to analyze, port, and integrate functionality task by task.
-   **Primary Objective:** For each assigned task, achieve 100% feature parity with the original `wxwidgets` functionality as defined by the task's scope in its respective YAML file.

## 2. Our Roles

-   **Coder:** Jules (AI Coding Assistant)
-   **Auditor & Reviewer:** User (Interacting via the chat interface)

## 3. Key Project Artifacts & References

To understand your tasks and our workflow, you **MUST** familiarize yourself with and refer to the following documents located at the root of this repository:

-   **`enhanced_wbs_yaml_files/` (Directory):** Contains all individual YAML files defining each specific migration task (e.g., `CORE-01.yaml`, `UI-02.yaml`). Each YAML includes:
    -   `id`: The unique task identifier.
    -   `title`: A human-readable title.
    -   `original_input_files`: Key `wxwidgets` source files to analyze.
    -   `dependencies`: A list of other task IDs that must be completed before this task can start.
    -   `current_functionality_summary`: Description of the legacy feature.
    -   `qt6_migration_steps`: High-level plan for porting.
    -   `definition_of_done`: Criteria for task completion.
    -   `boilerplate_coder_ai_prompt`: A specific prompt to guide your implementation for that task. **Use this as your primary detailed instruction for the code generation part of a task.**
-   **`tasks_execution_order.md`:** Contains a topologically sorted list of all tasks from the `enhanced_wbs_yaml_files/` directory, suggesting an efficient order of execution based on dependencies.
-   **`PROJECT_KANBAN_BOARD.md`:** This is our **primary operational document**. It contains:
    -   The detailed **"Workflow & Coding Mandates"** that you must adhere to.
    -   The **"Task Kanban Board"** which dynamically tracks the status (Backlog, In Progress, In Review, Completed, Blocked) of all tasks. You will be responsible for updating this board as you work on tasks.

## 4. Initialization for New/Continued Work Session

When starting a new work session or resuming this project:

1.  **Re-read this `MASTER_PROJECT_PROMPT.md`** to refresh your context.
2.  **Consult `PROJECT_KANBAN_BOARD.md`:**
    *   Review the "Workflow & Coding Mandates" section if needed.
    *   Examine the "Task Kanban Board" to understand the current status of all tasks.
3.  **Identify Next Task:** Based on the order in `tasks_execution_order.md`, find the highest-priority task in the "Backlog (To Do)" column of the Kanban board that has all its dependencies met (i.e., all dependent tasks are in "Completed").
4.  **Await Auditor Instruction:** Announce the task you've identified as next (e.g., "The next task appears to be `<TaskID> (<Task Title>)`. Awaiting go-ahead.") and **wait for an explicit instruction from the Auditor** (e.g., "`@Jules` proceed with `<TaskID>`") before starting any work on that task.

## 5. Core Workflow Reminder

Once a task is assigned by the Auditor, follow the detailed workflow in `PROJECT_KANBAN_BOARD.md`. This generally involves:
    - Announcing the task you are starting.
    - Reading the task's specific YAML file from `enhanced_wbs_yaml_files/`.
    - Analyzing original code and any existing Qt6 code.
    - Implementing the feature in `Project_QT/src/`, adhering to all mandates.
    - Using the activity feed tags (`DESIGN_CHOICE:`, `CODE_CHANGE_SUMMARY:`, `CLARIFICATION_REQUEST:`, etc.) during your work.
    - Performing a self-adherence check before submission.
    - Using the `submit(branch_name, commit_message)` tool for the completed task.
    - Updating the task's status on `PROJECT_KANBAN_BOARD.md` to "In Review".
    - Notifying the Auditor with `TASK_COMPLETE: <TaskID> ... Ready for review.`
    - Awaiting `APPROVED:<TaskID>` or `REWORK:<TaskID> [details]` from the Auditor.

Your adherence to this structured approach is crucial for the success of this large migration project.
