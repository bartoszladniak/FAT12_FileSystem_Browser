//
// Created by Bartosz on 23/05/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "funtions.h"
#include <string.h>
struct fat_data * data = NULL;
uint16_t current_directory_order =0;
int strcpy_l(char * dest ,const char * src, unsigned size){
    if(!dest||!src||size==0)
        return 0;
    for(int i=0;i<size;i++)
        dest[i] = src[i];
    return 0;
}
struct root_directory * open(char * name){
    if(!name)
        return NULL;
    int i=0;
    char new_name[14] = {' '};

    *(new_name+13) = 0;
    for(int l=0;l<12;l++)
        new_name[l]=' ';
    for (int l = 0; name[l]!='\0'; l++) {
        if(name[l] >= 'a' && name[l] <= 'z') {
            name[l] = name[l] -32;
        }
    }
    for(unsigned l=0;l<strlen(name);l++)
    {
        if(*(name+l) != '.')
            *(new_name+l) = *(name+l);
        else{
            for(int s = strlen((name+l))-2;s>=0;s--)
                *(new_name+10-s) = *(name+strlen(name)-s-1);
            break;
        }
    }
    short found = 0;
    while((current_directory+i)->first_character)
    {
        //Rozciaganie tekstu (filename) na format FAT
        if((current_directory+i)->first_character == name[0] && strcmp(new_name+1, (current_directory+i)->filename)==0)
        {
            found = 1;
            break;
        }
        i++;
    }
    if(found && !(((current_directory+i)->file_attributes)&16))
        return (current_directory+i);
    else
        return NULL;
}
struct root_directory * opendir(char * name){
    if(!strcmp(name, "."))
        return NULL;
    for (int l = 0; (name[l]!='\0')&&l<12; l++) {
        if(name[l] >= 'a' && name[l] <= 'z') {
            name[l] = name[l] -32;
        }
    }
    struct root_directory * temp = find_directory(current_directory,name);
    if(temp && (temp->file_attributes)&16){
        current_directory_order = temp->low_order;
        return load_directory(temp->low_order);
    }
    else
        return NULL;
}
struct root_directory * closedir(){
    struct root_directory *  ret = opendir("..");
    char name[12]={0};
    if(ret)
    {
        struct root_directory * lower = opendir("..");
        name[0] = lower->first_character;
        strcpy_l(name+1, lower->filename,10);
        struct root_directory * element = find_directory(lower,name);
        if(element)
            current_directory_order = element->low_order;
        else
            current_directory_order = 0;
    }
    else
        return NULL;
    return ret;
};
//LOADING DATA
void FileStructureToNull()
{
    FileStructure.root_dir = NULL;
    FileStructure.fat1 = NULL;
    FileStructure.fat2 = NULL;
    FileStructure.files = NULL;
}
int loadMEM(){
    if(data) {
        FileStructure.fat1 = malloc(sizeof(data->sizeofeachFAT) * data->bytespersector);
        if (!FileStructure.fat1) {
            FileStructureToNull();
            return 1;
        }
        FileStructure.fat2 = malloc(sizeof(data->sizeofeachFAT) * data->bytespersector);
        if (!FileStructure.fat2) {
            free(FileStructure.fat2);
            FileStructureToNull();

            return 2;
        }
        if (!readblock(FileStructure.fat1, data->size_of_reserved_area, data->sizeofeachFAT)) {
            free(FileStructure.fat1);
            free(FileStructure.fat2);
            FileStructureToNull();
            return 6;
        }
        if (!readblock(FileStructure.fat2, data->size_of_reserved_area + data->sizeofeachFAT, data->sizeofeachFAT)) {
            free(FileStructure.fat1);
            free(FileStructure.fat2);
            FileStructureToNull();
            return 7;
        }
        FileStructure.root_dir = malloc(sizeof(struct root_directory) * data->max_files_number_root);
        if (!FileStructure.root_dir) {
            free(FileStructure.fat1);
            free(FileStructure.fat2);
            FileStructureToNull();
            return 3;
        }
        if (!readblock(FileStructure.root_dir, data->size_of_reserved_area + (data->numberoffats * data->sizeofeachFAT),
                       (sizeof(struct root_directory) * (data->max_files_number_root) / data->bytespersector) ? (
                               (sizeof(struct root_directory) * (data->max_files_number_root)) / data->bytespersector)
                                                                                                              : 1)) {
            free(FileStructure.fat1);
            free(FileStructure.fat2);
            free(FileStructure.root_dir);
            FileStructureToNull();
            return 7;
        }


        FileStructure.files = malloc(data->numberofsecotrsinFS * data->bytespersector);
        if (!FileStructure.files) {
            free(FileStructure.fat1);
            free(FileStructure.fat2);
            free(FileStructure.root_dir);
            FileStructureToNull();
            return 3;
        }
        uint16_t sectors_for_rootdir = ((data->max_files_number_root * sizeof(struct root_directory)) / 512) ? (
                (data->max_files_number_root * sizeof(struct root_directory)) / 512) : 1;
        if (!readblock(FileStructure.files,
                       data->size_of_reserved_area + (data->numberoffats * data->sizeofeachFAT) + sectors_for_rootdir,
                       data->numberofsecotrsinFS)) {
            free(FileStructure.fat1);
            free(FileStructure.fat2);
            free(FileStructure.root_dir);
            free(FileStructure.files);
            FileStructureToNull();
            return 8;
        }
    }
    return 0;
}
size_t readblock(void* buffer, uint32_t first_block, size_t block_count){
    file = fopen( filename , "rb");
    if(!file)
        return 0;
    //W przypadku pierwszego wywolania
    if(data == NULL)
    {
            data = malloc(sizeof(struct fat_data));
            if(!data){
                printf("Error occurred: boot sector memory allocation");
                fclose(file);
                return 0;
            }
            if(!fread ( data, sizeof(struct fat_data), 1, file )){
                free(data);
                data = NULL;
                printf("ERROR! Couldn't load volume data!");
                fclose(file);
                return 0;
            }
            if(!data->sector_per_cluster || !data->max_files_number_root || !data->bytespersector || !data->numberofsecotrsinFS || !data->sizeofeachFAT || data->numberoffats==0||!data->sectors_per_track)
            {
                free(data);
                data = NULL;
                printf("ERROR! Incorrect data in boot sector!");
                fclose(file);
                return 0;
            }
    }
    if(block_count==0)
        return 1;//pierwsze wywolanie moze byc puste

    if(!buffer ||  block_count == 0 || !data ) // W przypadku 'blednych' danych dane dysku zostana wczytane przy pierwszym wywolaniu.
    {
        fclose(file);
        return 0;
    }
    int i=0;
    uint16_t a=0;
    fseek(file, (data->bytespersector * first_block), SEEK_SET);
    if(!fread ( buffer, data->bytespersector, block_count, file )){
        fclose(file);
        return 0;
    }
    fclose(file);
    return block_count;
}
uint16_t fat12_allocation_table(uint16_t n){
    if(n==0)
        return 0;
    uint16_t ret = 0;
    uint8_t a = 0,b=0,c=0;
    //yz Zx XY
    if(n%2==0){
        a = ((long)*((uint8_t *)(FileStructure.fat1 + ((3*n)/2))));
        b = ((long)*((uint8_t *)(FileStructure.fat1 + ((3*n)/2)+1)));
        c = ((long)*((uint8_t *)(FileStructure.fat1 + (2+(3*n)/2))));

        ret = ((b&15)<<8) + a;
    }
    else
    {
        a = ((long)*((uint8_t *)(FileStructure.fat1 + ((3*n)/2)-1)));
        b = ((long)*((uint8_t *)(FileStructure.fat1 + ((3*n)/2))));
        c = ((long)*((uint8_t *)(FileStructure.fat1 + (1+(3*n)/2))));
        ret = (c<<4) + (b>>4);
    }
    return ret;
}
//Dir_viewer prints data from one cluster. Dir sends to dir_viewer clusters to print.
void dir_viewer (struct root_directory * Root_t) {
    int entries_counter=0;
        do {
            if ((Root_t + entries_counter)->first_character == '\0' || (Root_t + entries_counter)->file_size < 0 ||
                (Root_t + entries_counter)->file_attributes == 0x0f) {
                entries_counter++;
                continue;
            }
            printf("%c%10s ", (Root_t + entries_counter)->first_character, (Root_t + entries_counter)->filename);
            if (((Root_t + entries_counter)->file_attributes) & 16)
                printf(" <DIR>");
            else
                printf("%6d", (Root_t + entries_counter)->file_size);

            int y = divide_bytes(15, 9, (Root_t + entries_counter)->file_modified_date) + 1980;
            int mo = divide_bytes(8, 5, (Root_t + entries_counter)->file_modified_date);
            int d = divide_bytes(4, 0, (Root_t + entries_counter)->file_modified_date);
            int h = divide_bytes(15, 11, (Root_t + entries_counter)->file_modified_time_h_m_s);
            int m = divide_bytes(10, 5, (Root_t + entries_counter)->file_modified_time_h_m_s);
            int s = divide_bytes(4, 0, (Root_t + entries_counter)->file_modified_time_h_m_s) / 2;
            printf("     %02d:%02d:%02d    %d-%d-%d\n", h, m, s, d, mo, y);
       entries_counter++;
       //If main root
       if((uint8_t *)Root_t == FileStructure.root_dir && data->max_files_number_root<entries_counter)
           break;
    }
    while(entries_counter < (data->bytespersector*data->sector_per_cluster)/ sizeof(struct root_directory));

}
void dir(){
    if(!current_directory)
    {
        return;
    }
    printf("FILENAME       SIZE     TIME        DATE\n");
    uint16_t a=current_directory_order;
    //Directory has only one cluster or it is a main root:

    if(current_directory_order==0 || fat_block_end(fat12_allocation_table(a)))
    {
        dir_viewer(current_directory);
        return;
    }
    int loops = 0;
    long block_size =(data->bytespersector * data->sector_per_cluster );
    do{
        if(loops ==0 && !fat_block_end(a))
            a = current_directory_order;
        else if(fat_block_end(a))
            break; //?
        dir_viewer((struct root_directory *)(FileStructure.files + (block_size * (a-2))));
        a = fat12_allocation_table(a*fat_entry_size);
        loops++;
    }
    while(!fat_block_end(a));

}
int compareFAT(const unsigned char * F1,const unsigned char * F2){
    int i=0;
    while(data->sizeofeachFAT > i)
    {
        if(*(F1+i) != *(F2+i))
            return 1;
        i++;
    }
    return 0;
}
int divide_bytes(int first_number, int second_number, uint16_t num){
    int x = 0;
    for( int i = first_number; i >= second_number; i-- ) {
        int bit = (int)(num&(1ll<<i));
        if( bit != 0)
            x = x + ((i - second_number)*( i - second_number));
    }
    return x;
}
struct root_directory * find_directory_in_sector(struct root_directory * Root_t, char * string)
{
    int entries_counter=0;
    do {
        char temp[12]={0};
        if((Root_t+entries_counter)->file_size !=0)
            continue;
        temp[0] = (Root_t+entries_counter)->first_character;
        for(int i=0;i<10;i++) {
            if((Root_t + entries_counter)->filename[i] ==' ')break;
            *(temp + i + 1) = (Root_t + entries_counter)->filename[i];
        }
        if(strcmp(temp, string) == 0)
            return Root_t+entries_counter;
    }
    while((++entries_counter)<data->max_files_number_root && ((Root_t+entries_counter)->first_character)!=0);
    return NULL;
}
struct root_directory * find_directory(struct root_directory * Root_t, char * string){
    int loops = 0;
    long block_size =(data->bytespersector * data->sector_per_cluster );
    if(Root_t==NULL || string ==NULL)
        return NULL;
    uint16_t a = current_directory_order;

    struct root_directory * found = find_directory_in_sector(Root_t, string);
    if(found)
        return found;


    if(current_directory_order==0 || fat_block_end(fat12_allocation_table(a)))
    {
        return find_directory_in_sector(current_directory, string);
    }
    do{
        if(loops ==0 && !fat_block_end(a))
            a = current_directory_order;
        else if(fat_block_end(a))
            break; //?
        struct root_directory * found = find_directory_in_sector((struct root_directory *)(FileStructure.files + (block_size * (a-2))), string);
        if(found)
            return found;

        a = fat12_allocation_table(a*fat_entry_size);
        loops++;
    }
    while(!fat_block_end(a));
    return NULL;
}
struct root_directory * load_directory( u_int16_t start)
{
    if(start == 0)
        return (struct root_directory *)(FileStructure.root_dir);
    else
        return (struct root_directory *)(FileStructure.files + (start*data->bytespersector*data->sector_per_cluster)-(data->sector_per_cluster*data->bytespersector)*2);
}

void cat_viewer(char * name){

    if( name == NULL ||strlen(name)==0)
        return;

    struct root_directory * dir = open(name);
    if(!dir)
    {
        printf("CAT: ERROR\nFile doesn't exist\n");
        return;
    }
    printf("CAT___FILE_VIEWER___\nFile content:\n");

    uint16_t a=0;
    a = fat12_allocation_table(fat_entry_size*(dir)->low_order);
    int loops = 0;
    long block_size =(data->bytespersector * data->sector_per_cluster );
    do{
        char text[block_size+1];

        if(loops ==0 && !fat_block_end(a))
            a = dir->low_order;
        if((fat_block_end(a)) && loops == 0)
            strcpy_l(text, (char *)(FileStructure.files+(data->bytespersector *(data->sector_per_cluster)* dir->low_order)-(data->sector_per_cluster*data->bytespersector)*2),block_size);
        else if(fat_block_end(a))
            break; //?
        else
            strcpy_l(text, (char *)(FileStructure.files+(data->bytespersector*(data->sector_per_cluster) * a)-(data->sector_per_cluster*data->bytespersector)*2),block_size);
        *(text+((data->bytespersector * data->sector_per_cluster )))= 0;
        printf("%s",text);

        if(!fat_block_end(a)){ //terminator fatu
            a = fat12_allocation_table(a*fat_entry_size);
        }
        loops++;
    }
    while(!fat_block_end(a));

}
int cat_downloader(char * name){

        if( name == NULL ||strlen(name)==0)
            return -1;
        struct root_directory * dir = open(name);
        if(!data)
        {
            printf("CAT: ERROR\nFile doesn't exist\n");
            return -1;
        }
        FILE * save = fopen(name, "w");
        if(!save)
        {
            printf("Couldn't create file");
            return -2;
        }
        printf("Saving file...\n");

 //       uint16_t a=0;
//        a = fat12_allocation_table(fat_entry_size*(dir)->low_order);
//        int loops = 0;
//        do{
//            char text[(data->bytespersector)+1];
////            if(loops ==0)
////                a = dir->low_order;
//            if((fat_block_end(a)) && loops == 0)
//                strcpy_l(text, (char *)(FileStructure.files+(data->bytespersector *data->sector_per_cluster* dir->low_order)-(data->sector_per_cluster*data->bytespersector)*2),data->bytespersector);
//            else if( fat_block_end(a))
//                break; //?
//            else
//                strcpy_l(text, (char *)(FileStructure.files+(data->bytespersector *data->sector_per_cluster* a)-(data->sector_per_cluster*data->bytespersector)*2),data->bytespersector);
//            *(text+data->bytespersector)= 0;
//            if(!fwrite(text, sizeof(char), strlen(text), save))
//            {
//                printf("Error: Couldn't write data to file!");
//                fclose(save);
//                return 8;
//            }
//
//            if(!fat_block_end(a)){ //terminator fatu
//                a = fat12_allocation_table(fat_entry_size*a);
//
//            }
//            loops++;
//        }
//    while( !fat_block_end(a));
    uint16_t a=0;
    a = fat12_allocation_table(fat_entry_size*(dir)->low_order);
    int loops = 0;
    long block_size =(data->bytespersector * data->sector_per_cluster );
    do{
        char text[block_size+1];

        if(loops ==0 && !fat_block_end(a))
            a = dir->low_order;
        if((fat_block_end(a)) && loops == 0)
            strcpy_l(text, (char *)(FileStructure.files+(data->bytespersector *(data->sector_per_cluster)* dir->low_order)-(data->sector_per_cluster*data->bytespersector)*2),block_size);
        else if(fat_block_end(a))
            break; //?
        else
            strcpy_l(text, (char *)(FileStructure.files+(data->bytespersector*(data->sector_per_cluster) * a)-(data->sector_per_cluster*data->bytespersector)*2),block_size);
        *(text+((data->bytespersector * data->sector_per_cluster )))= 0;
        if(!fwrite(text, sizeof(char), strlen(text), save))
        {
            printf("Error: Couldn't write data to file!");
            fclose(save);
            return 8;
        }
        if(!fat_block_end(a)){ //terminator fatu
            a = fat12_allocation_table(a*fat_entry_size);
        }
        loops++;
    }
    while(!fat_block_end(a));

    printf("File saved!\n");
    fclose(save);
    return 0;
}
int zip_downloader(char * files){

    if( files == NULL ||strlen(files)==0)
        return -1;
    long block_size =(data->bytespersector * data->sector_per_cluster );
    char * txt = strtok( files, " " );
    struct root_directory * file1 = NULL;
    struct root_directory * file2 = NULL;
    FILE * save = NULL;
    int files_counter = 0;
    while( txt != NULL )
    {
        if(files_counter==0){
            file1 = open(txt);
            if(!file1)
                return -1;
            printf("Input: %s\n",txt);

        }
        if(files_counter==1){
            file2 = open(txt);
            if(!file2)
                return -1;
            printf("Input: %s\n", txt);

        }
        if(files_counter==2){
            save = fopen(txt, "w");
            if(!save)
                return -3;
            printf("Output: %s\n", txt);

        }
        txt = strtok( NULL, " ");
        files_counter++;
    }
    if(files_counter!=3 || save == NULL || file1 == NULL || file2 == NULL)
        return -4;
    printf("Saving file...\n");
    uint16_t a=0, b=0;
//    a = (long)*((long *)(FileStructure.fat1 + (2*(file1)->low_order)));
//    b = (long)*((long *)(FileStructure.fat1 + (2*(file2)->low_order)));


    int loops = 0;
    char textA[block_size+1];
    char textB[block_size+1];
    char *saveptr1=NULL, *saveptr2 =NULL;

    do{

        if(loops == 0){
            a = file1->low_order;
            b = file2->low_order;
        }
        if(saveptr1==NULL){
            if(fat_block_end(a) && loops == 0)
                strcpy_l(textA, (char *)(FileStructure.files+(data->bytespersector *data->sector_per_cluster* file1->low_order)-(data->sector_per_cluster*data->bytespersector)*2), block_size);
            else if((!fat_block_end(a))){
                strcpy_l(textA, (char *)(FileStructure.files+(data->bytespersector*data->sector_per_cluster * a)-(data->sector_per_cluster*data->bytespersector)*2), block_size);
            }
            *(textA+block_size)= 0;

        }
        if(saveptr2==NULL){
        if((fat_block_end(b)) && loops == 0) {
            strcpy_l(textB, (char *) (FileStructure.files + (data->bytespersector *data->sector_per_cluster* file2->low_order)-(data->sector_per_cluster*data->bytespersector)*2),  block_size);
        }
        else if(!fat_block_end(b)) {
            strcpy_l(textB, (char *) (FileStructure.files + (data->bytespersector * data->sector_per_cluster * b) -
                                      (data->sector_per_cluster * data->bytespersector) * 2), block_size);
        }
            *(textB+block_size)= 0;

        }

//        if(a!=0xff08 && a != 0xff0f) {
//            if(!fwrite(textA, sizeof(char), strlen(textA), save))
//            {
//                printf("Error: Couldn't write data to file!");
//                fclose(save);
//                return 8;
//            }
//        }
//        if(b!=0xff08 && b != 0xff0f) {
//            if(!fwrite(textB, sizeof(char), strlen(textB), save))
//            {
//                printf("Error: Couldn't write data to file!");
//                fclose(save);
//                return 8;
//            }
//        }
        char * sentenceA, * sentenceB;
        if(saveptr1==NULL)
            sentenceA = strtok_r( textA, ".", &saveptr1 );
        if(saveptr2==NULL)
            sentenceB = strtok_r( textB, ".", &saveptr2);

        while(saveptr1 || saveptr2)
        {
            if(sentenceA == NULL && sentenceB==NULL)
                break;
            if(sentenceA)
            {
                if(!fwrite(sentenceA, sizeof(char), strlen(sentenceA), save))
                {
                    printf("Error: Couldn't write data to file!");
                    fclose(save);
                    return 8;
                }
                if(!fwrite(".", sizeof(char), 1, save))
                {
                    printf("Error: Couldn't write data to file!");
                    fclose(save);
                    return 8;
                }
                sentenceA = NULL;

            }
            if(sentenceB){
                if(!fwrite(sentenceB, sizeof(char), strlen(sentenceB), save))
                {
                    printf("Error: Couldn't write data to file!");
                    fclose(save);
                    return 8;
                }
                if(!fwrite(".", sizeof(char), 1, save))
                {
                    printf("Error: Couldn't write data to file!");
                    fclose(save);
                    return 8;
                }
                sentenceB = NULL;

            }


            if(saveptr1)
                sentenceA = strtok_r( NULL, ".", &saveptr1 );
            if(saveptr2)
                sentenceB = strtok_r( NULL, ".", &saveptr2);
            if(sentenceA == NULL && !fat_block_end(a))
                break;
            if(sentenceB == NULL && !fat_block_end(b))
                break;
            if(sentenceA == NULL && fat_block_end(a) && sentenceB == NULL && fat_block_end(b))
            {
                printf("File saved!");
                fclose(save);
                return 0;
            }

        }

        if(!fat_block_end(a) && !saveptr1){ //terminator fatu
            a = fat12_allocation_table(a);
        }
        if(!fat_block_end(b) && !saveptr2){ //terminator fatu
            b = fat12_allocation_table(b);
        }
        loops++;
    }
    while(!fat_block_end(a) || !fat_block_end(b));
    printf("File saved!");
   fclose(save);

    return 0;
}

char * get_volume_name(){
    return data->volume_label;

}
char * get_oem_name(){
    return data->Oem_name;
}
void free_data(){
    if(FileStructure.fat1)
        free(FileStructure.fat1);
    if(FileStructure.fat2)
        free(FileStructure.fat2);
    if(FileStructure.files)
        free(FileStructure.files);
    if(FileStructure.root_dir)
        free(FileStructure.root_dir);
    if(data)
        free(data);
}

//Info
void rootinfo(){
    if(data){
        int entries = 0;
        struct root_directory * Root = (struct root_directory *) FileStructure.root_dir;
        int entries_counter = 0;
        do {
            if((Root+entries_counter)->first_character == '\0')continue;
            entries++;
        }
        while((++entries_counter)<data->max_files_number_root && ((Root+entries_counter)->first_character)!=0);
        printf("Number of entries in root directory: %d\n", entries);
        printf("Maximum number of elements in root: %u\n", data->max_files_number_root);
        printf("Used space: %2.2f%%\n", ((float)entries/(float)data->max_files_number_root)*100);

    }
}
void spaceinfo()
{
    if(data){
        uint16_t fat_walker;
        unsigned long used=0;
        unsigned long free=0;
        unsigned long broken=0;
        unsigned long endingclusters=0;
        unsigned long reserved=2;
        long i=2;
        do{
            fat_walker = fat12_allocation_table(i);
            if(fat_walker == 0)
                free++;
            if(fat_walker>=2 && fat_walker<=0xfef){
                /*if(fat_walker>=data->numberofsecotrsinFS/data->sector_per_cluster)
                {
                  //adres do nieistniejacego miejsca w pamieci ze wzgledu na ograniczony rozmiar (adres>rozmiar fs)

                    broken++;
                    i++;
                    continue;
                }
                else*/
                used++;
            }
            if(fat_walker>=0xff0 && fat_walker<=0xff6)
            {
                used++;
                reserved++;
            }
            if(fat_walker == 0xff7)
                broken++;
            if(fat_walker>=0xff8 && fat_walker<=0xfff)
            {
                used++;
                endingclusters++;
            }

            i++;
        }
        while(i<data->numberofsecotrsinFS/data->sector_per_cluster);//do max ilosci klastrow
        printf("Space info:\n Used clusters: %lu\n Free clusters: %lu\n Last clusters: %lu\n Broken clusters: %lu\n Reserved: %lu\n",used, free, endingclusters,broken, reserved);

    }
}
int number_of_clusters(char * name){
    if( name == NULL ||strlen(name)==0)
        return 0;
    struct root_directory * dir = open(name);
    if(!dir)
        return 0;
    uint16_t a=0;

    a =  fat12_allocation_table(fat_entry_size*(dir)->low_order);
    int loops = 0;
    do{
        if(loops ==0)
            a = dir->low_order;
        printf("%d ",a);

        if(!fat_block_end(a)){ //terminator fatu
            a =  fat12_allocation_table(fat_entry_size*a);

        }
        loops++;
    }
    while(!fat_block_end(a));
    return loops;
}
