#pragma once
#ifndef DEDUP_CONTAINERS_H
#define DEDUP_CONTAINERS_H

#include <stdio.h>
#include <stdlib.h>
#include "comdef.h"
#include "dedup_data_set.h"

Dedup_Error_Val parse_file(char* file_name, PDedup_data_set data_set);
Dedup_Error_Val parse_header(FILE *fd, PDedup_data_set data_set);
Dedup_Error_Val print_data_set(PDedup_data_set data_set, char *fileName);
Dedup_Error_Val delete_system(PDedup_data_set data_set, uint32 system_num);
Dedup_Error_Val read_user_input(PDedup_data_set data_set);


#endif // !DEDUP_CONTAINERS_H
