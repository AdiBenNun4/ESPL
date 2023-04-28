#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    FILE *outfile = stdout;
    FILE *infile = stdin;
    char c;
    int encoder_mode = 0; //-1 if negative, 1 if positive, 0 if not oprated yet
    int debug_mode = 0;   // debug mode 1-on , 0-off
    int key_position_argv;
    int position_in_key = 2;
    int key_length=0;
    //loop over argv
    for (int i = 1; i < argc; i++)
    { //loop start from 1 because 0 is file name
        if (strcmp(argv[i], "+D") == 0)
            debug_mode = 1;
        else if (strcmp(argv[i], "-D") == 0)
            debug_mode = 0;
        else if (strncmp(argv[i], "+e", 2) == 0 || strncmp(argv[i], "-e", 2) == 0) //found the encoder key
        {
            if (strncmp(argv[i], "+e", 2) == 0)
            {
                encoder_mode = 1;
            }
            else
            {
                encoder_mode = -1;
            }
            key_position_argv = i;

            char *temp=argv[i];
            while(*temp!='\0'){
                key_length++;
                temp++;
            }
        }
        else if (strncmp(argv[i], "-i", 2) == 0)
        {
            infile = fopen(argv[i] + 2, "r");
            if (infile == NULL)
                fprintf(stderr, "error");
        }
        else if (strncmp(argv[i], "-o", 2) == 0)
        {
            outfile = fopen(argv[i] + 2, "w");
            if (outfile == NULL)
                fprintf(stderr, "error");
        }

        if (debug_mode == 1 && strcmp(argv[i], "+D") != 0)
        {
            fprintf(stderr, "%s\n", argv[i]);
        }
    }

    //loop over stdin until EOF

    while ((c = fgetc(infile)) != EOF)
    {
        if (encoder_mode == 0)
        { //encoder mode was not activated
            fputc(c, outfile);
        }
        else
        { //encoder mode was activated
            if ('0' <= c && c <= '9')
            {
                c = (char)(c + encoder_mode * (argv[key_position_argv][position_in_key] - '0'));
                if (c > '9')
                {
                    c = (char)(c - 10);
                }
                else if (c < '0')
                {
                    c = (char)(c + 10);
                }
            }
            else if ('a' <= c && c <= 'z')
            {
                c = (char)(c + encoder_mode * (argv[key_position_argv][position_in_key] - '0'));
                if (c > 'z')
                {
                    c = (char)(c - 26);
                }
                else if (c < 'a')
                {
                    c = (char)(c + 26);
                }
            }
            else if ('A' <= c && c <= 'Z')
            {
                c = (char)(c + encoder_mode * (argv[key_position_argv][position_in_key] - '0'));
                if (c > 'Z')
                {
                    c = (char)(c - 26);
                }
                else if (c < 'A')
                {
                    c = (char)(c + 26);
                }
            }
            
            fputc(c, outfile);

            position_in_key = (position_in_key + 1) %key_length;
            if (position_in_key == 0)
            {
                position_in_key = 2;
            }
        }
    }

    return (0);
}