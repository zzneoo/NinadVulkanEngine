# Scenario Instructions

This file contains scenario-specific instructions and recorded user preferences.

## User Preferences

### Execution Style
- Flow Mode: Guided (default)

### Source Control
- Prepare changes but do not commit; show diffs/patches to the user before any commits or branch creation.
- If build/tests pass, notify user and await explicit approval before committing or pushing.

### Notification
- User requested to be informed before any push; do not push without explicit approval.

## Purpose
Resolve C++ build issues introduced by an MSVC Build Tools upgrade and validate fixes following the Assess-Plan-Execute workflow.
