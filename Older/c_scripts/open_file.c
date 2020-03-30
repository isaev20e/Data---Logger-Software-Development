#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){

    DIR *data;
    FILE *new_file;
    struct dirent *entry;
    data = opendir("./data");

    char *target;
    int num;
    int file_num = 0;
    char file_name[100];

    if(data == NULL){
        puts("Unable to read directory");
        return(1);
    }else{
        while(entry=readdir(data)){
          target = entry->d_name+18;
          target[strlen(target)-4]=0;
          num = atoi(target);
          if(num>file_num){
            file_num = num;
          }
        }
        file_num++;
    }
    sprintf(file_name, "data/%d.txt",file_num);
    new_file = fopen(file_name, "w");

    return 0;

}
