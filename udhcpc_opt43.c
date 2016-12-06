/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/


/* 
***************************************************************************************
*  udhcpc_opt43_gen.c
*  ------------------
*  This program reads the file /opt/udhcpc.vendor_specific and then
*  executes udhcpc with the -x option filled with the converted data.
*  
* ***************************************************************************************
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE        1
#define FALSE       0
#define LINE_LIMIT  1024
#define SUBOPT_STR  "SUBOPTION"

#ifdef ETC_UDHCPC
#define UDHCPC_OPT43_FILE "/etc/udhcpc.vendor_specific"
#else
#define UDHCPC_OPT43_FILE "/opt/persistent/udhcpc/wan.option43"
#endif

void display_help(void)
{
	printf("udhcpc_opt43 - This program reads the file %s and then\n", UDHCPC_OPT43_FILE);
    printf("                   executes udhcpc with the -x option filled with the converted data.\n");
    printf("                   Any command line options are passed directly to udhcpc.");
}

int read_file_line(FILE *inp_file, unsigned char *out_str, int read_limit)
{
    int read_char;
    int num_chars   = 0;
    int done        = FALSE;

    do
    {
        /* Get a single character from the file */
        read_char = fgetc(inp_file);

        /* Check for line terminator */
        if ((read_char == '\n') || (read_char == '\r'))
        {
            done = TRUE;

            /* Found a CR or LF, check to see if there is either a trailing CR or LF (support DOS format) */
            read_char = fgetc(inp_file);
            if ((read_char != '\n') && (read_char != '\r'))
            {
                /* Next char is not a line terminator, put it back */
                ungetc(read_char, inp_file);
            }
        }
        else if (read_char != EOF)
        {
            /* Copy the file input to output string */
            *out_str++ = (unsigned char) read_char;
            num_chars++;
        }

    } while (!done && (read_char != EOF) && (num_chars < read_limit)); 

    /* Null terminate the string */
    *out_str = '\0';

    /* Indicate if EOF is reached */
    if ((num_chars == 0) && (read_char == EOF))
        return(FALSE);
    else
        return(TRUE);
}

int create_opt43_string(FILE *inp_file, unsigned char *out_str)
{
    int retval = 0;
    int i;
    unsigned char buff[LINE_LIMIT];
    unsigned char inp_line[LINE_LIMIT];
    unsigned char sub_opt[LINE_LIMIT];
    unsigned int  sub_opt_num;
    unsigned int  sub_opt_len;
    unsigned char sub_opt_str[LINE_LIMIT];
    unsigned char hex_char[3];
    unsigned char *ptr;

    /* Start the output string off as '-x 43:' */
    strcpy(out_str, "-x 43:");
    out_str += 6;

    /* Read one line from the file */
    while(read_file_line(inp_file, inp_line, LINE_LIMIT))
    {
        if (strlen(inp_line) > 0)
        {
            if ((ptr = strstr(inp_line, SUBOPT_STR)) != NULL)
            {
                /* Found a suboption, get the suboption number */
                ptr += strlen(SUBOPT_STR);

                i = 0;
                /* Make sure a numeric follows */
                while ((*ptr >= 0x30) && (*ptr <= 0x39))
                {
                    sub_opt[i++] = *ptr++;
                }
                sub_opt[i] = '\0';

                /* skip over whitespace */
                while ((*ptr == ' ') || (*ptr == '\t'))
                    ptr++;

                /* Convert the following string to ASCII Hex bytes */
                i = 0;
                sub_opt_len = 0;
                while ((*ptr >= 0x20) && (*ptr <= 0x7E))
                {
                    sprintf(hex_char, "%02X", *ptr);
                    sub_opt_str[i++] = hex_char[0];
                    sub_opt_str[i++] = hex_char[1];
                    sub_opt_str[i++] = ':';
                    ptr++;
                    sub_opt_len++;
                }
                /* Null terminate the sub_opt_string (and overwrite the trailing ':') */
                sub_opt_str[i] = '\0';

                /* Convert the Suboption number to binary */
                sub_opt_num = atoi(sub_opt);

                /* Add this suboption to the output string */
                out_str += sprintf(out_str, "%02X:%02X:%s", sub_opt_num, sub_opt_len, sub_opt_str);
            }
        }
    }

    /* Remove the final trailing ':' */
    *(out_str - 1) = '\0';

    return(TRUE);
}


int build_exe_string(int num_args, char *argv[], char *opt43, char *out_str)
{
    int i;

    strcpy(out_str, "udhcpc ");

    for (i = 1; i < num_args; i++) 
    {
        strcat(out_str, argv[i]);
        strcat(out_str, " ");
    }
    strcat(out_str, opt43);
}


int main(int argc, char *argv[])
{
    FILE *inp_file = NULL;
    unsigned char ucOpt43[LINE_LIMIT];
    unsigned char ucExeStr[LINE_LIMIT*2];
    int ret;
    
    /* First check for the existance of /opt/udhcpc.vendor_specific */
    inp_file = fopen(UDHCPC_OPT43_FILE, "r");
    if (inp_file == NULL)
    {
        printf("WARNING: %s does not exist.  Opt43 not used.\n", 
		UDHCPC_OPT43_FILE);

    }
    else
    {
        /* Generate the option 43 command line string */
        ret = create_opt43_string(inp_file, ucOpt43);
        fclose(inp_file);
        if (! ret)
        {
            printf("ERROR: Syntax error in udhcpc.vendor_specific\n");
            exit(10);
        }

        /* Generate the command line */
        build_exe_string(argc, argv, ucOpt43, ucExeStr);
        printf("\n\r%s\n\r", ucExeStr);

        /* Execute udhcpc with the generated options */
        system(ucExeStr);
    }
}
