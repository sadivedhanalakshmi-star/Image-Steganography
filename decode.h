#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"  // Contains user-defined types

/*
 Structure to store information required for
 decoding secret data from the stego image */
typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char *output_fname;
    FILE *fptr_output;

    long int extn_size;
    char extn_secret_file[10];
    long int size_secret_file;
    char magic_string[100];
    int magic_len;
    
}DecodeInfo; 

/* Decoding function prototypes */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for input (stego image) and output files */
Status open_decode_files(DecodeInfo *decInfo);

Status skip_bmp_header(FILE *fptr_stego_image);

/* Decode magic string */
Status decode_magic_string(const char *magic_string , DecodeInfo *decInfo);

/* Decode the size of secret file extension */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode the secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode the secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode the secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte from LSB of image data */
Status decode_byte_from_lsb(char *data, unsigned char *image_buffer);

/* Decode an integer size from LSBs */
Status decode_size_from_lsb(long int *size, unsigned char *image_buffer);
#endif