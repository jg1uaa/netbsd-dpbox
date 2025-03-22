#ifndef CRC_H
#define CRC_H


/* Vier verschiedene CRC-Verfahren:

   Das erste ist die CRC16, (Register mit 0 initialisieren),
   das zweite ist die FCS aus AX.25 (Register mit 0xFFFF initialisieren),
   das dritte ist das CRC-Verfahren aus THP/SP/GP/DieBox (mit 0 initialisieren),
   das vierte die CRC im F6FBB - S&F (ebenfalls mit 0 initialisieren).

  */

extern void crc_16(unsigned char Data, unsigned short *crc);
extern void crcfcs(unsigned char Data, unsigned short *crc);
extern void crcthp(unsigned char Data, unsigned short *crc);
extern void crcfbb(unsigned char Data, unsigned short *crc);
extern void checksum8(unsigned char Data, unsigned short *crc);
extern void checksum16(unsigned char Data, unsigned short *crc);

extern void crc_16_buf(unsigned char *adr, long size, unsigned short *crc);
extern void crcfcs_buf(unsigned char *adr, long size, unsigned short *crc);
extern void crcthp_buf(unsigned char *adr, long size, unsigned short *crc);
extern void crcfbb_buf(unsigned char *adr, long size, unsigned short *crc);
#ifdef WITHCRC32
extern void crc_32_buf(unsigned char *adr, long size, unsigned long *crc);
#endif
extern void checksum8_buf(unsigned char *adr, long size, unsigned short *crc);
extern void checksum16_buf(unsigned char *adr, long size, unsigned short *crc);

#endif /*CRC_H*/
