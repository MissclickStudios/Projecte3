# Issues Guidelines

- Issues should be added when a possible bug or crash is noticed.
- The issue shall follow the template provided in [Docs/Issues/issue_template.md](https://github.com/MissclickStudios/Projecte3/blob/main/Docs/Issues/issues_template.md).

## Issue Content

- **Title:** Brief description of the bug.
- **Bug Type:**

```
- Fatal: Gamebreaking, makes the game unplayable.
- Unintended Action: Something is not working as intended.
- Aesthetic: Audio/Visual asset is not being shown as intended.
```

- **Priority:**

```
- ASAP: The bug must be fixed as soon as possible.
- High: The bug must be fixed within 2 days.
- Medium: The bug must be fixed within a week.
- Low: The bug should be fixed whenever there are no higher priority bugs left.
```

- **Frequency:**

```
- Always: Unavoidable during playtime.
- High: Avoidable, but occurs more often than not.
- Average: Happens about 50% of the times.
- Low: Rarely occurs during playtime.
```

- **Description:** Detailed description of the bug. What and Where.
- **Reproduction:** Step by step explanation on how to reproduce the bug. The steps could be unknown.
- **Expected behavior:** Detailed description on what should happen.
- **Game Version:** The current version of the build. 
- **Additional Context:** Extra information that might be needed to fix the bug. This can include an image or gif.
- **Suggested Assignees:** Suggestion on which team member should be assigned to fix the bug.

## Bug Report Example:

```
Units pathfind erratically.

- Bug Type: Unintended Action.

- Priority: High.

- Frequency: High.

- Description: Unit paths erratically through the map.

- Reproduction: Left-Click on a unit and then right-click on an inaccessible tile.

- Expected behaviour: Unit should stay idle if the given destination is not accessible.

- Game Version: v0.4 

- Additional Context: Multiple screenshots of the bug reproduction process.

- Suggested Assignees: Code Lead
```

## Other Parameters

On top of using the template shown above, the development team will also use one of the features already integrated in GitHub Issues, the labels:

### Labels

Will be used to add visual information regarding the current state of each bug and whether or not fixing them is a priority.

#### Label Types

- **Label nºX:**
- **Label nºX:**
- **Label nºX:**
- **Label nºX:**
- **Label nºX:**
- **Label nºX:**
- **Label nºX:**
- **Label nºX:**

{Add a PNG of all the labels}

## Creating Issues

Issues can be created by anyone that finds a bug during any QA or while working on the project.

### Step by step

1. Go to https://github.com/MissclickStudios/Projecte3/issues and click on the "New Issue" button.
2. Copy the issue template from Root/Docs/Issues/issues_template.md .
3. Fill the title of the Issue with a brief summary of the bug/crash using the Title Template. Fill the Bug Type, Priority and Frequency brackets with the corresponding category found underneath the Title Template.
4. Now delete the the title guidelines from the issue description and start filling out the different categories to further describe the bug/crash.
5. An image can be dragged inside this text box to add it to the description. Put in in the "Additional context" category.
6. Add the appropriate Labels and add Assignees if it is adequate.

## Working on Issues

When making a push which is related to fixing an issue the commit summary will have to contain the issue's number like so:

```Fixed problem with camera in Issue #10```

## Closing Issues

Once an issue's problem is fixed the responsible for the fix can go to the issue's page and close it providing a comment if there is relevant information about said fix.