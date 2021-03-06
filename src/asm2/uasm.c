/*
 * uasm.c
 * 
 * program entry point and command line interface
 * 
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "uasm.h"
#include "assemble.h"

int main(int argc, char *argv[]) {
    asm_context_t asm_context = {
        .current_addr  = INTTABLE_SIZE,
        .current_line  = 0,
        .current_file  = NULL,
        .output_file   = "u.out",
        .gen_debuginf  = FALSE,
        /* init size of read_buf is 0xff, but getline() might enlarge it */
        .read_buf      = malloc(sizeof(char) * 0xff),
        .read_buf_size = sizeof(char) * 0xff
    };

    int wait_on_error = FALSE;

    /* parse command line options */
    opterr = 0; // suppress getopt error messages
    int c = 0;  // current command line option

    while ((c = getopt(argc, argv, "o:ghw")) != -1) {
        switch (c) {
            case 'o': // output file
                asm_context.output_file = optarg;
                break;
            case 'g':
                asm_context.gen_debuginf = TRUE;
                break;
            case 'h':
                fprintf(stdout, "Usage: %s [-o outfile] [-g] [-w] file(s)\n", argv[0]);
                fprintf(stdout, "Options:\n");
                fprintf(stdout, "  -o <outfile> Place the output into <outfile>\n");
                fprintf(stdout, "  -g           Produce debugging information\n");
                fprintf(stdout, "  -w           Wait on error for ENTER key\n");
                fprintf(stdout, "  -h           Print this text and quit\n");
                exit(EXIT_SUCCESS);
                break;
            case 'w':
                wait_on_error = TRUE;
                break;
            default:
                if (optopt == 'o')
                    fprintf(stderr, "The option -o expects an output file name.\n");
                else
                    fprintf(stderr, "Unknown option: -%c. Try -h to show usage.\n", optopt);

                exit(EXIT_FAILURE);
        }
    }

    if (argc - optind < 1) {
        /* We need at least one assembly source */
        fprintf(stderr, "No program files specified, ");
        fprintf(stderr, "run \"%s -h\" for usage info.\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int success = assemble(&asm_context, &(argv[optind]), argc - optind);

    if (!success && wait_on_error) {
        fprintf(stderr, "Press ENTER to quit.\n");
        getchar();
    }

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

void print_error(asm_context_t *cntxt, const char *format, ...) {
    fprintf(stderr, "%s, line %d: ", cntxt->current_file, cntxt->current_line);

    va_list arg_list;

    va_start(arg_list, format);

    vfprintf(stderr, format, arg_list);
    fprintf(stderr, "\n");

    va_end(arg_list);
}
