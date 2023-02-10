#include "partition_util.h"

void ICACHE_FLASH_ATTR LoadDefaultPartitionMap(void)
{
    if(system_partition_table_regist(
                    partition_table_opt2,
                    sizeof(partition_table_opt2) / sizeof(partition_table_opt2[0]),
                    SPI_FLASH_SIZE_MAP_OPT2))
    {
    }
    else if(system_partition_table_regist(
                    partition_table_opt3,
                    sizeof(partition_table_opt3) / sizeof(partition_table_opt3[0]),
                    SPI_FLASH_SIZE_MAP_OPT3))
    {
    }
    else if(system_partition_table_regist(
                partition_table_opt4,
                sizeof(partition_table_opt4) / sizeof(partition_table_opt4[0]),
                SPI_FLASH_SIZE_MAP_OPT4))
    {
    }
    else
    {
        os_printf("system_partition_table_regist all fail\r\n");
        while(1);
    }
}

