//
// Created by Bartosz on 23/05/2020.
//
#define fat_entry_size 1
#define fat_block_end(a)(((a)>=0xff8 && (a)<=0xfff)?1:0)
struct __attribute__((packed)) fat_data
{
    char assembly[3] ;
    char Oem_name[8];
    uint16_t bytespersector;
    uint8_t sector_per_cluster;
    uint16_t size_of_reserved_area;
    uint8_t numberoffats; //1-2
    uint16_t max_files_number_root;
    uint16_t numberofsecotrsinFS;
    uint8_t mediatype;
    uint16_t sizeofeachFAT;
    uint16_t sectors_per_track;
    uint16_t heads_storagedevice;
    uint32_t sectors_before_start;
    uint32_t numberofsectorsinFS_LARGE;
    uint8_t bios_int;
    uint8_t empty_space;
    uint8_t boot_signature;
    uint32_t serial_number;
    char volume_label[11];
    char FS_type[8]; //COŚ ŹLE
    char not_used2[448];
    uint16_t signature_value;

};
struct __attribute__((packed)) root_directory
{
    char first_character;
    char filename[10];
    uint8_t file_attributes;
    uint8_t reserved;
    uint8_t file_creation_time_s;
    uint16_t file_creation_time_h_m_s;
    uint16_t file_creation_date;
    uint16_t file_access_date;
    uint16_t high_order;
    uint16_t file_modified_time_h_m_s;
    uint16_t file_modified_date;
    uint16_t low_order;
    int file_size;
};
struct content{
    uint8_t *fat1;
    uint8_t *fat2;
    uint8_t *root_dir;
    uint8_t *files;

} FileStructure;
union attributes{
    uint8_t attributes;
    struct{
        uint8_t read:1;
        uint8_t hidden:1;
        uint8_t system:1;
        uint8_t vol_label:1;
        uint8_t subdirectory:1;
        uint8_t archive:1;
        uint8_t device:1;
        uint8_t unused:1;
    }elements;
};

//Zmienne globalne stworzone na potrzeby uproszczenia funkcji
struct root_directory * current_directory;
FILE * file;
char * filename;


//API podstawowe
struct root_directory * open(char * filename);
struct root_directory * opendir(char * name);
struct root_directory * closedir();
size_t readblock(void* buffer, uint32_t first_block, size_t block_count);



int loadMEM();
struct root_directory * find_directory(struct root_directory *, char *);
void dir ();
int compareFAT(const unsigned char *,const unsigned char *);
int divide_bytes(int first_number, int second_number, uint16_t num);
struct root_directory * load_directory(u_int16_t start);
void cat_viewer(char * name);
int cat_downloader(char * name);
int zip_downloader(char * files);

char * get_volume_name();
char * get_oem_name();
void free_data();
//Stats
void rootinfo ();
void spaceinfo();
int number_of_clusters(char * name);
#ifndef SYSOPY_PROJEKT3_FUNTIONS_H
#define SYSOPY_PROJEKT3_FUNTIONS_H

#endif //SYSOPY_PROJEKT3_FUNTIONS_H
