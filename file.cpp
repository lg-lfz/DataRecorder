#include "file.h"
#include <LittleFS.h>

int initFileSystem()
{
    Serial.printf("Init LittleFS Filesystem...\n");
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        Serial.flush();
        if (LittleFS.format())
        {
            Serial.println("File system formatted successfully. Rebooting...");
            ESP.restart(); // Restart the ESP to apply changes
            return -1;
        }
        else
        {
            Serial.println("Failed to format the file system.");
            return -1; // Exit setup if formatting fails
        }
    }
    Serial.println("LittleFS mounted successfully");
    Serial.flush();
    return 0;
}

void checkAvailableFlashSpace()
{
  FSInfo fs_info;
  LittleFS.info(fs_info);
  Serial.printf("Total space:      %u bytes\n", fs_info.totalBytes);
  Serial.printf("Used space:       %u bytes\n", fs_info.usedBytes);
  Serial.printf("Available space:  %u bytes\n", fs_info.totalBytes - fs_info.usedBytes);
  LittleFS.end();
}

void listDir(const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    Dir dir = LittleFS.openDir(dirname);

    while (dir.next())
    {
        String fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        Serial.printf("Filename: %s, Size: %d bytes\n", fileName.c_str(), fileSize);

        if (levels)
        {
            File file = dir.openFile("r");
            if (file.isDirectory())
            {
                listDir(fileName.c_str(), levels - 1);
            }
            file.close();
        }
    }
    checkAvailableFlashSpace();
}

void writeFile(const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = LittleFS.open(path, "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("File written successfully");
    }
    else
    {
        Serial.println("Write failed");
    }
    file.close();
}

void readFile(const char *path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = LittleFS.open(path, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }
    Serial.println("File content:");
    while (file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}
