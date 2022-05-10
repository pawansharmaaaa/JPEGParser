#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void checkHostName(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

int main (int argc, char **argv)
{   
    //Declarations
    
    FILE *file = fopen( argv[1], "rb");
    unsigned char buffer;
    
    long position;
    long possiblefooters[1000];
    long verifiedheaders[1000];
    long verifiedfooters[1000];
    long subverfooters[1000];
    long finalposition;
    
    int i = 0, j = 0, k;
    int switcher;

    //Finding out Headers and Footers.

    do {
        fread(&buffer, 1, 1, file);
        if (buffer == 0xFF && i != 1000){
            fread(&buffer,1,1,file);
            if (buffer == 0xD8){
                fread(&buffer, 1, 1, file);
                if (buffer == 0xFF){
                    fread(&buffer, 1, 1, file);
                    if (buffer == 0xE0){
                        fread(&buffer, 1, 1, file);
                        if (buffer == 0x00){
                            fread(&buffer, 1, 1, file);
                            if (buffer == 0x10){
                                finalposition = ftell(file);
                                printf("HEADER%d found at Position; %ld\n", i+1, finalposition-6);
                                verifiedheaders[i] = finalposition-6;
                                switcher = 1;
                                i++;
                            }
                        }
                    }              
                }
            }

            // Below statement contains a switcher variable to filter out unwanted footers, since footers that come immediately after a verified header is considered valid, switcher is implemented
            if (buffer == 0xD9 && switcher){
                printf("FOOTER%d found at Position: %ld\n", j+1, ftell(file)-2);
                verifiedfooters[j] = ftell(file)-2;
                j++;
                switcher=0;
            }
        };
    }while (!feof(file));
    fclose(file); //Closing the file.

    //Listing Start and End of Possible Jpeg Files.

    for (k=0; k<i; k++){
        printf("FILE%d : %ld to %ld\n", k, verifiedheaders[k], verifiedfooters[k]);
    }

    //CARVING OUT FILES

    // Declaration For hostname
   
    char hostbuffer[256];
    int hostname;

    // Retrieve Hostname

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);

    //Declarations for Extraction Part

    FILE *fileread = fopen(argv[1], "r"); //reopening the same file in simple read mode.
    FILE *filewrite;
    long pos, end;
    unsigned char wbuffer;
    int act, m;

    char filename_format[] = "%s_%d.jpg";
    char filename[sizeof(filename_format) + 3];  // for up to 4 digit numbers

    for (m = 0; m < 2; m++) {

        act = 1;
        pos = verifiedheaders[m];
        end = verifiedfooters[m];

        snprintf(filename, sizeof(filename), filename_format, hostbuffer,m); //for creating filename.
        
        filewrite = fopen(filename, "wb");
        rewind(fileread);
        fseek(fileread, pos, SEEK_SET);
        while (!feof(fileread) && act){
            fread(&wbuffer, 1, 1, fileread);
            fwrite(&wbuffer, 1, 1, filewrite);
            if(ftell(fileread)==end+1){
                act = 0;
            }
        }
        printf("%s successfully extracted\n", filename);
        fclose(filewrite);
    }

    printf("Extracted the files with Log-in-Name %s on host %s \n", getlogin(), hostbuffer);
    return 0;
}