#include <stdio.h>      // for standard input/output
#include <string.h>     // for string functions
#include "types.h"      // for user-defined types
#include "encode.h"     // for encoding function declarations

// to read and validate command-line arguments for encoding
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp") != NULL)       // check if source image has .bmp extension
    {
        printf(".bmp is present\n");
        encInfo->src_image_fname = argv[2];    // save source image name
    }
    else
    {
        printf(".bmp is not present\n");
        return e_failure;
    }

    if (strstr(argv[3], ".txt") != NULL)       // check if secret file has .txt extension
    {
        printf(".txt is present\n");
        encInfo->secret_fname = argv[3];       // save secret file name
    }
    else
    {
        printf(".txt is not present\n");
        return e_failure;
    }

    if (argv[4] != NULL && strstr(argv[4], ".bmp") != NULL)   // check if output file is .bmp
    {
        printf(".stego.bmp is present\n");
        encInfo->stego_image_fname = argv[4];   // store output stego file name
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";   // use default output file name
    }

    return e_success;
}

// do encoding function call
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success)      // open input/output files
        printf("All the files are opened successfully\n");
    else
    {
        printf("Files are not opened\n");
        return e_failure;
    }

    if (check_capacity(encInfo) == e_success)  // verify image can store secret data
        printf("Check capacity is successful\n");
    else
    {
        printf("Check capacity is unsuccessful\n");
        return e_failure;
    }

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success) // copy 54-byte BMP header
        printf("Header copied successfully\n");
    else
    {
        printf("Header is not copied successfully\n");
        return e_failure;
    }

    if (encode_magic_string(encInfo) == e_success)   // hide magic string into image
        printf("Magic string encoded successfully\n");
    else
    {
        printf("Magic string is not encoded successfully\n");
        return e_failure;
    }

    int size = strlen(strchr(encInfo->secret_fname, '.'));  // get extension length including dot

    if (encode_size_to_lsb(size, encInfo) == e_success)   // encode extension size
        printf("Size of extension encoded successfully\n");
    else
    {
        printf("Size of extension not encoded successfully\n");
        return e_failure;
    }

    if (encode_secret_file_extn(strchr(encInfo->secret_fname, '.'), encInfo) == e_success) // encode extension text
        printf("Secret file extension encoded successfully\n");
    else
    {
        printf("Secret file extension not encoded successfully\n");
        return e_failure;
    }

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success) // encode secret file size
        printf("Secret file size encoded successfully\n");
    else
    {
        printf("Secret file size not encoded successfully\n");
        return e_failure;
    }

    if (encode_secret_file_data(encInfo) == e_success)   // encode secret file content
        printf("Secret file data encoded successfully\n");
    else
    {
        printf("Secret file data not encoded successfully\n");
        return e_failure;
    }

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success) // copy remaining image bytes
        printf("Remaining data copied\n");
    else
    {
        printf("Remaining data not copied\n");
        return e_failure;
    }

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");     // open source .bmp file
    if (encInfo->fptr_src_image == NULL)
    {
        printf("Source file is not present\n");
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");           // open secret .txt file
    if (encInfo->fptr_secret == NULL)
    {
        printf("Secret file is not present\n");
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w"); // open output stego .bmp

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    printf("Enter magic string length: ");
scanf("%d", &encInfo->magic_len);

while (encInfo->magic_len <= 0 || encInfo->magic_len >= 100)
{
    printf("Invalid length! Enter a value between 1–99: ");
    scanf("%d", &encInfo->magic_len);
}

printf("Magic string length = %d\n", encInfo->magic_len);

// now take magic string input
while (1)
{
    printf("Enter magic string of exactly %d characters: ", encInfo->magic_len);
    scanf("%s", encInfo->magic);

    if (strlen(encInfo->magic) == encInfo->magic_len)
        break;

    printf("Error! Length mismatch. Expected %d characters.\n", encInfo->magic_len);
}

    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);  // total bytes available
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);           // size of secret file

    printf("Enter the magic string: ");
    scanf("%[^\n]", encInfo->magic);                                           // read magic string

    if (encInfo->image_capacity > (strlen(encInfo->magic)*8 + 32 + 32 + 32 + encInfo->size_secret_file*8))
        return e_success;
    else
    {
        printf("Image capacity failed\n");
        return e_failure;
    }
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);                   // return size of file
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, 4, 1, fptr_image);      // read BMP width
    fread(&height, 4, 1, fptr_image);     // read BMP height

    return width * height * 3;            // total bytes in pixel data
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    char buffer[54];
    fread(buffer, 54, 1, fptr_src_image);     // read 54-byte header
    fwrite(buffer, 54, 1, fptr_dest_image);   // write 54-byte header
    return e_success;
}

Status encode_magic_string(EncodeInfo *encInfo)
{
    if (encode_data_to_image(encInfo->magic, strlen(encInfo->magic) + 1, encInfo) == e_success)  // encode magic string + '\0'
        return e_success;
    else
        return e_failure;
}

Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo)
{
    for (int i = 0; i < size; i++)
    {
        fread(encInfo->image_data, 8, 1, encInfo->fptr_src_image);         // read 8 bytes from image
        encode_byte_to_lsb(data[i], encInfo->image_data);                  // hide 1 character into 8 bytes
        fwrite(encInfo->image_data, 8, 1, encInfo->fptr_stego_image);      // write encoded bytes
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> i) & 1);    // store each bit in LSB

    return e_success;
}

Status encode_size_to_lsb(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);                         // read 32 bytes

    for (int i = 0; i < 32; i++)
        buffer[i] = (buffer[i] & 0xFE) | ((size & (1 << i)) >> i);         // encode size bit-by-bit

    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);                      // write encoded bytes
    return e_success;
}

Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    if (encode_data_to_image(file_extn, strlen(file_extn), encInfo) == e_success)   // encode extension
        return e_success;
    else
        return e_failure;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(file_size, encInfo);      // encode secret file size
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[encInfo->size_secret_file];
    rewind(encInfo->fptr_secret);                // rewind secret file

    fread(buffer, encInfo->size_secret_file, 1, encInfo->fptr_secret);  // read full secret file
    encode_data_to_image(buffer, encInfo->size_secret_file, encInfo);   // encode secret data

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src))           // read remaining bytes
        fwrite(&ch, 1, 1, fptr_dest);            // write remaining bytes

    return e_success;
}