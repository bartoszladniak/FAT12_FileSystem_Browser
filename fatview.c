#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "funtions.h"
#include <string.h>
int main(int argc, char **argv)
{
    if(argc!=2)
        return 0;
    filename = *(argv+1);
    if(!readblock(NULL, 0,0)) { //Wczytanie disk data
        printf("Volume doesn't exist or it is broken!");
        return 1;
    }
    printf("Loading FAT Volume: %s, %s\n",get_volume_name() ,get_oem_name());
    //Ładowanie FAT1, FAT2, ROOT,FS
    if(loadMEM())
    {
        printf("ERROR: Couldn't load FAT structures: Fat arrays");
        free_data();
        return 3;
    }
    if(compareFAT(FileStructure.fat1,FileStructure.fat2)){
        printf("ERROR: FAT structure broken");
        free_data();
        return 3;
    }
    //LOAD ROOT
      struct root_directory * ROOT = (struct root_directory *)FileStructure.root_dir;

//    ___________________________USER INTERFACE________________________________
    char directory[150] = "/";
    current_directory = ROOT;
    char input[140]={0};
    do{
        printf("%s$:", directory);
        fgets(input,140, stdin);
        *(input+strlen(input)-1)=0;
        if(strcmp(input, "exit") == 0)
            break;
        if(strcmp(input, "dir") == 0)
             dir();
            //dir(current_directory);
        if(strcmp(input, "pwd") == 0)
            printf("%s\n",directory);
        if(strcmp(input, "rootinfo") == 0)
            rootinfo();
        if(strcmp(input, "spaceinfo") == 0)
            spaceinfo();
        if(strcmp(input, "help") == 0)
            printf("Commands: exit\n dir\npwd\nhelp\ncd <directory>\ncd ..\ncat <filename>, get <filename>\nzip <input file> <input file> <output file>\nrootinfo\nspaceinfo\nfileinfo <filename>\n");
        if(input[0] == 'c' &&input[1] == 'd' &&input[2] == ' '){
                struct root_directory * temp = opendir(input+3);
                if(temp)
                {
                    if(strcmp(input, "cd ..")==0)
                    {
                        unsigned int i = strlen(directory)-1;
                        for(;i>0;i--)
                            if(*(directory+i)=='/')
                            {
                                *(directory+i) = 0;
                                break;
                            }
                        if(i == 0)
                            *(directory+1) = 0;

                    }else {
                        if (strlen(directory) != 1)
                            strcat(directory, "/");
                        strcat(directory, input + 3);
                    }
                    if(strcmp(input, "cd ..")==0)
                        current_directory = closedir();
                    else
                        current_directory = opendir(input+3);
                }
                else
                    printf("There is no such a folder\n");
            }
        if(input[0] == 'f' &&input[1] == 'i' &&input[2] == 'l'&&input[3] == 'e' && input[4] == 'i' && input[5] == 'n' && input[6] == 'f' && input[7] == 'o' && input[8] !=0){
            printf("FILE: %s%s ",directory, input+8);
//            char new_name[14] = {' '};
            char * name = input+9;
            struct root_directory * temp =open(name);
            if(!temp){
                printf("doesn't exist!\n");
            }
            else
            {
                printf("exists: \n");
                union attributes att;
                att.attributes = temp->file_attributes;
                if(att.elements.subdirectory)
                    printf(" D");
                if(att.elements.read)
                    printf(" R");
                if(att.elements.archive)
                    printf(" A");
                if(att.elements.hidden)
                    printf(" H");
                if(att.elements.device)
                    printf(" Device");
                if(att.elements.vol_label)
                    printf(" Volume label");
                if(att.elements.system)
                    printf(" System file");
                printf("\nSize of file: %u\n", temp->file_size);
                int y = divide_bytes(15, 9, (temp)->file_modified_date) + 1980;
                int mo = divide_bytes(8, 5, (temp)->file_modified_date);
                int d = divide_bytes(4, 0, (temp)->file_modified_date);
                int h = divide_bytes(15, 11, (temp)->file_modified_time_h_m_s);
                int m = divide_bytes(10, 5, (temp)->file_modified_time_h_m_s);
                int s = divide_bytes(4, 0, (temp)->file_modified_time_h_m_s) / 2;
                printf("Last modification:  %02d:%02d:%02d    %d-%d-%d\n",h,m,s,d,mo,y );
                y = divide_bytes(15, 9, (temp)->file_creation_date) + 1980;
                mo = divide_bytes(8, 5, (temp)->file_creation_date);
                d = divide_bytes(4, 0, (temp)->file_creation_date);
                h = divide_bytes(15, 11, (temp)->file_modified_time_h_m_s);
                m = divide_bytes(10, 5, (temp)->file_modified_time_h_m_s);
                s = divide_bytes(4, 0, (temp)->file_modified_time_h_m_s) / 2;
                printf("Creation date:  %02d:%02d:%02d    %d-%d-%d\n",h,m,s,d,mo,y );
                printf("Clusters: ");

                int l = number_of_clusters(name);
                printf("\nNumber of clusters: %d\n", l);



            }

        }
        if(input[0] == 'c' &&input[1] == 'a' &&input[2] == 't'&&input[3] == ' ' &&input[4] !=0){
            cat_viewer(input+4);
        }
        if(input[0] == 'g' &&input[1] == 'e' &&input[2] == 't'&&input[3] == ' ' &&input[4] !=0){
            cat_downloader(input+4);
        }
        if(input[0] == 'z' &&input[1] == 'i' &&input[2] == 'p'&&input[3] == ' ' &&input[4] !=0){
            if(zip_downloader(input+4))
            {
                printf("ZIP: Couldn't open or save file\n");
            }
        }
        } while(1);
    free_data();
    return 0;
}
