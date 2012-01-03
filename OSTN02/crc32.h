static void make_crc_table(void);
static unsigned long update_crc(unsigned long crc, unsigned char *buf, int len);
unsigned long crc32(unsigned char *buf, int len);
