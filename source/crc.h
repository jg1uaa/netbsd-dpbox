#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

/* Vier verschiedene CRC-Verfahren:

   Das erste ist die CRC16, (Register mit 0 initialisieren),
   das zweite ist die FCS aus AX.25 (Register mit 0xFFFF initialisieren),
   das dritte ist das CRC-Verfahren aus THP/SP/GP/DieBox (mit 0 initialisieren),
   das vierte die CRC im F6FBB - S&F (ebenfalls mit 0 initialisieren).

  */

void crc_16(unsigned char Data, unsigned short *crc);
void crcfcs(unsigned char Data, unsigned short *crc);
void crcthp(unsigned char Data, unsigned short *crc);
void crcfbb(unsigned char Data, unsigned short *crc);
void checksum8(unsigned char Data, unsigned short *crc);
void checksum16(unsigned char Data, unsigned short *crc);

void crc_16_buf(unsigned char *adr, size_t size, unsigned short *crc);
void crcfcs_buf(unsigned char *adr, size_t size, unsigned short *crc);
void crcthp_buf(unsigned char *adr, size_t size, unsigned short *crc);
void crcfbb_buf(unsigned char *adr, size_t size, unsigned short *crc);
#ifdef WITHCRC32
void crc_32_buf(unsigned char *adr, size_t size, uint32_t *crc);
#endif
void checksum8_buf(unsigned char *adr, size_t size, unsigned short *crc);
void checksum16_buf(unsigned char *adr, size_t size, unsigned short *crc);

#endif /*CRC_H*/
