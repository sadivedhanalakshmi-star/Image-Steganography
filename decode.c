#include <stdio.h>       // for input/output functions
#include <string.h>      // for string handling functions
#include "decode.h"      // for decode function declarations
#include "types.h"       // for enum and structure definitions

// Read and validate command-line arguments for decoding
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2], ".bmp") != NULL)   // check if stego file is a .bmp image
    {
        printf(".bmp file is present\n");
        decInfo->stego_image_fname = argv[2];   // store stego image file name
    }
    else
    {
        printf(".bmp file is not present\n");
        return e_failure;
    }

    if(argv[3] != NULL)   // if user provided output filename
    {
        decInfo->output_fname = argv[3];   // store output file name
        printf("Output file name is provided: %s\n", decInfo->output_fname);
    }
    else
    {
        decInfo->output_fname = "decoded.txt";   // default output filename
        printf("Default output file name: decoded.txt\n");
    }
    return e_success;
}

// Perform the full decoding process
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo) == e_success)    // open required files
    {
        printf("All decode files opened successfully\n");
    }
    else
    {
        printf("Error in opening files\n");
        return e_failure;
    }

    if(skip_bmp_header(decInfo->fptr_stego_image) == e_success)  // skip 54-byte BMP header
    {
        printf("Skipped BMP header bytes successfully\n");
    }
    else
    {
        printf("Failed to skip BMP header\n");
        return e_failure;
    }

    char magic_string[20];
    printf("Enter the magic string used during encoding: ");
    scanf("%s", magic_string);  // take magic string from user

    if (decode_magic_string(magic_string, decInfo) == e_success)  // decode & check magic string
    {
       printf("Magic string decoded successfully\n");
    }
    else
    {
       printf("Magic string decoding failed\n");
       return e_failure;
    }

    if(decode_secret_file_extn_size(decInfo) == e_success)  // decode extension size
    {
        printf("Secret file extension size decoded successfully\n");
    }
    else
    {
        printf("Secret file extension size decoding failed\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo) == e_success)  // decode extension characters
    {
        printf("Secret file extension decoded successfully\n");
    }
    else
    {
        printf("Secret file extension decoding failed\n");
        return e_failure;
    }

    if(decode_secret_file_size(decInfo) == e_success)  // decode secret file size
    {
        printf("Secret file size decoded successfully\n");
    }
    else
    {
        printf("Secret file size decoding failed\n");
        return e_failure;
    }

    if(decode_secret_file_data(decInfo) == e_success)  // decode secret file data
    {
        printf("Secret file data decoded successfully\n");
    }
    else
    {
        printf("Secret file data decoding failed\n");
        return e_failure;
    }

    fclose(decInfo->fptr_stego_image);   // close stego image file
    fclose(decInfo->fptr_output);        // close output file

    printf("Decoding completed successfully! Output written to %s\n", decInfo->output_fname);

    return e_success;
}

// Open stego image and output file
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");  // open stego image
    if(decInfo->fptr_stego_image == NULL)
    {
        printf("Error: Stego image file not found\n");
        return e_failure;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname, "w");  // open output file
    if(decInfo->fptr_output == NULL)
    {
        printf("Error: Unable to create output file\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

// Skip the 54-byte BMP header
Status skip_bmp_header(FILE *fptr_stego_image)
{
    fseek(fptr_stego_image, 54, SEEK_SET);   // move file pointer to pixel data
    return e_success;
}

// Decode 1 byte from 8 LSBs
Status decode_byte_from_lsb(char *data, unsigned char *image_buffer)
{
    unsigned char ch = 0;
    for(int i = 0; i < 8; i++)
    {
        ch = ch | ((image_buffer[i] & 1) << i);   // extract each bit
    }
    *data = ch;   // store decoded byte
    return e_success;
}

// Decode a 32-bit number from LSBs
Status decode_size_from_lsb(long int *size, unsigned char *image_buffer)
{
    *size = 0;
    for(int i = 0; i < 32; i++)
    {
        *size = *size | ((image_buffer[i] & 1) << i);   // extract 32 bits
    }
    return e_success;
}

// Decode and verify magic string
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    unsigned char image_buffer[8];
    char buffer[50];
    int len = strlen(magic_string);

    for (int i = 0; i < len; i++)    // decode each character
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&buffer[i], image_buffer);
    }

    buffer[len] = '\0';   // terminate decoded string

    unsigned char next_buffer[8];   // decode next character to confirm ending
    char next_char;
    fread(next_buffer, 1, 8, decInfo->fptr_stego_image);
    decode_byte_from_lsb(&next_char, next_buffer);

    if ((strcmp(buffer, magic_string) == 0) && (next_char == '\0'))  // full match
    {
        printf("Magic string fully matched\n");
        return e_success;
    }
    else
    {
        printf("Magic string is not matched\n");
        return e_failure;
    }
}

// Decode extension size (32 bits)
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    unsigned char image_buffer[32];
    fread(image_buffer, 1, 32, decInfo->fptr_stego_image);  // read 32 bytes
    decode_size_from_lsb(&decInfo->extn_size, image_buffer); // extract extension size
    return e_success;
}

// Decode extension characters
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    unsigned char image_buffer[8];
    for(int i = 0; i < decInfo->extn_size; i++)   // decode each extension char
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&decInfo->extn_secret_file[i], image_buffer);
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';   // null-terminate extension
    return e_success;
}

// Decode size of secret file (32 bits)
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    unsigned char image_buffer[32];
    fread(image_buffer, 1, 32, decInfo->fptr_stego_image);
    decode_size_from_lsb(&decInfo->size_secret_file, image_buffer);
    return e_success;
}

// Decode actual secret file data
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    unsigned char image_buffer[8];
    char ch;

    for(long int i = 0; i < decInfo->size_secret_file; i++)   // decode each byte
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, image_buffer);
        fputc(ch, decInfo->fptr_output);   // write to output file
    }
    return e_success;
}
