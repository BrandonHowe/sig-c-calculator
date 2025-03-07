#include <stdio.h>
#include <stdlib.h>

void read_file(const char* file_path)
{
    FILE* file = fopen(file_path, "r"); // Open the file path in read mode
    if (!file)
    {
        printf("File could not be opened!\n");
        return;
    }

    fseek(file, 0, SEEK_END); // Navigate to the end of the file
    int len = ftell(file); // Ask windows where we are
    rewind(file);

    char* buffer = malloc(len + 1);

    fread(buffer, 1, len, file);

    fclose(file);
    buffer[len] = 0;
}

int main(void)
{
    read_file("/Users/brandonhowe/Documents/Projects/SIGC-live-calculator/program.sigc");

    return 0;
}
