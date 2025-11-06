#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> 
#include <sys/stat.h> 

#define EXIT_SUCCESS 0 
#define EXIT_FAILURE 1

#define MAX_PATH 1024

// --- Methods ---
/**
 * @brief Checks if a file exists and is a regular file.
 * @param filename The path to the file.
 * @return 1 if the file exists and is a regular file, 0 otherwise.
 */
int file_exists(const char *filename) 
{
    struct stat buffer;
    if (stat(filename, &buffer) != 0) 
    {
        return 0; // File does not exist or error
    }
    return S_ISREG(buffer.st_mode); // Check if it's a regular file
}

/**
 * @brief Deletes a file and checks for success.
 * @param filename The path to the file.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int delete_file(const char *filename) 
{
    if (remove(filename) != 0) 
    {
        // Only warn, don't necessarily fail the driver on a delete error
        // as the compilation itself succeeded.
        fprintf(stderr, "Warning: Could not delete temporary file: %s\n", filename);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief A stub function for the actual compiler pass (Lexing, Parsing, Assembly Gen).
 * @param input_file The preprocessed file (.i).
 * @param output_file The assembly file (.s).
 * @param option The special compiler option (--lex, --parse, --codegen, or NULL).
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int run_compiler_pass(const char *input_file, const char *output_file, const char *option) 
{
    if (option) 
    {
        // For now, we assume success for the stubbed out passes.
        // The real compiler would return non-zero on error.
        if (strcmp(option, "--lex") == 0) 
        {
        
            fprintf(stderr, "Stub: Running lexer on %s.\n", input_file);
        } else if (strcmp(option, "--parse") == 0) 
        {
            fprintf(stderr, "Stub: Running lexer and parser on %s.\n", input_file);
        } else if (strcmp(option, "--codegen") == 0) 
        {

            fprintf(stderr, "Stub: Running lexer, parser, and assembly generation on %s.\n", input_file);
        }
        return EXIT_SUCCESS;
    }

    // This is the full compilation path (no special option).
    fprintf(stderr, "Stub: Compiling %s to assembly file %s.\n", input_file, output_file);
    
    // For the stub, we simulate the compiler pass by creating an empty assembly file,
    // which `gcc` will assemble and link without issue.
    FILE *fp = fopen(output_file, "w");
    if (fp == NULL) 
    {
        perror("Failed to create stub assembly file");
        return EXIT_FAILURE;
    }
    // Write a minimal, valid assembly stub (just a main label)
    fprintf(fp, "\t.globl main\n");
    fprintf(fp, "main:\n");
    fprintf(fp, "\tret\n"); 
    fclose(fp);

    // In a real compiler, this is where a non-zero exit code would signify an error.
    return EXIT_SUCCESS;
}

// --- Main Driver Logic --- 

/**
 * @brief The main function for the compiler driver.
 * @param argc The argument count.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char *argv[]) 
{
    if (argc < 2 || argc > 3) 
    {
        fprintf(stderr, "Usage: %s <path/to/source.c> [--lex | --parse | --codegen | -S]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *input_path =argv[1];
    char *compiler_option = NULL;
    int emit_assembly_only = 0;

    if (argc ==  3)
    {
        if (strcmp(argv[2], "--lex") == 0 || strcmp(argv[2], "--parse") == 0 || strcmp(argv[2], "--codegen") == 0) 
        {
            compiler_option = argv[2];
        } 
        else if (strcmp(argv[2], "-S") == 0) 
        {
            emit_assembly_only = 1;
        } 
        else 
        {
            fprintf(stderr, "Error: Unknown compiler option: %s\n", argv[2]);
            return EXIT_FAILURE;
        }
    }

    if (!file_exists(input_path))
    {
        fprintf(stderr, "Error: Input file does not exist or is not a regular file: %s\n", input_path);
        return EXIT_FAILURE;
    }

    char input_copy[MAX_PATH];
    strncpy(input_copy, input_path, MAX_PATH - 1);
    input_copy[MAX_PATH - 1] = '\0';

    char *dir = dirname(input_copy);

    char base_copy[MAX_PATH];
    strncpy(base_copy, input_path, MAX_PATH - 1);
    base_copy[MAX_PATH - 1] = '\0';

    char *base = basename(base_copy);

    char *dot = strrchr(base, '.');
    size_t name_len = (dot != NULL) ? (size_t)(dot - base) : strlen(base);

    char final_name[MAX_PATH];
    snprintf(final_name, MAX_PATH, "%s/%.*s", dir, (int)name_len, base);

    char preprocessed_file[MAX_PATH];
    char assembly_file[MAX_PATH]; 

    snprintf(preprocessed_file, MAX_PATH, "%s.i", final_name);
    snprintf(assembly_file, MAX_PATH, "%s.s", final_name);

    char *output_file = emit_assembly_only ? assembly_file : final_name;

    fprintf(stderr, "Step 1: Preprocessing...\n");
    char preprocess_cmd[MAX_PATH * 3];

    snprintf(preprocess_cmd, sizeof(preprocess_cmd), "gcc -E -P %s -o %s", 
             input_path, preprocessed_file);

    if (system(preprocess_cmd) != 0) 
    {
        fprintf(stderr, "Error: Preprocessing failed.\n");
        return EXIT_FAILURE;
    }

    if (compiler_option) 
    {
        int result = run_compiler_pass(preprocessed_file, NULL, compiler_option);
        delete_file(preprocessed_file); // Clean up
        return result;
    }

    // --- 4. Compiler Pass (Step 2 - Stubbed) ---
    fprintf(stderr, "Step 2: Compiling to Assembly (Stub)...\n");
    int compiler_result = run_compiler_pass(preprocessed_file, assembly_file, NULL);
    
    delete_file(preprocessed_file); // Delete the preprocessed file

    if (compiler_result != 0) 
    {
        fprintf(stderr, "Error: Compiler stub failed.\n");
        // No need to delete assembly_file here, as run_compiler_pass is 
        // responsible for not creating it on failure.
        return EXIT_FAILURE;
    }

    // If -S is used, stop after generating the assembly file.
    if (emit_assembly_only) 
    {
        fprintf(stderr, "Outputting Assembly file: %s\n", assembly_file);
        return EXIT_SUCCESS;
    }

    // --- 5. Assemble and Link (Step 3) ---
    fprintf(stderr, "Step 3: Assembling and Linking...\n");
    char assemble_link_cmd[MAX_PATH * 3];
    // Command: gcc ASSEMBLY_FILE -o OUTPUT_FILE
    snprintf(assemble_link_cmd, sizeof(assemble_link_cmd), "gcc %s -o %s", 
             assembly_file, final_name);

    int link_status = system(assemble_link_cmd);
    
    delete_file(assembly_file); // Delete the assembly file

    if (link_status != 0) 
    {
        fprintf(stderr, "Error: Assembly/Linking failed.\n");
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Success: Executable created at %s\n", final_name);
    return EXIT_SUCCESS;

}