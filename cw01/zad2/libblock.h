void create_blocks(const int SIZE);
void set_search_dir_and_file(const char* dir_name, const char* tmp_file_name, const char* file_name);
void search_and_tmp_save();
int create_block_from_tmp_file();
int search_dir(const char* dir_name, const char* tmp_file_name, const char* file_name);
void delete_block(int index);
void free_mem();

