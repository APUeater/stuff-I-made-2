#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAM_SIZE 1024 * 640 // 640 KB of RAM, similar to MS-DOS limits
#define MAX_NAME_LENGTH 12 // Adjusted for typical MS-DOS file name length
#define MAX_FILES 64 // Reduced to fit in memory constraints
#define MAX_FILE_SIZE 1024 * 32 // Limit file size to 32 KB to reflect typical MS-DOS constraints

typedef struct File {
    char name[MAX_NAME_LENGTH];
    char *data;
    size_t size;
} File;

typedef struct Directory {
    char name[MAX_NAME_LENGTH];
    struct Directory *parent;
    struct Directory *subdirs[MAX_FILES];
    File *files[MAX_FILES];
    int subdir_count;
    int file_count;
} Directory;

typedef struct FileSystem {
    char ram[RAM_SIZE];
    Directory root;
    Directory *current_dir;
} FileSystem;

// Initialize the file system
void init_filesystem(FileSystem *fs) {
    memset(fs->ram, 0, RAM_SIZE);
    strcpy(fs->root.name, "root");
    fs->root.parent = NULL;
    fs->root.subdir_count = 0;
    fs->root.file_count = 0;
    fs->current_dir = &fs->root;
}

// Create a new directory
Directory *create_directory(Directory *parent, const char *name) {
    Directory *dir = (Directory *)malloc(sizeof(Directory));
    if (dir == NULL) {
        perror("Failed to allocate memory for directory");
        exit(EXIT_FAILURE);
    }
    strcpy(dir->name, name);
    dir->parent = parent;
    dir->subdir_count = 0;
    dir->file_count = 0;
    return dir;
}

// Create a new file
File *create_file(const char *name, const char *data, size_t size) {
    File *file = (File *)malloc(sizeof(File));
    if (file == NULL) {
        perror("Failed to allocate memory for file");
        exit(EXIT_FAILURE);
    }
    strcpy(file->name, name);
    file->data = (char *)malloc(size);
    if (file->data == NULL) {
        perror("Failed to allocate memory for file data");
        free(file);
        exit(EXIT_FAILURE);
    }
    memcpy(file->data, data, size);
    file->size = size;
    return file;
}

// Add a directory to a parent directory
void add_directory(Directory *parent, Directory *subdir) {
    if (parent->subdir_count < MAX_FILES) {
        parent->subdirs[parent->subdir_count++] = subdir;
    } else {
        printf("Cannot add more directories. Maximum limit reached.\n");
    }
}

// Add a file to a directory
void add_file(Directory *dir, File *file) {
    if (dir->file_count < MAX_FILES) {
        dir->files[dir->file_count++] = file;
    } else {
        printf("Cannot add more files. Maximum limit reached.\n");
    }
}

// List contents of a directory
void list_directory(const Directory *dir) {
    printf("Directory: %s\n", dir->name);
    for (int i = 0; i < dir->file_count; ++i) {
        printf("  File: %s\n", dir->files[i]->name);
    }
    for (int i = 0; i < dir->subdir_count; ++i) {
        printf("  Directory: %s\n", dir->subdirs[i]->name);
    }
}

// Change current directory
void change_directory(FileSystem *fs, const char *path) {
    if (strcmp(path, "..") == 0) {
        if (fs->current_dir->parent != NULL) {
            fs->current_dir = fs->current_dir->parent;
        } else {
            printf("Already at root directory.\n");
        }
    } else {
        for (int i = 0; i < fs->current_dir->subdir_count; ++i) {
            if (strcmp(fs->current_dir->subdirs[i]->name, path) == 0) {
                fs->current_dir = fs->current_dir->subdirs[i];
                return;
            }
        }
        printf("Directory not found: %s\n", path);
    }
}

// Get current directory path
void get_current_path(const Directory *dir, char *path) {
    if (dir->parent != NULL) {
        get_current_path(dir->parent, path);
        strcat(path, "/");
    }
    strcat(path, dir->name);
}

// Find a file by name in the current directory
File *find_file(Directory *dir, const char *name) {
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->files[i]->name, name) == 0) {
            return dir->files[i];
        }
    }
    return NULL;
}

// Edit file content
void edit_file(File *file) {
    char buffer[MAX_FILE_SIZE];
    printf("Editing file '%s'. Type 'SAVE' to save changes and 'CANCEL' to discard changes.\n", file->name);

    size_t total_size = 0;
    while (1) {
        if (total_size >= MAX_FILE_SIZE - 1) {
            printf("File size limit reached.\n");
            break;
        }

        printf(">> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        // Check for 'SAVE' or 'CANCEL'
        if (strcmp(buffer, "SAVE\n") == 0) {
            file->data[total_size] = '\0'; // Null-terminate the data
            file->size = total_size;
            printf("File saved.\n");
            break;
        } else if (strcmp(buffer, "CANCEL\n") == 0) {
            printf("Editing cancelled.\n");
            break;
        }

        // Append to file data
        size_t len = strlen(buffer);
        if (total_size + len < MAX_FILE_SIZE) {
            memcpy(file->data + total_size, buffer, len);
            total_size += len;
        } else {
            printf("Not enough space to add more data.\n");
        }
    }
}

// Handle user input
void handle_command(FileSystem *fs, const char *command) {
    char cmd[256];
    char arg1[MAX_NAME_LENGTH];
    char arg2[MAX_NAME_LENGTH];

    int result = sscanf(command, "%s %s %s", cmd, arg1, arg2);
    if (strcmp(cmd, "mkdir") == 0 && result >= 2) {
        Directory *new_dir = create_directory(fs->current_dir, arg1);
        add_directory(fs->current_dir, new_dir);
        printf("Directory '%s' created.\n", arg1);
    } else if (strcmp(cmd, "touch") == 0 && result >= 3) {
        File *new_file = create_file(arg1, arg2, strlen(arg2) + 1);
        add_file(fs->current_dir, new_file);
        printf("File '%s' created.\n", arg1);
    } else if (strcmp(cmd, "ls") == 0) {
        list_directory(fs->current_dir);
    } else if (strcmp(cmd, "cd") == 0 && result >= 2) {
        change_directory(fs, arg1);
    } else if (strcmp(cmd, "pwd") == 0) {
        char path[256] = "";
        get_current_path(fs->current_dir, path);
        printf("Current directory: /%s\n", path);
    } else if (strcmp(cmd, "edit") == 0 && result >= 2) {
        File *file = find_file(fs->current_dir, arg1);
        if (file != NULL) {
            edit_file(file);
        } else {
            printf("File not found: %s\n", arg1);
        }
    } else if (strcmp(cmd, "quit") == 0) {
        printf("Exiting program.\n");
        exit(0);
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}

// Main function to demonstrate the file system
int main() {
    FileSystem fs;
    init_filesystem(&fs);

    char command[256];
    char path[256];
    printf("File System CLI\n");
    printf("Commands: mkdir <name>, touch <name> <data>, ls, cd <dir>, pwd, edit <file>, quit\n");

    // Display MS-DOS memory and file size constraints
    printf("\n--- MS-DOS Memory and File Size Constraints ---\n");
    printf("RAM Size: 640 KB\n");
    printf("File Size Limit: 32 KB (reflecting typical MS-DOS constraints)\n");
    printf("Maximum Number of Files/Directories: 64\n");
    printf("------------------------------------------------\n");

    while (1) {
        // Update prompt with current directory
        path[0] = '\0';  // Reset path
        get_current_path(fs.current_dir, path);
        printf("/%s> ", path);
        if (fgets(command, sizeof(command), stdin) != NULL) {
            command[strcspn(command, "\n")] = 0; // Remove newline
            handle_command(&fs, command);
        }
    }

    return 0;
}
