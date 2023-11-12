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


void afisare(char *nume, int inaltime, int latime,struct stat file_info, int dimensiune, char *modificare)
{
    printf("Nume imagine: %s \n", nume);
    printf("inaltime: %d\nlatime: %d\ndimensiune: %d\n", inaltime, latime, dimensiune);
    printf("identificatorul utilizatorului: %ld\n", (long)file_info.st_uid);
    printf("timpul ultimei modificari: %s\n", modificare);
    printf("contorul de legaturi: %ld\n", (long)file_info.st_nlink);
    printf("drepturi de acces user: %s\n",permisie(file_info.st_mode & S_IRWXU));
    printf("drepturi de acces grup: %s\n",permisie((file_info.st_mode & S_IRWXG) >> 3));
    printf("drepturi de acces altii: %s\n",permisie(file_info.st_mode & S_IRWXO));
    //printbinary((file_info.st_mode & S_IRWXO));
}
void scrieFisierBMP(int file_out, char *nume, int inaltime, int latime, int dimensiune, struct stat file_info,char *modificare) {

    char buffer[256];
    int scrie ;
    scrie = sprintf(buffer, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %d\n", nume, inaltime, latime, dimensiune);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\n", file_info.st_uid, modificare,file_info.st_nlink);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %s\n drepturi de accesgrup: %s\n drepturi de acces altii: %s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) >> 3),permisie(file_info.st_mode & S_IRWXO));
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }

}

void scrieFisierNormal(int file_out, char *nume, int dimensiune, struct stat file_info,char *modificare) {

    char buffer[256];
    int scrie ;
    scrie = sprintf(buffer, "nume legatura: %s\ndimensiune: %d\n", nume, dimensiune);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\n", file_info.st_uid, modificare,file_info.st_nlink);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %s\n drepturi de accesgrup: %s\n drepturi de acces altii: %s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) >> 3),permisie(file_info.st_mode & S_IRWXO));
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }

}

void scrieDirector(int file_out, char *nume,struct stat file_info) {

    char buffer[256];
    int scrie ;
    scrie = sprintf(buffer, "nume director: %s\n", nume);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing the name on the output file");
        exit(-1);
    }

    scrie = sprintf(buffer, "identificatorul utilizatorului: %d\n", file_info.st_uid);
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }
    scrie = sprintf(buffer,"drepturi de acces user: %s\n drepturi de accesgrup: %s\n drepturi de acces altii: %s\n\n",permisie(file_info.st_mode & S_IRWXU),permisie((file_info.st_mode & S_IRWXG) >> 3),permisie(file_info.st_mode & S_IRWXO));
    if (write(file_out, buffer, scrie) == -1) {
        perror("Error at writing to the output file");
        exit(-1);
    }

}

void openFile(char *pathname, int open_file_out,int type){
    int flags_in = O_RDONLY;
    mode_t mode_in = S_IRUSR | S_IRGRP | S_IROTH; 
    int open_file_in = open(pathname, flags_in, mode_in);
    struct stat file_info;
    int height; 
    int width;
    int size;

    if (open_file_in < 0) {
        perror("Eroare la deschiderea fisierului");
        close(open_file_in);
        exit(-1);
    }

    if (lseek(open_file_in, 2, SEEK_SET) < 0) {
        perror("Eroare cursor dimensiune \n");
    }

    if ((read(open_file_in, &size, 4) < 0) && type!=2) {
        perror("Eroare dimensiune");
    }

    if (lseek(open_file_in, 18, SEEK_SET) < 0) {
        perror("Eroare cursor latime \n");
    }

    if ((read(open_file_in, &width, 4) < 0) && type!=2) {
        perror("Eroare latime");
    }

    if (lseek(open_file_in, 22, SEEK_SET) < 0) {
        perror("Eroare cursor inaltime \n");
    }

    if ((read(open_file_in, &height, 4) < 0) && type!=2) {
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

    //afisare(pathname,height,width,file_info,size,modification_time_str);
    if(type == 1)
        scrieFisierBMP(open_file_out,pathname,height,width,size,file_info,modification_time_str);
    else if(type == 0)
        scrieFisierNormal(open_file_out,pathname,size,file_info,modification_time_str);
    else if(type == 2)
        scrieDirector(open_file_out,pathname,file_info);
    
    if (close(open_file_in) < 0) {
        perror("Eroare la inchidere");
        exit(EXIT_FAILURE);
    }
}

void afisareFisiere(char *director, int open_file_out) {
    DIR *dir;
    struct dirent *intrare;
    int type; 
    dir = opendir(director);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        exit(-1);
    }
    while ((intrare = readdir(dir)) != NULL) {
        if (strcmp(intrare->d_name, ".") != 0 && strcmp(intrare->d_name, "..") != 0) {
            if (intrare->d_type == DT_DIR) {
                openFile(intrare->d_name,open_file_out,2);
            } else {
                
                if (strcmp(intrare->d_name + strlen(intrare->d_name) - 4, ".bmp") == 0) {
                    type = 1;
                } else {
                    type = 0;
                }

            openFile(intrare->d_name,open_file_out,type);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {

    char *director = argv[1];
    int flags_out = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode_out = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int open_file_out = open("statistica.txt",flags_out, mode_out);

    if (argc != 2) {
        printf("Usage ./%s <fisier_intrare>\n", argv[0]);
        exit(-1);
    }
    if (open_file_out < 0) {
        perror("Eroare la deschiderea fisierului statistica.txt");
        close(open_file_out);
        exit(-1);
    } 

    afisareFisiere(director,open_file_out);
    

    if (close(open_file_out) < 0) {
        perror("Eroare la inchidere");
        exit(EXIT_FAILURE);
    }

    return 0;
}
