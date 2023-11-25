#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define BMP_HEADER_SIZE 54

void convertToGrayscale(char *bmpPath) 
{
    int bmpFile = 0;
    int width = 0, height = 0, pixelDataOffset = 0, i = 0;

    if ((bmpFile = open(bmpPath, O_RDWR)) == -1) {
        printf("Error BMP gray");
        exit(-1);
    }

    lseek(bmpFile, 10, SEEK_SET);
    read(bmpFile, &pixelDataOffset, sizeof(pixelDataOffset));

    lseek(bmpFile, 18, SEEK_SET);
    read(bmpFile, &width, sizeof(width));
    
    lseek(bmpFile, 22, SEEK_SET);
    read(bmpFile, &height, sizeof(height));

    
    lseek(bmpFile, pixelDataOffset, SEEK_SET);

  
    for (i = 0; i < height * width; i++) 
    {
            unsigned char pixel[3];
            unsigned char gray = 0;

            read(bmpFile, pixel, sizeof(pixel));
            gray = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
            pixel[0] = pixel[1] = pixel[2] = gray;
            lseek(bmpFile, -3, SEEK_CUR); 
            write(bmpFile, pixel, sizeof(pixel)); 
    }

    if(close(bmpFile) == -1)
    {
        printf("Error closing bmp file\n");
        exit(-1);
    }
}
char* createNameOutputFile(const char *string) {
    static char outputFileName[256];
    strncpy(outputFileName, string, sizeof(outputFileName) - 1);
    outputFileName[sizeof(outputFileName) - 1] = '\0';

    char *p = strtok(outputFileName, ".");
    if (p == NULL) {
        return outputFileName; 
    }
    return p;
}

char *permisie(mode_t permis) {
    static char result[10];

    if (permis & S_IRUSR) result[0] = 'R';else result[0] = '-';
    if (permis & S_IWUSR) result[1] = 'W';else result[1] = '-';
    if (permis & S_IXUSR) result[2] = 'X';else result[2] = '-';
    if (permis & S_IRGRP) result[3] = 'R';else result[3] = '-';
    if (permis & S_IWGRP) result[4] = 'W';else result[4] = '-';
    if (permis & S_IXGRP) result[5] = 'X';else result[5] = '-';
    if (permis & S_IROTH) result[6] = 'R';else result[6] = '-';
    if (permis & S_IWOTH) result[7] = 'W';else result[7] = '-';
    if (permis & S_IXOTH) result[8] = 'X';else result[8] = '-';

    result[9] = '\0';  
    return result;
}

void openFileDIR(char *DirName, char *dir_out){

    struct stat file_info;
    char buffer[BUFFER_SIZE];
    int scrie ;
    int flags_out = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char output_path[256];
    char aux_DirName[256];

    strcpy(aux_DirName,DirName);
    sprintf(output_path, "%s/%s_statistica", dir_out, createNameOutputFile(aux_DirName));
    
    int file_out= open(output_path,flags_out, mode_out);
    
    if (file_out < 0) {
        printf("Eroare la deschiderea fisierului statistica.txt");
        close(file_out);
        exit(-1);
    } 

    if (lstat(DirName, &file_info) == -1) {
        printf("Eroare obtinere informatii despre fisier");
        exit(-1);
    }

    scrie = sprintf(buffer, "nume director: %s\n", DirName);
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\n", file_info.st_uid);
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }

    
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\ndrepturi de accesgrup: %.3s\ndrepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }

    if(close(file_out)<0){
        exit(0);
    }
    close(file_out);
}

void openFileLINK(char *LinkName, char *dir_out){
    struct stat file_info;
    char buffer[BUFFER_SIZE];
    int scrie;
    int flags_out = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char output_path[256];
    char aux_LinkName[256];

    strcpy(aux_LinkName,LinkName);
    sprintf(output_path, "%s/%s_statistica", dir_out, createNameOutputFile(aux_LinkName));
    
    int file_out = open(output_path,flags_out, mode_out);
    if (file_out < 0) {
        printf("Eroare la deschiderea fisierului statistica.txt");
        close(file_out);
        exit(-1);
    } 

    if (lstat(LinkName, &file_info) == -1) {
        printf("Eroare obtinere informatii despre fisier");
        exit(-1);
    }

    scrie = sprintf(buffer, "nume legatura: %s\n", LinkName);

    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "dimensiune legatura: %ld\n", file_info.st_size);
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\ndrepturi de accesgrup: %.3s\ndrepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }
    
    close(file_out);
}

void openFile(char *fileName, char *dir_out, char *dir_in){
    
    int flags_in = O_RDONLY;
    mode_t mode_in = S_IRUSR | S_IRGRP | S_IROTH; 
    int open_file_in = open(fileName, flags_in, mode_in);
    int flags_out = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    
    struct stat file_info;
    int height, width, size, type =0;
    char buffer[256];
    int scrie;

    char output_path[256];
    char aux_fileName[256];
    char gray_path[512];
    
    strcpy(aux_fileName,fileName);
    sprintf(gray_path, "%s/%s", dir_in, aux_fileName);
    sprintf(output_path, "%s/%s_statistica", dir_out, createNameOutputFile(aux_fileName));
    
    int file_out = open(output_path,flags_out, mode_out);
    if (file_out < 0) {
        printf("Eroare la deschiderea fisierului statistica.txt");
        close(file_out);
        exit(-1);
    } 

    if (strcmp(fileName+ strlen(fileName) - 4, ".bmp") == 0) {
        type = 1;
            pid_t pid = fork();
            if (pid == -1) {
                printf("fork error");
            }

            if (pid == 0) { 
                convertToGrayscale(gray_path);
                exit(0); 
            }
    } 

    if (open_file_in < 0) {
        printf("Eroare la deschiderea fisierului");
        close(open_file_in);
        exit(-1);
    }

    if (lseek(open_file_in, 2, SEEK_SET) < 0) {
        printf("Eroare cursor dimensiune \n");
    }

    if (read(open_file_in, &size, 4) < 0) {
        printf("Eroare dimensiune");
    }

    if (lseek(open_file_in, 18, SEEK_SET) < 0) {
        printf("Eroare cursor latime \n");
    }

    if ((read(open_file_in, &width, 4) < 0) && type==1) {
        printf("Eroare latime");
    }

    if (lseek(open_file_in, 22, SEEK_SET) < 0) {
        printf("Eroare cursor inaltime \n");
    }

    if ((read(open_file_in, &height, 4) < 0) && type==1) {
        printf("Eroare inalime");
    }

    if (fstat(open_file_in, &file_info) == -1) {
        printf("Eroare obtinere informatii despre fisier");
        close(open_file_in);
        exit(-1);
    }
    
    if(type == 1){
        scrie = sprintf(buffer, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %d\n", fileName, height, width, size);
        if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing the name on the output file");
        exit(-1);
        }
    }
    else{
        scrie = sprintf(buffer, "nume fisier: %s\ndimensiune: %d\n", fileName, size);
        if (write(file_out, buffer, scrie) == -1) {
            printf("Error at writing the name on the output file");
            exit(-1);
        }
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %ld\n", file_info.st_uid, ctime(&file_info.st_mtime),file_info.st_nlink);
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\ndrepturi de accesgrup: %.3s\ndrepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }
    close(open_file_in);
    close(file_out);
}

void getFiles(char *director_in, char *director_out){
    DIR *dir;
    struct dirent *dir_index;
    int nr_linii;
    pid_t pid ;
    dir = opendir(director_in);
    if (dir == NULL) {
        printf("Eroare la deschiderea directorului");
        exit(-1);
    }
    while ((dir_index = readdir(dir)) != NULL) {
      
            if (strcmp(dir_index->d_name, ".") != 0 && strcmp(dir_index->d_name, "..") != 0) {
                pid = fork();
                if (pid == -1) {
                    printf("fork error");
                    continue;
                }

                if (pid == 0) { 
                    if (dir_index->d_type == DT_DIR) {
                    nr_linii = 5;
                    //printf("sg test: ajunge1\n");
                    openFileDIR(dir_index->d_name,director_out);
                    } 
                    if (dir_index->d_type == DT_LNK) {
                        nr_linii = 8;
                        //printf("sg test: ajunge1\n");
                        openFileLINK(dir_index->d_name,director_out);
                    }
                    if (dir_index->d_type == DT_REG) {
                        if(strstr(dir_index->d_name, ".bmp") != NULL)
                        {
                            nr_linii = 10;
                        }
                        else{
                            nr_linii = 8;
                        }
                        //printf("sg test: ajunge1\n");
                        openFile(dir_index->d_name,director_out,director_in);
                    }  
                    exit(nr_linii);
                } 
            }  
    }
    int status;
    while ((pid = wait(&status)) > 0) {
        if (WIFEXITED(status)) {
            int line_count = WEXITSTATUS(status);
            printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", pid, line_count);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {

    char *director_in = argv[1];
    char *director_out = argv[2];

    if (argc != 3) {
        printf("Eroare la argumente");
        exit(-1);
    }

    getFiles(director_in,director_out);
    
    return 0;
}