#include <idc.idc>

// ========================================================================
// Helper Functions
// ========================================================================

static ReadLine(hFile) {
    auto line, c; line = "";
    while ( (c = fgetc(hFile)) != -1 ) {
        if ( c == '\r' ) continue; if ( c == '\n' ) break;
        line = line + sprintf("%c", c);
    }
    if (c == -1 && line == "") { return 0; } return line;
}

static FindLastCharIndex(haystack, needleCharAscii) {
    auto i, lastIndex = -1, len = strlen(haystack);
    for (i = 0; i < len; i = i + 1) {
        auto charStr = substr(haystack, i, 1);
        if(strlen(charStr) == 1) {
             // Assuming byte(charStr) might be unreliable
             if (byte(charStr) == needleCharAscii) { lastIndex = i; }
        }
    }
    return lastIndex;
}

static GetBaseFileName(fullPath) {
    auto filename, baseName;
    auto lastBackslashIdx, lastSlashIdx, separatorPos;
    auto filenameLen, baseNameLen;

    // Minimal debugging or remove if stable
    // Message("  Debug (GetBaseFileName): Input fullPath: '" + fullPath + "'\n");

    lastBackslashIdx = FindLastCharIndex(fullPath, 92); // '\'
    lastSlashIdx = FindLastCharIndex(fullPath, 47);  // '/'
    separatorPos = -1;
    if (lastBackslashIdx != -1 && lastSlashIdx != -1) { separatorPos = max(lastBackslashIdx, lastSlashIdx); }
    else if (lastBackslashIdx != -1) { separatorPos = lastBackslashIdx; }
    else if (lastSlashIdx != -1) { separatorPos = lastSlashIdx; }

    if (separatorPos != -1) { filename = substr(fullPath, separatorPos + 1, -1); }
    else { filename = fullPath; }
    // Message("  Debug (GetBaseFileName): Filename after path removal: '" + filename + "'\n");

    // Fixed cut removal for .dll, .exe, .sys etc.
    filenameLen = strlen(filename);
    if (filenameLen > 4) {
        baseNameLen = filenameLen - 4;
        baseName = substr(filename, 0, baseNameLen);
    } else {
        baseName = filename;
    }

    if (baseName == ".") { baseName = ""; }

    // Message("  Debug (GetBaseFileName): Returning final baseName: '" + baseName + "'\n");
    return baseName;
}


static CompareStrings(s1, s2) {
    auto len1 = strlen(s1), len2 = strlen(s2), i;
    if (len1 != len2) { return 0; }
    for (i = 0; i < len1; i = i + 1) {
        // Stick with byte comparison for simplicity, acknowledge potential issues
        if (byte(s1 + i) != byte(s2 + i)) { return 0; }
    }
    return 1;
}


// ========================================================================
// Main Script Logic
// ========================================================================

static main() {
    auto COLOR, imageBase, currentModuleName, offsetValue, targetAddr;
    auto lineBuffer, fileHandle, filePath, count, readResult;
    auto plusPos, fileModuleName, offsetString;
    auto inputFileRaw;
    auto adjustedFileModuleName; // Variable to hold the name used for comparison
    auto fileModuleNameLen, suffix; // Variables for extension check

    Message("\n===== Module-Specific Coverage Highlighter (Flexible Extension) =====\n\n");

    COLOR = 0x99FF99; // Reset color to fixed Light Green
    filePath = AskFile(0, "C:\\path\\to\\your\\coverage.txt", "Select Coverage File (ModuleName+Offset format)");

    // --- Get Current Module Info ---
    imageBase = get_imagebase();
    inputFileRaw = GetInputFile();
    currentModuleName = GetBaseFileName(inputFileRaw); // Should return name without extension

    // --- Validate Module Info ---
    if (imageBase == BADADDR) { Warning("Error: Could not determine image base address. Aborting."); return; }
    if (currentModuleName == "") { Warning("Error: Could not determine base module name. Aborting."); return; }

    Message("Targeting module: '" + currentModuleName + "' (Base address: 0x" + sprintf("%x", imageBase) + ")\n"); // Combined message

    // --- File Processing ---
    if (filePath == 0 || filePath == "") { Warning("Operation cancelled: No coverage file selected."); return; }
    Message("Attempting to read coverage data from: " + filePath + "\n");
    fileHandle = fopen(filePath, "rt");
    if (fileHandle == 0) { Warning("Error: Failed to open file: " + filePath); return; }
    Message("File opened successfully. Processing entries...\n");

    count = 0;

    // --- Read File and Apply Highlights ---
    while ( (readResult = ReadLine(fileHandle)) != 0 ) {
        lineBuffer = readResult;
        plusPos = strstr(lineBuffer, "+");
        if (plusPos == -1) { continue; } // Skip lines without '+'

        // Extract module name and offset from the line
        fileModuleName = substr(lineBuffer, 0, plusPos - lineBuffer);
        offsetString = substr(lineBuffer, (plusPos - lineBuffer) + 1, -1);

        // --- Adjust fileModuleName if it has a known extension ---
        adjustedFileModuleName = fileModuleName; // Default to original
        fileModuleNameLen = strlen(fileModuleName);
        // Message("  Debug: Checking fileModuleName '" + fileModuleName + "' (len=" + sprintf("%d", fileModuleNameLen) + ") for extension.\n"); // Optional Debug

        if (fileModuleNameLen > 4) {
            // Extract the last 4 characters
            suffix = substr(fileModuleName, fileModuleNameLen - 4, -1);
            // Message("  Debug: Extracted suffix: '" + suffix + "'\n"); // Optional Debug

            // Check if the suffix matches known extensions (case-sensitive)
            // NOTE: IDC string comparison '==' works for literals
            if (suffix == ".dll" || suffix == ".exe" || suffix == ".sys") {
                Message("  Info: Found extension '" + suffix + "' in '" + fileModuleName + "'. Adjusting for comparison.\n");
                // Remove the last 4 characters
                adjustedFileModuleName = substr(fileModuleName, 0, fileModuleNameLen - 4);
            }
        }
        //Message("  Debug: Using '" + adjustedFileModuleName + "' for comparison against '" + currentModuleName + "'.\n"); // Optional Debug
        // --- End Adjustment ---

        // --- Compare the (potentially adjusted) file module name with the current module name ---

        if (CompareStrings(adjustedFileModuleName, currentModuleName) == 1) {
            offsetValue = xtol(offsetString);
            if (offsetValue == 0 && offsetString != "0" && offsetString != "0x0") {
                 Message("  Warning: Skipping potentially invalid offset '" + offsetString + "' in line: " + lineBuffer + "\n");
                 continue;
            }
            targetAddr = imageBase + offsetValue;
            if (is_loaded(targetAddr)) {
                 SetColor(targetAddr, CIC_ITEM, COLOR);
                 // MakeComm(targetAddr, "COVERED"); // Uncomment if comment is needed
                 count = count + 1;
            } else {
                 Message("  Warning: Calculated address 0x%x is outside loaded segments. Skipping highlight.\n", targetAddr);
            }
        }
        // Else: Names do not match, skip this line.
    } // End while loop

    // --- Cleanup and Final Report ---
    fclose(fileHandle);
    Message("----------------------------------------\n");
    Message("\nCoverage highlighting complete.\n");
    Message("Processed and highlighted %d addresses for module '%s'.\n", count, currentModuleName);
    Message("========================================\n");
}