/*
Name           : S Dhana Lakshmi
Language       : C
Project Name   : LSB Image Steganography
Technique Used : LSB (Least Significant Bit Substitution)
Files Used     :  → Input : BMP image 
                  → Secret: .txt file
                  → Output: stego.bmp (encoded image)
Description    : This project hides a secret text file inside a BMP image using  
                 Least Significant Bit (LSB) substitution. Only the last bit of each  
                 pixel is modified, so the visual appearance of the image remains  
                 unchanged to the human eye.
Key Features    : -> Magic string + magic length validation 
                  -> Secret file extension encoding
                  -> Secret file size encoding
                  -> Secret data encoding
                  -> Full decoding with error checks
*/


#include <stdio.h>         // For input/output functions
#include "encode.h"       // Header file for encoding operations
#include "decode.h"      // Header file for decoding operations 
#include "types.h"      // Header file containing enum definitions and constants
#include <string.h>    // For string handling functions

int main(int argc,char *argv[])
{
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("  Encoding: %s -e <.bmp file> <secret.txt> [output_stego.bmp]\n", argv[0]);
        printf("  Decoding: %s -d <stego.bmp> [output.txt]\n", argv[0]);
        return 1;
    }

    if(check_operation_type(argv) == e_encode) // check if the user selected "-e"
    {
        if (argc < 4)  // check if the user passed enough arguments for encoding
        {
            printf("Error: Not enough arguments for encoding.\n");
            printf("Usage: %s -e <.bmp file> <secret.txt> [output_stego.bmp]\n", argv[0]);
            return 1;
        }

        printf("You have choosed encoding\n");  // Inform user that encoding mode is selected
        EncodeInfo encInfo;  // Structure to store encoding info
        if(read_and_validate_encode_args(argv,&encInfo) == e_success)   // Validate command-line arguments for encoding
        {
            printf("Raed and validate is successfull\n");  // Inform user that encoding arguments are read and validated successfully
            if(do_encoding(&encInfo) == e_success)  //Perform encoding
            {
                printf("Encoding is successfull\n");  // Success message for encoding
            }
            else
            {
                printf("Encoding is un-successfull\n"); // Failure message for encoding
                return e_failure;  //Exit program with failure status
            }
        }
        else
        {
            printf("Raed and validate is un-successfull\n"); // Inform user that encoding arguments are not read and validated successfully
            return e_failure;  //Exit program
        }
    }
    else if(check_operation_type(argv) == e_decode)  // Check if the user selected "-d" 
    {
        if (argc < 3)  // check if the user passed enough arguments for decoding
        {
            printf("Error: Not enough arguments for decoding.\n");
            printf("Usage: %s -d <stego.bmp> [output.txt]\n", argv[0]);
            return 1;
        }
        
        printf("You have choosed decoding\n"); // Inform user that decoding mode is selected
        DecodeInfo decInfo;  //Structure to store decoding info

        if (read_and_validate_decode_args(argv, &decInfo) == e_success) // Validate command-line arguments for decoding
        {
            printf("Read and validate for decoding is successful\n"); // Inform user that decoding arguments are read and validated successfully

            if (do_decoding(&decInfo) == e_success)  //Perform decoding
            {
                printf("Decoding is successful\n");  // Success message for decoding
            }
            else
            {
                printf("Decoding failed\n");  // Failure message for decoding
                return e_failure;  //Exit program with failure status
            }
        }
        else
        {
            printf("Read and validate for decoding failed\n"); // Inform user that decoding arguments are not read and validated successfully
            return e_failure;  //Exit program
        } 
    }
    else  // If the user didn't provide enough arguments that time this block will executed
    {   
        printf("Pass correct arguments\n");
        printf("./a.out -e beautiful.bmp secret.txt --> for encoding\n");
        printf("./a.out -d stego.bmp decode.txt--> for decoding\n");
    }

    return e_success;  // Program executed successfully
}

OperationType check_operation_type(char *argv[])  // Function to check if user selected encode or decode
{
    if(strcmp(argv[1],"-e") == 0)   // compare input with "-e"
       return e_encode;             
    else if(strcmp(argv[1],"-d")==0)  // compare input with "-d"
       return e_decode;
    else                             // this is for invalid input
       return e_unsupported;
}