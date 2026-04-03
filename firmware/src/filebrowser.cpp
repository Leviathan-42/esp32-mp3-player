#include "filebrowser.h"
#include "pins.h"
#include <SdFat.h>

char fileList[MAX_FILES][MAX_PATH];
int  fileCount = 0;

static SdFat sd;

int scanSD() {
    fileCount = 0;

    if (!sd.begin(SD_CS, SD_SCK_MHZ(25))) {
        Serial.println("SD init failed");
        return 0;
    }

    SdFile root, file;
    if (!root.open("/")) {
        Serial.println("Failed to open SD root");
        return 0;
    }

    while (file.openNext(&root, O_RDONLY) && fileCount < MAX_FILES) {
        char name[MAX_PATH];
        file.getName(name, MAX_PATH);

        // Only include .mp3 files (case-insensitive check on extension)
        int len = strlen(name);
        if (len > 4) {
            char ext[5];
            strncpy(ext, name + len - 4, 4);
            ext[4] = '\0';
            for (int i = 0; i < 4; i++) ext[i] = tolower(ext[i]);
            if (strcmp(ext, ".mp3") == 0) {
                snprintf(fileList[fileCount], MAX_PATH, "/%s", name);
                fileCount++;
            }
        }
        file.close();
    }

    root.close();
    Serial.printf("Found %d MP3 files\n", fileCount);
    return fileCount;
}
