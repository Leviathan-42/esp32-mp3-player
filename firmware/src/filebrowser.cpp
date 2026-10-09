#include "filebrowser.h"
#include "pins.h"
#include <SD.h>
#include <SPI.h>

char fileList[MAX_FILES][MAX_PATH];
int  fileCount = 0;

static bool hasSupportedAudioExtension(const char* name) {
    const char* ext = strrchr(name, '.');
    if (!ext) return false;

    char lower[8];
    strlcpy(lower, ext, sizeof(lower));
    for (size_t i = 0; lower[i]; i++) {
        lower[i] = tolower(lower[i]);
    }

    return strcmp(lower, ".mp3") == 0 ||
           strcmp(lower, ".aac") == 0 ||
           strcmp(lower, ".flac") == 0 ||
           strcmp(lower, ".wav") == 0;
}

int scanSD() {
    fileCount = 0;

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);

    if (!SD.begin(SD_CS, SPI, 12000000)) {
        Serial.println("SD init failed");
        return 0;
    }

    File root = SD.open("/");
    if (!root) {
        Serial.println("Failed to open SD root");
        return 0;
    }

    File file = root.openNextFile();
    while (file && fileCount < MAX_FILES) {
        char name[MAX_PATH];
        strlcpy(name, file.name(), MAX_PATH);

        if (file.isDirectory() || name[0] == '.' || strncmp(name, "._", 2) == 0) {
            file.close();
            file = root.openNextFile();
            continue;
        }

        if (hasSupportedAudioExtension(name)) {
            snprintf(fileList[fileCount], MAX_PATH, "/%s", name);
            fileCount++;
        }
        file.close();
        file = root.openNextFile();
    }

    root.close();
    Serial.printf("Found %d audio files\n", fileCount);
    return fileCount;
}
