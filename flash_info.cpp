#if 0

#include "mbed.h"
#include "FlashIAP.h"

int main() {
    FlashIAP flash;
    flash.init();  // 初始化 FlashIAP

    uint32_t flash_start = flash.get_flash_start();  //Get the starting address of Flash
    uint32_t flash_size = flash.get_flash_size();  //Get the total size of Flash
    uint32_t flash_end = flash_start + flash_size;  //Calculate the ending address of Flash
    uint32_t sector_size = flash.get_sector_size(flash_end - 1);  //Get the last sector size of Flash
    uint32_t storage_addr = flash_end - sector_size;  // Calculate the address of the data stored in Flash memory 

    printf("\n=== Flash information ===\n");
    printf("Flash starting address: 0x%08X\n", flash_start);
    printf("Flash total size: %u bytes (%u KB)\n", flash_size, flash_size / 1024);
    printf("Last sector size: %u bytes\n", sector_size);
    printf("Recommended storage address: 0x%08X\n", storage_addr);

    flash.deinit();
    return 0;
}

#endif