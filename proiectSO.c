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

void convertToGrayscale(char *bmpPath,char *outputPath) 
{
    int bmpFile = 0, bmpFileOut = 0;
    int flags_out = O_RDWR | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int width = 0, height = 0, pixelDataOffset = 0;
    char copia[54];

    bmpFile = open(bmpPath, O_RDONLY);
    if (bmpFile == -1) {
        printf("eroare la deschidere bmp");
        exit(-1);
    }

    bmpFileOut = open(outputPath, flags_out, mode_out);
    if (bmpFileOut == -1) {
        printf("eroare la deschidere bmp_out");
        exit(-1);
    }

    read(bmpFile, copia, sizeof(copia));

    memcpy(&width, &copia[18], sizeof(width));
    memcpy(&height, &copia[22], sizeof(height));
    memcpy(&pixelDataOffset, &copia[10], sizeof(pixelDataOffset));

    write(bmpFileOut, copia, sizeof(copia));

    lseek(bmpFile, pixelDataOffset, SEEK_SET);
    lseek(bmpFileOut, pixelDataOffset, SEEK_SET);

    for (int i = 0; i < height * width; i++) {
         char pixel[3];
         char gray;

        read(bmpFile, pixel, sizeof(pixel));
        gray = (char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
        pixel[0] = pixel[1] = pixel[2] = gray;

        write(bmpFileOut, pixel, sizeof(pixel)); 
    }

    if (close(bmpFile) == -1) {
        printf("eroare la inchidere bmp");
        exit(-1);
    }
    if (close(bmpFileOut) == -1) {
        printf("eroare la deschidere bmp");
        exit(-1);
    }
}
char* createNameOutputFile(char *string){
    char delimiters[] = ".";
    char *p = strtok(string, delimiters);
    return p;
}


char *permisie(mode_t permis) {
    char *result = (char *)malloc(10); 
    if (result == NULL) {
        printf("eroare malloc");
        exit(-1);
    }

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

    scrie = sprintf(buffer, "nume director: %s\n", DirName);

    char modification_time_str[50];
    struct tm *modification_tm = localtime(&file_info.st_mtim.tv_sec);
    strftime(modification_time_str, sizeof(modification_time_str), "%d.%m.%Y", modification_tm);

    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\n", file_info.st_uid);
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }

    scrie = sprintf(buffer,"drepturi de acces user: %.3s\n drepturi de accesgrup: %.3s\n drepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }

    if(close(file_out)<0){
        exit(0);
    }
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

    char modification_time_str[50];
    struct tm *modification_tm = localtime(&file_info.st_mtim.tv_sec);
     strftime(modification_time_str, sizeof(modification_time_str), "%d.%m.%Y", modification_tm);

    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "dimensiune legatura: %ld\n", file_info.st_size);
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\n drepturi de accesgrup: %.3s\n drepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }
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
    char modification_time_str[50];
    char output_path[256];
    char aux_fileName[256];
    char gray_path[512];
    char gray_path_out[512];
    struct tm *modification_tm = localtime(&file_info.st_mtim.tv_sec);

    strftime(modification_time_str, sizeof(modification_time_str), "%d.%m.%Y", modification_tm);


    strcpy(aux_fileName,fileName);
    sprintf(gray_path, "%s/%s", dir_in, aux_fileName);
    sprintf(gray_path_out, "%s/%s", dir_out, aux_fileName);
    sprintf(output_path, "%s/%s_statistica", dir_out, createNameOutputFile(aux_fileName));
    
    int file_out = open(output_path,flags_out, mode_out);
    if (file_out < 0) {
        printf("Eroare la deschiderea fisierului statistica.txt");
        close(file_out);
        exit(-1);
    } 

    if (strcmp(fileName+ strlen(fileName) - 4, ".bmp") == 0) {
         int pipefd[2];
        if (pipe(pipefd) == -1) { 
            printf("pipe failed");
            exit(EXIT_FAILURE);
        }
        type = 1;
        pid_t pid = fork();
        if (pid == -1) {
            printf("Eroare la fork");
            exit(-1);
        }
        if (pid == 0) {
            close(pipefd[0]); 
            convertToGrayscale(gray_path, gray_path_out);
            int linii = 0;
            write(pipefd[1], &linii, sizeof(linii)); 
            close(pipefd[1]);
            close(file_out);
            exit(0);
        }else{
            close(pipefd[1]); 

            int linii;
            read(pipefd[0], &linii, sizeof(linii)); 
            close(pipefd[0]);

            int status;
            waitpid(pid, &status, 0); 

            if (WIFEXITED(status)) {
                printf("S-a incheiat procesul cu pid-ul %d si codul %d. Numărul de linii scrise: %d\n", pid, WEXITSTATUS(status), linii);
        }
        }
        wait(NULL);
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
        scrie = sprintf(buffer, "nume legatura: %s\ndimensiune: %d\n", fileName, size);
        if (write(file_out, buffer, scrie) == -1) {
            printf("Error at writing the name on the output file");
            exit(-1);
        }
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\n", file_info.st_uid, modification_time_str,file_info.st_nlink);
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\n drepturi de accesgrup: %.3s\n drepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        printf("Error at writing to the output file");
        exit(-1);
    }

}

void getFiles(char *director_in, char *director_out){
    DIR *dir;
    struct dirent *dir_index;
    dir = opendir(director_in);
    if (dir == NULL) {
        printf("Eroare la deschiderea directorului");
        exit(-1);
    }
    while ((dir_index = readdir(dir)) != NULL) {
        if (strcmp(dir_index->d_name, ".") != 0 && strcmp(dir_index->d_name, "..") != 0) {
            pid_t pid = fork();
            if (pid == -1) {
                printf("eroare fork");
                exit(-1);
            } else if (pid == 0) { 
                if (dir_index->d_type == DT_DIR) {
                    openFileDIR(dir_index->d_name,director_out);
                } 
                if (dir_index->d_type == DT_LNK) {
                    openFileLINK(dir_index->d_name,director_out);
                }
                if (dir_index->d_type == DT_REG) {
                    openFile(dir_index->d_name,director_out,director_in);
                }  
            exit(0);
            }
        wait(NULL); 
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