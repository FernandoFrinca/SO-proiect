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
#define BUFFER_PIPE 1024


//functie pentru verificarea unui caracter daca este alfanumeric
int isAlfanumeric(char c){
    if(c >= 'A' && c <= 'Z'){
        return 1;
    }
    else if(c >= 'a' && c <= 'z'){
        return 1;
    }
    else if(c >= '0' && c <= '9'){
        return 1;
    }
    else{
        return 0;
    }
}

//functie care converteste imagine ain tonuri de gri
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

    //se extrage inaltimea si latimea pentru a putea parcurge toata imaginea
    lseek(bmpFile, 18, SEEK_SET);
    read(bmpFile, &width, sizeof(width));
    
    lseek(bmpFile, 22, SEEK_SET);
    read(bmpFile, &height, sizeof(height));

    
    lseek(bmpFile, pixelDataOffset, SEEK_SET);

    //fiecare pixel este parcurs si modificat dupa formula data dupa care este scris inapoi 
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

//functie pentru a genera calea de output, extrage din text.txt doar text sa putem adauga _statistica.txt  
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


//functie pentru a afisa permisiile grup,owner,other fisierului, 
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


//functie pentru a extrage si printa in fisierul de output datele cerute despre directoare
void openFileDIR(char *DirName, char *dir_out){

    struct stat file_info;
    char buffer[BUFFER_SIZE];
    int scrie ;
    int flags_out = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char output_path[256];
    char aux_DirName[256];

    //GENERAREA PATH ULUI DE OUTPUT
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
//functie pentru a extrage si printa in fisierul de output datele cerute despre legaturile simbolice
void openFileLINK(char *LinkName, char *dir_out){
    struct stat file_info;
    char buffer[BUFFER_SIZE];
    int scrie;
    int flags_out = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char output_path[256];
    char aux_LinkName[256];

    //GENERAREA PATH ULUI DE OUTPUT
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
//functie pentru a extrage si printa in fisierul de output datele cerute despre fisiere, 
// daca sunt BMP se executa un anumit set de instructiunin intr-un proces copil
// daca sunt fisiere normale se executa alt set de instrunctiuni intr-un proces copil
void openFile(char *fileName, char *dir_out, char *dir_in, char caracter) {
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
    char gray_path[300];
    char script_path[300];

    pid_t pid_BMP;
    int fd[2]; // pentru capetele pipe-ului
    
    strcpy(aux_fileName,fileName);
    sprintf(gray_path, "%s/%s", dir_in, aux_fileName);
    sprintf(script_path, "%s/%s", dir_in, "script.sh");
    sprintf(output_path, "%s/%s_statistica", dir_out, createNameOutputFile(aux_fileName));
    
    int file_out = open(output_path,flags_out, mode_out);
    if (file_out < 0) {
        printf("Eroare la deschiderea fisierului statistica.txt");
        close(file_out);
        exit(-1);
    } 

    // Verifică dacă fișierul este o imagine BMP
    if (strcmp(fileName + strlen(fileName) - 4, ".bmp") == 0){
        type = 1;
        pid_BMP = fork();
        if (pid_BMP == -1) {
            printf("fork error");
            exit(-1);
        }

        if (pid_BMP == 0) { 
            convertToGrayscale(gray_path);
            exit(0);
        }
        int status;
        waitpid(pid_BMP, &status, 0);
    }
        // Deschidem pipe ul pentru fisiere NON-BMP
        if (pipe(fd) == -1) {
            perror("pipe error");
            exit(EXIT_FAILURE);
        }

        // Proces fiu pentru scrierea statisticilor atat BMP cat si fisiere normale
        pid_t pid_stat = fork();
        if (pid_stat == 0) {
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
            exit(0);
        }

    if(type != 1)
    {
        // Proces fiu pentru calculul propozitiilor corecte
        pid_t pid_count = fork();
        if (pid_count == 0) {
            close(fd[1]); // inchidem capatul de write
            dup2(fd[0], STDIN_FILENO);// redirectionare iesirea standard standard din pipe
            close(fd[0]);// inchidem capatul de read
            char c_str[2] = {caracter, '\0'}; // aici face ca caracterul citit d ela tastatura sa fie de tip string adaugand un terminator de sir
            execlp("bash", "bash", "script.sh", c_str, NULL); // rulam scriptul 
            printf("execlp failed");
            exit(-1);
        }

        int status;
        waitpid(pid_stat, &status, 0);

        close(fd[0]); //inchidem capatul de citire al pipe-ului
        int file_in = open(fileName, O_RDONLY);
        char read_buf[BUFFER_PIPE];
        ssize_t bytes_read;
        while ((bytes_read = read(file_in, read_buf, sizeof(read_buf))) > 0) {
            write(fd[1], read_buf, bytes_read);
        }
        close(file_in);
        close(fd[1]);

        waitpid(pid_count, &status, 0);
        if (WIFEXITED(status)) {
            int num_prop = WEXITSTATUS(status);
            printf("Au fost identificate in total %d propozitii corecte care contin caracterul %c\n", num_prop, caracter);
        }
    }

        close(open_file_in);
        close(file_out);
}

//functie pentru gestionarea tuturor tipurilor de fisiere/dir din directorul dat ca argument
void getFiles(char *director_in, char *director_out, char caracter) {
    int nr_linii = 0;
    DIR *dir;
    struct dirent *dir_index;
    pid_t pid;
    int status;

    dir = opendir(director_in);
    if (dir == NULL) {
        printf("Eroare la deschiderea directorului");
        exit(-1);
    }

    while ((dir_index = readdir(dir)) != NULL) {
        if (strcmp(dir_index->d_name, ".") != 0 && strcmp(dir_index->d_name, "..") != 0) {
            pid = fork();// cate un process fiu pentru prelucrarea fiecarei intrari
            if (pid == -1) {
                printf("fork error");
                continue; 
            }

            if (pid == 0) { 
                if (dir_index->d_type == DT_DIR) {
                    nr_linii = 5; //setare nr linii pt afisare
                    //printf("\nintraDIR\n");
                    openFileDIR(dir_index->d_name, director_out);
                } else if (dir_index->d_type == DT_LNK) {
                    nr_linii = 8;
                    openFileLINK(dir_index->d_name, director_out);
                } else if (dir_index->d_type == DT_REG) {
                   if(strstr(dir_index->d_name, ".bmp") != NULL)
                        {
                            //printf("\nintraBMP\n");
                            nr_linii = 0;
                        }
                        else{
                            //printf("\nintraFILE\n");
                            nr_linii = 8;
                        }
                    openFile(dir_index->d_name, director_out, director_in, caracter);
                }  
                exit(nr_linii); 
            } else {
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    int nr_linii = WEXITSTATUS(status); // pentru fiecare proces se extrage nr de linii sris
                    printf("S-a încheiat procesul cu pid-ul %d și nr linii scrise: %d\n", pid, nr_linii);
                }
            }
        }  
    } 
    closedir(dir);
}


int main(int argc, char *argv[]) {

    char *director_in = argv[1];
    char *director_out = argv[2];
    char caracter;

     if (argc != 4) {
        printf("Eroare la argumente\n");
        exit(-1);
     }

    if(strlen(argv[3])>1){
        printf("Trebuia introdus doar un singur caracter <alfanumeric>\n");
        exit(-1);
    }
    else{
        caracter = argv[3][0];
        if(isAlfanumeric(caracter) != 1){
            printf("Trebuia introdus un caracter alfanumeric\n");
            exit(-1);
        }
       
    }

    getFiles(director_in,director_out,caracter);
       

    return 0;
}

