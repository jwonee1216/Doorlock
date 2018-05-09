#ifndef TABLE_MANAGER_H_
#define TABLE_MANAGER_H_

typedef struct message message_t;

struct message {

};

typedef struct table_manager table_manager_t;

struct table_manager {
		
};

int initialize_table_manager(table_manager_t *, const char *);
message_t *code_to_command(table_manager_t *, unsigned int);
char *devid_to_ip(table_manager_t *, unsigned int, char *);


#endif
