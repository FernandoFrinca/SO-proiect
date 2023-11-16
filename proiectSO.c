#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include<string.h>
/*
void printbinary(int nr){
    while (nr>0)
    {
        printf("%d",nr%2);
        nr = nr / 2;
    }
    
}
*/

char *permisie(mode_t permis) {
    char *result = (char *)malloc(10); 
    if (result == NULL) {
        perror("eroare malloc");
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

void openFileDIR(char *pathname, int file_out){
    struct stat file_info;
    char buffer[256];
    int scrie ;
    scrie = sprintf(buffer, "nume director: %s\n", pathname);

    char modification_time_str[50];
    struct tm *modification_tm = localtime(&file_info.st_mtim.tv_sec);
    strftime(modification_time_str, sizeof(modification_time_str), "%d.%m.%Y", modification_tm);

    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\n", file_info.st_uid);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\n drepturi de accesgrup: %.3s\n drepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
}

void openFileLINK(char *pathname, int file_out){
    struct stat file_info;
    char buffer[256];
    int scrie ;

    if (lstat(pathname, &file_info) == -1) {
        perror("Eroare obtinere informatii despre fisier");
        exit(-1);
    }

    scrie = sprintf(buffer, "nume legatura: %s\n", pathname);
    char modification_time_str[50];
    struct tm *modification_tm = localtime(&file_info.st_mtim.tv_sec);
    strftime(modification_time_str, sizeof(modification_time_str), "%d.%m.%Y", modification_tm);

    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "dimensiune legatura: %ld\n", file_info.st_size);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\n drepturi de accesgrup: %.3s\n drepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
}

void openFile(char *pathname, int file_out){
    
    int flags_in = O_RDONLY;
    mode_t mode_in = S_IRUSR | S_IRGRP | S_IROTH; 
    int open_file_in = open(pathname, flags_in, mode_in);
    struct stat file_info;
    int height; 
    int width;
    int size;
    int type = 0;
    char buffer[256];
    int scrie ;

    if (strcmp(pathname+ strlen(pathname) - 4, ".bmp") == 0) {
        type = 1;
    } 

    if (open_file_in < 0) {
        perror("Eroare la deschiderea fisierului");
        close(open_file_in);
        exit(-1);
    }

    if (lseek(open_file_in, 2, SEEK_SET) < 0) {
        perror("Eroare cursor dimensiune \n");
    }

    if (read(open_file_in, &size, 4) < 0) {
        perror("Eroare dimensiune");
    }

    if (lseek(open_file_in, 18, SEEK_SET) < 0) {
        perror("Eroare cursor latime \n");
    }

    if ((read(open_file_in, &width, 4) < 0) && type==1) {
        perror("Eroare latime");
    }

    if (lseek(open_file_in, 22, SEEK_SET) < 0) {
        perror("Eroare cursor inaltime \n");
    }

    if ((read(open_file_in, &height, 4) < 0) && type==1) {
        perror("Eroare inalime");
    }

    if (fstat(open_file_in, &file_info) == -1) {
        perror("Eroare obtinere informatii despre fisier");
        close(open_file_in);
        exit(-1);
    }
    
    char modification_time_str[50];
    struct tm *modification_tm = localtime(&file_info.st_mtim.tv_sec);
    strftime(modification_time_str, sizeof(modification_time_str), "%d.%m.%Y", modification_tm);

    if(type == 1){
        scrie = sprintf(buffer, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %d\n", pathname, height, width, size);
        if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing the name on the output file");
        exit(-1);
        }
    }
    else{
        scrie = sprintf(buffer, "nume legatura: %s\ndimensiune: %d\n", pathname, size);
        if (write(file_out, buffer, scrie) == -1) {
            perror("Error at writing the name on the output file");
            exit(-1);
        }
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\n", file_info.st_uid, modification_time_str,file_info.st_nlink);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %.3s\n drepturi de accesgrup: %.3s\n drepturi de acces altii: %.3s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) << 3),permisie((file_info.st_mode & S_IRWXO)<<6));
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }

}

void getFiles(char *director, int file_out){
    DIR *dir;
    struct dirent *dir_index;
    dir = opendir(director);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        exit(-1);
    }
    while ((dir_index = readdir(dir)) != NULL) {
        if (strcmp(dir_index->d_name, ".") != 0 && strcmp(dir_index->d_name, "..") != 0) {
            if (dir_index->d_type == DT_DIR) {
                openFileDIR(dir_index->d_name,file_out);
            } 
            if (dir_index->d_type == DT_LNK) {
                openFileLINK(dir_index->d_name,file_out);
            }
           if (dir_index->d_type == DT_REG) {
                openFile(dir_index->d_name,file_out);
            }  
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {

    char *director = argv[1];
    int flags_out = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int file_out = open("statistica.txt",flags_out, mode_out);

    if (argc != 2) {
        printf("Usage ./%s <fisier_intrare>\n", argv[0]);
        exit(-1);
    }
    if (file_out < 0) {
        perror("Eroare la deschiderea fisierului statistica.txt");
        close(file_out);
        exit(-1);
    } 

    getFiles(director,file_out);
    

    if (close(file_out) < 0) {
        perror("Eroare la inchidere");
        exit(-1);
    }

    return 0;
}