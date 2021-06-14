#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include <ftw.h>

#include <pwd.h>
#include <grp.h>
#include <time.h>

#define _XOPEN_SOURCE 500

static unsigned long total = 0;

int sum(const char *fpath, const struct stat *sb, int typeflag) {
    total += sb->st_size;
    return 0;
}

int compareSize(const void *const A, const void *const B)
{
    return strcmp((*(struct dirent **) A)->d_name, (*(struct dirent **) B)->d_name);
}

int getChmod(char *path){
    struct stat ret;

    if (stat(path, &ret) == -1) {
        return -1;
    }

    return (ret.st_mode & S_IRUSR)|(ret.st_mode & S_IWUSR)|(ret.st_mode & S_IXUSR)|/*owner*/
        (ret.st_mode & S_IRGRP)|(ret.st_mode & S_IWGRP)|(ret.st_mode & S_IXGRP)|/*group*/
        (ret.st_mode & S_IROTH)|(ret.st_mode & S_IWOTH)|(ret.st_mode & S_IXOTH);/*other*/
}

void getPermissions(){
        struct stat fileStat;
        printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
        printf(" ");
}

void getNumOfLinks(){
        struct stat fileStat;
        printf("%ld ", fileStat.st_nlink);
}

void getOwnerGroup(struct dirent **list, int index){
        struct stat sb;
        char outstr[200];

        stat(list[index]->d_name, &sb);
            
        struct passwd *pw = getpwuid(sb.st_uid);
        struct group  *gr = getgrgid(sb.st_gid);

        printf("%s %s ", pw->pw_name, gr->gr_name);
}

void getFileSize(){
        struct stat fileStat;
        printf("%ld ", fileStat.st_size);
}

void getLastModifiedTime(struct dirent **list, int index){
        struct stat attrib;
        stat(list[index]->d_name, &attrib);
        char date[10];
        strftime(date, 20, "%d-%m-%y", localtime(&(attrib.st_ctime)));
        printf("%s ", date);
}

void functionL(struct dirent **list, int countOfNames){

        for (int index = 0; index < countOfNames; ++index) {

                getChmod(list[index]->d_name);

                getPermissions();
                getNumOfLinks();
                getOwnerGroup(list, index);
                getFileSize();
                getLastModifiedTime(list, index);

                fputs(list[index]->d_name, stdout);
                printf("\n");
        }
}

void functionM(struct dirent **list, int countOfNames){

        for (int index = 0; index < countOfNames; ++index) {
                fputs(list[index]->d_name, stdout);
                if(index+1 != countOfNames) printf(", ");
                else printf("\n");
        }
}

void functionR(struct dirent **list, int countOfNames){
        for (int index = countOfNames - 1; index > 0; --index) 
                puts(list[index]->d_name);
}

void functionS()
{
	DIR *current_dir = NULL;
        struct dirent *current_entry = NULL;

        current_dir = opendir(".");

	if (current_dir == NULL)
        {
                fprintf(stderr, "%s %d: opendir() failed (%s)\n", __FILE__, __LINE__, strerror(errno));
                exit(-1);
        }
        current_entry = readdir(current_dir);

        while (current_entry != NULL)
        {
                if (current_entry->d_name[0] != '.'){
                        struct stat st;
                        lstat(current_entry->d_name, &st);
                        off_t size = st.st_size;
                        total = 0;
			if (ftw(current_entry->d_name, &sum, 1)){
                                perror("ftw");
                        }
                        fputs(current_entry->d_name, stdout);
                        fprintf(stdout, " - %.1f MB\n", total/1000000.0);
                }
                current_entry = readdir(current_dir);
        }
        closedir(current_dir);
}

void function(int opt)
{
	int countOfNames = 0;
        DIR *current_dir = NULL;
        struct dirent **list = NULL;
        struct dirent *current_entry = NULL;

        current_dir = opendir(".");
        if (current_dir == NULL){
                fprintf(stderr, "%s %d: opendir() failed (%s)\n", __FILE__, __LINE__, strerror(errno));
                exit(-1);
        }

        while ((current_entry = readdir(current_dir)) != NULL)
                if (current_entry->d_name[0] != '.') ++countOfNames;
        list = malloc(countOfNames * sizeof(*list));

        rewinddir(current_dir); /* reset position */
        countOfNames = 0;
        while ((current_entry = readdir(current_dir)) != NULL)
                if (current_entry->d_name[0] != '.') list[countOfNames++] = current_entry;
        qsort(list, countOfNames, sizeof(*list), compareSize);

        switch(opt) {  
                        case 'l':
                                functionL(list, countOfNames);
                                break;
                        case 'm':
                                functionM(list, countOfNames);
                                break;
			case 's': 
				functionS();
                                break;
			case 'r':
				functionR(list, countOfNames);
                                break;
                        case '?':
				fprintf(stderr, "Unknown option: %c\n", optopt);
	} 
        if (opt == -1) for (int index = 0 ; index < countOfNames ; ++index) puts(list[index]->d_name); 
        closedir(current_dir);
}

int main(int argc, char *argv[]){

	int opt;
        opt = getopt(argc, argv, ":if:lmsr");
        function(opt);
	
	return 0;
}

