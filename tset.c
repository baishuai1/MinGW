#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MOD_ADLER      ((int)65521)

void byte_to_hexstr(char *src, char *dest, int len){
	char record[4] = {0};
	int i = 0;
	unsigned char c1;
	unsigned char c2;
	    for (i = 0; i < len; i++)
    	{
        	c1 = src[i] & 0xF; 
        	c2 = (src[i] >> 4) & 0xF;
        	sprintf(dest + i * 2, "%x%x", c2, c1);
        }
}

int alder32(char *data, int len) {
	int a = 1, b = 0;
	int index;
	printf("len = %d \r\n", len);
	if (len > 1024) {
		len = 1024;
		printf("alder32 error %d \r\n", len);
	}
	for (index = 0; index < len; index++) {
		a = (a + data[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}
	return (b << 16) | a;
}

void main(void) {
	FILE * fpr = NULL;
	FILE * fpw = NULL;
	int i = 0;
	int j = 0;
	int symbol = 0;
	int size = 0;
	int total_len;
	int eff_len;
	int file_pos;
	int checksum;
	int check_index;
	static unsigned char hex_buff[512];
	static unsigned char bin_buff[512];
	static unsigned char str_buff[1024];
	static unsigned int INIT_SIZE = 460;
	static unsigned int read_size = 0;


	// char *p = "hello";
	// char dest[12] = {0};
	//byte_to_hexstr(p, dest, 10);
	//printf("dest:%s", dest);

	hex_buff[0] = 0x55;
	hex_buff[1] = 0x55;

	strcpy(&hex_buff[9], "864867022511015\0");

	hex_buff[24] = 0x04;

	hex_buff[29] = 0xF0;
	hex_buff[30] = 0x01;

	if ((fpr = fopen("mt210.bin", "rb")) == NULL) {
		printf("不能读取资源文件\n");
		return -1;
	}

	if ((fpw = fopen("tset.txt", "wb+")) == NULL) {
		printf("不能写入目标文件!\n");
		return -1;
	}

	fseek(fpr, 0L, SEEK_END);
	symbol = ftell(fpr) % INIT_SIZE;
	if (symbol == 0) {
		size = ftell(fpr) / INIT_SIZE;
	} else {
		size = ftell(fpr) / INIT_SIZE + 1;
	}

	for (j = 0; j < size; j++) {
		read_size = INIT_SIZE;

		memset(bin_buff, 0, sizeof(bin_buff));
		fseek(fpr, j * INIT_SIZE, SEEK_SET);
		if (j == size - 1 && symbol > 0) {
			read_size = symbol;
		}
		fread(bin_buff, read_size, 1, fpr); /* 读一条记录 */

		hex_buff[25] = (char) (j >> 8); //编号
		hex_buff[26] = (char) (j);

		eff_len = 10 + read_size;     //有效长度
		hex_buff[27] = (char) (eff_len >> 8);
		hex_buff[28] = (char) (eff_len);

		total_len = 29 + eff_len + 4 + 2;  //总长度
		hex_buff[2] = (char) (total_len >> 8);
		hex_buff[3] = (char) (total_len);

		file_pos = j * read_size;     //地址偏移
		hex_buff[35] = (char) (file_pos >> 24);
		hex_buff[36] = (char) (file_pos >> 16);
		hex_buff[37] = (char) (file_pos >> 8);
		hex_buff[38] = (char) (file_pos);

		memcpy(hex_buff + 39, bin_buff, read_size);  //bin文件

		checksum = alder32(hex_buff + 2, total_len - 8);
		check_index = total_len - 6;
		hex_buff[check_index] = (char) (checksum >> 24);
		hex_buff[check_index + 1] = (char) (checksum >> 16);
		hex_buff[check_index + 2] = (char) (checksum >> 8);
		hex_buff[check_index + 3] = (char) (checksum);
		hex_buff[check_index + 4] = 0xAA;
		hex_buff[check_index + 5] = 0xAA;
		byte_to_hexstr(hex_buff, str_buff, total_len);

		printf("str_buff = %s\n", str_buff);
		fwrite(str_buff, total_len * 2, 1, fpw); /* 写一条记录 */
		fprintf(fpw, "\n");
	}
	fclose(fpr);
	fclose(fpw);

}

