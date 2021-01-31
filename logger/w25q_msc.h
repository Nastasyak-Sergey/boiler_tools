#ifndef W25Q_MSC
#define W25Q_MSC

#define FLASH_SIZE 8388608l   // 64 Mbit = 32768*256  8388608
#define FLASH_MAX_BLOCKS (FLASH_SIZE / 512)

int write_block(uint32_t lba, const uint8_t *copy_from);
int read_block(uint32_t block_no, uint8_t *data);

#endif //W25Q_MSC
