/*
 * Copyright (C) 2023 TooManySugar
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>

#ifndef PROGRAM_NAME
#error PROGRAM_NAME not specified
#endif
#ifndef VERSION
#error VERSION not specified
#endif

#define PRETTY_NAME "Itch! Save Coder"

#define BUFFER_SIZE 1 << 14

typedef struct run_config_s {
    FILE *out;
    bool show_help;
    bool print_version;
    FILE *in;
} run_config;

run_config run_config_default()
{
    return (run_config){
        stdout,
        false,
        false,
        NULL,
    };
}

void close_config(run_config *cfg)
{
    if (cfg->out != stdout && cfg->out != NULL)
        fclose(cfg->out);

    if (cfg->in != NULL)
        fclose(cfg->in);
}

bool check_long_option(bool is_long_option, const char* option,
    const char* value)
{
    if (!is_long_option)
        return true;

    if (strcmp(option, value) == 0)
        return true;

    fprintf(stderr,
        "ERROR: Unknown option '%s'\n",
        option);
    return false;
}

bool parse_options(run_config *dst, const char* option, int* i, int argc,
    char **argv)
{
    int j = 0;
    bool is_long_option = false;
    if (option[j] == '-') {
        is_long_option = true;
        j++;
    }

    for ( ; option[j] != '\0'; j++)
    {
        switch (option[j])
        {
        case 'h':
            if (!check_long_option(is_long_option, &option[j], "help"))
                return false;

            dst->show_help = true;
            break;

        case 'v':
            if (!check_long_option(is_long_option, &option[j], "version"))
                return false;

            dst->print_version = true;
            break;

        case 'o':
            if (!check_long_option(is_long_option, &option[j], "output"))
                return false;

            if (dst->out != stdout)
            {
                fprintf(stderr,
                    "ERROR: Multiple output files not allowed\n");
                return false;
            }
            *i += 1;
            if (*i >= argc)
            {
                fprintf(stderr,
                    "ERROR: option '%s' expects filename after it\n",
                    is_long_option ? "output" : "o");
                return false;
            }
            dst->out = fopen(argv[*i], "wb");
            if (dst->out == NULL)
            {
                fprintf(stderr,
                    "ERROR: Can't open file '%s' for write: %s\n",
                    argv[*i], strerror(errno));
                return false;
            }
            break;

        default:
            fprintf(stderr,
                "ERROR: Unknown option '%c'\n",
                option[j]);
            return false;
        }

        if (is_long_option)
            break;
    }

    return true;
}

inline bool on_parse_args_false(run_config *dst)
{
    close_config(dst);
    return false;
}

bool parse_args(run_config *dst, int argc, char **argv)
{
    int i = 1;
    int j = 0;
    for (; i < argc; i++)
    {
        const char* arg = argv[i];

        j = 0;
        if (arg[0] == '-')
        {
            j++;
            if (arg[1] == '\0') {
                fprintf(stderr,
                    "ERROR: After '-' in argument %d expected short option "
                    "or '-'\n",
                    i);
                return on_parse_args_false(dst);
            }

            if (!parse_options(dst, &arg[1], &i, argc, argv))
            {
                return on_parse_args_false(dst);
            }

            continue;
        }

        if (dst->in != NULL)
        {
            fprintf(stderr,
                "ERROR: Multiple input files not allowed\n");
            return on_parse_args_false(dst);
        }
        dst->in = fopen(argv[i], "rb");
        if (dst->in == NULL)
        {
            fprintf(stderr,
                "ERROR: Can't open file '%s' for read: %s\n",
                argv[i], strerror(errno));
            return on_parse_args_false(dst);
        }
    }

    if (dst->show_help || dst->print_version)
        return true;

    if (dst->in == NULL)
    {
        fprintf(stderr,
            "ERROR: No input file specified\n");
        return on_parse_args_false(dst);
    }

    return true;
}

void help(FILE* f)
{
    fprintf(f,"\
%s is a tool to decode and encode back save files for game Itch!\n\
\n\
Usage:\n\
    %s [OPTIONS]... [INPUT_FILE]\n\
\n\
OPTIONS are:\n\
    -h    --help            Print this usage info to output\n\
    -v    --version         Print program version info to output\n\
    -o    --output <FILE>   Specify output file, console(stdout) by default\n\
\n\
    If neither 'help' or 'version' options are specified INPUT_FILE required\n\
\n\
INPUT_FILE - path to input file either save data or decoded xml data\n",
    PRETTY_NAME, PROGRAM_NAME);
}

void version(FILE* f)
{
    fprintf(f, PRETTY_NAME " v" VERSION "\n");
}

inline void code(char* b)
{
    *b = *b ^ 34;
}

int run_end(run_config *cfg, char* buf, int ret_val)
{
    close_config(cfg);

    if (buf != NULL)
        free(buf);

    return ret_val;
}

int run(run_config cfg)
{
    char *buf = NULL;
    if (cfg.show_help)
    {
        help(cfg.out);
        return run_end(&cfg, buf, 0);
    }
    if (cfg.print_version)
    {
        version(cfg.out);
        return run_end(&cfg, buf, 0);
    }

    buf = (char*)malloc(BUFFER_SIZE);
    if (buf == NULL)
    {
        fprintf(stderr,
            "ERROR: Can't allocate working buffer with size %d bytes\n",
            BUFFER_SIZE);
        return run_end(&cfg, buf, 1);
    }

    int bytes_read = 0;
    int bytes_written = 0;
    do
    {
        bytes_read = fread((void*)buf, 1, BUFFER_SIZE, cfg.in);
        if (ferror(cfg.in))
        {
            fprintf(stderr,
                "ERROR: reading from input file\n");
            return run_end(&cfg, buf, 1);
        }

        for (int i = 0; i < bytes_read; i++)
            code(buf + i);

        bytes_written = fwrite(buf, 1, bytes_read, cfg.out);
        if (bytes_written != bytes_read)
        {
            fprintf(stderr,
                "ERROR: writing to output file\n");
            return run_end(&cfg, buf, 1);
        }
    }
    while(!feof(cfg.in));

    return run_end(&cfg, buf, 0);
}

int main(int argc, char **argv)
{
    run_config config = run_config_default();
    if (!parse_args(&config, argc, argv))
    {
        help(stderr);
        return 1;
    }

    return run(config);
}