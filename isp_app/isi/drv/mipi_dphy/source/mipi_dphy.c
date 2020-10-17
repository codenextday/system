#include "mipi_dphy.h"
#include "isi.h"
#include "isi_iss.h"
#include <fpga/altera_fpga.h>

#include "comdef.h"
#include "sys_con3.h"

//unsigned int mipi_dphy0_base_addr, mipi_dphy1_base_addr, mipi_dphy2_base_addr;

//unsigned mipi_dphy_adr[3] = {mipi_dphy0_base_addr, mipi_dphy1_base_addr, mipi_dphy2_base_addr};
volatile unsigned int* mipi_vi_regs_mmap;
volatile unsigned int* mipi_dphy2_regs_mmap;
volatile unsigned int* mipi_dphy1_regs_mmap;
volatile unsigned int* mipi_dphy0_regs_mmap;

unsigned int imi_GetMipiRegVaddr(int fd)
{
	mipi_dphy0_regs_mmap = (unsigned int*) mmap(NULL, MIPI_REGISTER_SPACE_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x3000);

	MIPI_DBG("mmap mipi dphy0 regs addr = 0x%8x\n",(unsigned int)mipi_dphy0_regs_mmap);
	if (!mipi_dphy0_regs_mmap || (mipi_dphy0_regs_mmap == MAP_FAILED)){
		MIPI_ERR("mmap mipi dphy0 regs addr failed \n");
		mipi_dphy0_regs_mmap = NULL;
		return -1;
	}

	mipi_dphy1_regs_mmap = (unsigned int*) mmap(NULL, MIPI_REGISTER_SPACE_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x4000);
	MIPI_DBG("mmap mipi dphy1 regs addr = 0x%8x\n",(unsigned int)mipi_dphy1_regs_mmap);
	if (!mipi_dphy1_regs_mmap || (mipi_dphy1_regs_mmap == MAP_FAILED)){
		MIPI_ERR("mmap mipi dphy1 regs addr failed\n");
		mipi_dphy1_regs_mmap = NULL;
		return -1;
	}

	mipi_dphy2_regs_mmap = (unsigned int*) mmap(NULL, MIPI_REGISTER_SPACE_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x5000);
	MIPI_DBG("mmap mipi dphy2 regs addr = 0x%8x\n",(unsigned int)mipi_dphy2_regs_mmap);
	if (!mipi_dphy2_regs_mmap || (mipi_dphy2_regs_mmap == MAP_FAILED)){
		MIPI_ERR("mmap mipi dphy2 regs addr failed\n");
		mipi_dphy1_regs_mmap = NULL;
		return -1;
	}

	mipi_vi_regs_mmap = (unsigned int*) mmap(NULL, MIPI_REGISTER_SPACE_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x6000);
	MIPI_DBG("mmap mipi vi regs addr = 0x%8x\n",(unsigned int)mipi_vi_regs_mmap);
	if (!mipi_vi_regs_mmap || (mipi_vi_regs_mmap == MAP_FAILED)) {
		MIPI_ERR("mmap mipi vi regs addr failed\n");
		mipi_vi_regs_mmap = NULL;
		return -1;
	}

	return 0;
}

static unsigned int imi_mipi0_regs_read(unsigned int addr)
{
	return mipi_dphy0_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)];
}

static unsigned int imi_mipi1_regs_read(unsigned int addr)
{
	return mipi_dphy1_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)];
}

static unsigned int imi_mipi2_regs_read(unsigned int addr)
{
	return mipi_dphy2_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)];
}

/*static unsigned int imi_vi_regs_read(unsigned int addr)
{
	return mipi_vi_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)];
}
*/

static void imi_mipi0_regs_write(unsigned int addr, unsigned data)
{
	mipi_dphy0_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)] = data;
}

static void imi_mipi1_regs_write(unsigned int addr, unsigned data)
{
	mipi_dphy1_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)] = data;
}

static void imi_mipi2_regs_write(unsigned int addr, unsigned data)
{
	mipi_dphy2_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)] = data;
}

static void imi_vi_regs_write(unsigned int addr, unsigned data)
{
	mipi_vi_regs_mmap[MIPI_DPHY_REG_ADDR_MOD(addr >> 2)] = data;
}

void imi_delay_fpga(UINT32 cycles)
{
	volatile UINT32 i;
	for(i = 0; i < cycles; i++);
}

unsigned int imi_freq2range(unsigned int freq)
{
	int i;
	unsigned int hsfreqranges[63];
	unsigned int hsfreqranges_ulim[63];
	hsfreqranges[ 0] = 0 ; //7'b0000000;
	hsfreqranges[ 1] = 16; //7'b0010000;
	hsfreqranges[ 2] = 32; //7'b0100000;
	hsfreqranges[ 3] = 48; //7'b0110000;
	hsfreqranges[ 4] = 1 ; //7'b0000001;
	hsfreqranges[ 5] = 17; //7'b0010001;
	hsfreqranges[ 6] = 33; //7'b0100001;
	hsfreqranges[ 7] = 49; //7'b0110001;
	hsfreqranges[ 8] = 2 ; //7'b0000010;
	hsfreqranges[ 9] = 18; //7'b0010010;
	hsfreqranges[10] = 34; //7'b0100010;
	hsfreqranges[11] = 50; //7'b0110010;
	hsfreqranges[12] = 3 ; //7'b0000011;
	hsfreqranges[13] = 19; //7'b0010011;
	hsfreqranges[14] = 35; //7'b0100011;
	hsfreqranges[15] = 51; //7'b0110011;
	hsfreqranges[16] = 4 ; //7'b0000100;
	hsfreqranges[17] = 20; //7'b0010100;
	hsfreqranges[18] = 37; //7'b0100101;
	hsfreqranges[19] = 53; //7'b0110101;
	hsfreqranges[20] = 5 ; //7'b0000101;
	hsfreqranges[21] = 22; //7'b0010110;
	hsfreqranges[22] = 38; //7'b0100110;
	hsfreqranges[23] = 55; //7'b0110111;
	hsfreqranges[24] = 7 ; //7'b0000111;
	hsfreqranges[25] = 24; //7'b0011000;
	hsfreqranges[26] = 40; //7'b0101000;
	hsfreqranges[27] = 57; //7'b0111001;
	hsfreqranges[28] = 9 ; //7'b0001001;
	hsfreqranges[29] = 25; //7'b0011001;
	hsfreqranges[30] = 41; //7'b0101001;
	hsfreqranges[31] = 58; //7'b0111010;
	hsfreqranges[32] = 10; //7'b0001010;
	hsfreqranges[33] = 26; //7'b0011010;
	hsfreqranges[34] = 42; //7'b0101010;
	hsfreqranges[35] = 59; //7'b0111011;
	hsfreqranges[36] = 11; //7'b0001011;
	hsfreqranges[37] = 27; //7'b0011011;
	hsfreqranges[38] = 43; //7'b0101011;
	hsfreqranges[39] = 60; //7'b0111100;
	hsfreqranges[40] = 12; //7'b0001100;
	hsfreqranges[41] = 28; //7'b0011100;
	hsfreqranges[42] = 44; //7'b0101100;
	hsfreqranges[43] = 61; //7'b0111101;
	hsfreqranges[44] = 13; //7'b0001101;
	hsfreqranges[45] = 29; //7'b0011101;
	hsfreqranges[46] = 46; //7'b0101110;
	hsfreqranges[47] = 62; //7'b0111110;
	hsfreqranges[48] = 14; //7'b0001110;
	hsfreqranges[49] = 30; //7'b0011110;
	hsfreqranges[50] = 47; //7'b0101111;
	hsfreqranges[51] = 63; //7'b0111111;
	hsfreqranges[52] = 15; //7'b0001111;
	hsfreqranges[53] = 64; //7'b1000000;
	hsfreqranges[54] = 65; //7'b1000001;
	hsfreqranges[55] = 66; //7'b1000010;
	hsfreqranges[56] = 67; //7'b1000011;
	hsfreqranges[57] = 68; //7'b1000100;
	hsfreqranges[58] = 69; //7'b1000101;
	hsfreqranges[59] = 70; //7'b1000110;
	hsfreqranges[60] = 71; //7'b1000111;
	hsfreqranges[61] = 72; //7'b1001000;
	hsfreqranges[62] = 73; //7'b1001001;

	hsfreqranges_ulim[ 0] = 165 ; //  82.5
	hsfreqranges_ulim[ 1] = 185 ; //  92.5
	hsfreqranges_ulim[ 2] = 205 ; //  102.
	hsfreqranges_ulim[ 3] = 225 ; //  112.
	hsfreqranges_ulim[ 4] = 245 ; //  122.
	hsfreqranges_ulim[ 5] = 265 ; //  132.
	hsfreqranges_ulim[ 6] = 285 ; //  142.
	hsfreqranges_ulim[ 7] = 305 ; //  152.
	hsfreqranges_ulim[ 8] = 325 ; //  162.
	hsfreqranges_ulim[ 9] = 345 ; //  172.
	hsfreqranges_ulim[10] = 365 ; // 182.5
	hsfreqranges_ulim[11] = 385 ; // 192.5
	hsfreqranges_ulim[12] = 415 ; // 207.5
	hsfreqranges_ulim[13] = 445 ; // 222.5
	hsfreqranges_ulim[14] = 475 ; // 237.5
	hsfreqranges_ulim[15] = 525 ; // 262.5
	hsfreqranges_ulim[16] = 575 ; // 287.5
	hsfreqranges_ulim[17] = 625 ; // 312.5
	hsfreqranges_ulim[18] = 675 ; // 337.5
	hsfreqranges_ulim[19] = 750 ; // 375;
	hsfreqranges_ulim[20] = 850 ; // 425;
	hsfreqranges_ulim[21] = 950 ; // 475;
	hsfreqranges_ulim[22] = 1050; // 525;
	hsfreqranges_ulim[23] = 1150; // 575;
	hsfreqranges_ulim[24] = 1250; // 625;
	hsfreqranges_ulim[25] = 1350; // 675;
	hsfreqranges_ulim[26] = 1450; // 725;
	hsfreqranges_ulim[27] = 1550; // 775;
	hsfreqranges_ulim[28] = 1650; // 825;
	hsfreqranges_ulim[29] = 1750; // 875;
	hsfreqranges_ulim[30] = 1850; // 925;
	hsfreqranges_ulim[31] = 1950; // 975;
	hsfreqranges_ulim[32] = 2050; // 1025;
	hsfreqranges_ulim[33] = 2150; // 1075;
	hsfreqranges_ulim[34] = 2250; // 1125;
	hsfreqranges_ulim[35] = 2350; // 1175;
	hsfreqranges_ulim[36] = 2450; // 1225;
	hsfreqranges_ulim[37] = 2550; // 1275;
	hsfreqranges_ulim[38] = 2650; // 1325;
	hsfreqranges_ulim[39] = 2750; // 1375;
	hsfreqranges_ulim[40] = 2850; // 1425;
	hsfreqranges_ulim[41] = 2950; // 1475;
	hsfreqranges_ulim[42] = 3050; // 1525;
	hsfreqranges_ulim[43] = 3150; // 1575;
	hsfreqranges_ulim[44] = 3250; // 1625;
	hsfreqranges_ulim[45] = 3350; // 1675;
	hsfreqranges_ulim[46] = 3450; // 1725;
	hsfreqranges_ulim[47] = 3550; // 1775;
	hsfreqranges_ulim[48] = 3650; // 1825;
	hsfreqranges_ulim[49] = 3750; // 1875;
	hsfreqranges_ulim[50] = 3850; // 1925;
	hsfreqranges_ulim[51] = 3950; // 1975;
	hsfreqranges_ulim[52] = 4050; // 2025;
	hsfreqranges_ulim[53] = 4150; // 2075;
	hsfreqranges_ulim[54] = 4250; // 2125;
	hsfreqranges_ulim[55] = 4350; // 2175;
	hsfreqranges_ulim[56] = 4450; // 2225;
	hsfreqranges_ulim[57] = 4550; // 2275;
	hsfreqranges_ulim[58] = 4650; // 2325;
	hsfreqranges_ulim[59] = 4750; // 2375;
	hsfreqranges_ulim[60] = 4850; // 2425;
	hsfreqranges_ulim[61] = 4950; // 2475;
	hsfreqranges_ulim[62] = 5050; // 2525;
	for(i = 0; i < 63; i++) {
		if( (freq * 2) <= hsfreqranges_ulim[i])
			break;
	}

	if(i == 63)
		while(1){};//sim_fail(0xdeadbeaf);
	return hsfreqranges[i];
}

void imi_sendtestcode_offset(unsigned int idx, unsigned int code)
{
	switch(idx) {
		case (0):
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0  & 0xff));//tb_testdin  = 8'h00;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));    //tb_testen   = 1'b1;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk  = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen   = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (code & 0xff));//tb_testdin  = code;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			break;
		case (1):
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (0x0  & 0xff));//tb_testdin  = 8'h00;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));    //tb_testen   = 1'b1;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk  = 1'b0;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen   = 1'b0;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (code & 0xff));//tb_testdin  = code;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			break;
		case (2):
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (0x0  & 0xff));//tb_testdin  = 8'h00;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));    //tb_testen   = 1'b1;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk  = 1'b0;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen   = 1'b0;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (code & 0xff));//tb_testdin  = code;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			break;
		default:
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0  & 0xff));//tb_testdin  = 8'h00;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));    //tb_testen   = 1'b1;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk  = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen   = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (code & 0xff));//tb_testdin  = code;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk  = 1'b1;
			break;
	}
}

void imi_sendtestdata(unsigned int idx, unsigned int data)
{
	switch (idx) {
		case (0):
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (data & 0xff));//tb_testdin = data;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk = 1'b1;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			break;
		case (1):
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (data & 0xff));//tb_testdin = data;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk = 1'b1;
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			break;
		case (2):
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (data & 0xff));//tb_testdin = data;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk = 1'b1;
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			break;
		default:
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (data & 0xff));//tb_testdin = data;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));     //tb_testclk = 1'b1;
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0;
			break;
	}
}

void imi_sendtestcode(unsigned int idx, unsigned int code)
{
	switch (idx) {
		case (0):
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));                //tb_testen  = 1'b1; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));                 //tb_testclk = 1'b1; //0: phy_testclr, 1: phy_testclk
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16) | (code & 0xff));//tb_testdin = code; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0; //0: phy_testclr, 1: phy_testclk
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
		case (1):
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));                //tb_testen  = 1'b1; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));                 //tb_testclk = 1'b1; //0: phy_testclr, 1: phy_testclk
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16) | (code & 0xff));//tb_testdin = code; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0; //0: phy_testclr, 1: phy_testclk
			imi_mipi1_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
		case (2):
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));                //tb_testen  = 1'b1; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));                 //tb_testclk = 1'b1; //0: phy_testclr, 1: phy_testclk
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16) | (code & 0xff));//tb_testdin = code; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0; //0: phy_testclr, 1: phy_testclk
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
		default:
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16));                //tb_testen  = 1'b1; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x1<<1));                 //tb_testclk = 1'b1; //0: phy_testclr, 1: phy_testclk
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x1<<16) | (code & 0xff));//tb_testdin = code; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, (0x0<<1));     //tb_testclk = 1'b0; //0: phy_testclr, 1: phy_testclk
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL1, (0x0<<16));    //tb_testen  = 1'b0; //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
	}
}

void imi_setfreq(int idx, int freq, int cfg_clk_freq)
{
  /*
   * to do, set clock gen: txescclk, refclk, cfg_clk
   * set bps/hsbyteclk:2.5GHz, 1.5GHz, 80MHz
   * set tx esc clk   : 5MHz , 10MHz , 20MHz
   * set ref clk      :24MHz, range: 2 ~ 64MHz
   * set cfg clk      :24MHz, range:17 ~ 27MHz
   */
	unsigned int wr_dat;
	unsigned int phy_enable = 1;
	unsigned int base_dir0  = 1; //for lane0, 1:RX, 0:TX
	unsigned int cfgclkfreqrange = (cfg_clk_freq - 17) * 4;
	unsigned int hsfreqrange = imi_freq2range( freq );
	unsigned int osc_freq_target = 0;
	wr_dat = phy_enable + (base_dir0<<1) + (cfgclkfreqrange<<2) + (hsfreqrange<<10);

	if(idx == 0) {
		imi_vi_regs_write(VI_CSI_0_DPHY_CTRL, wr_dat);
		imi_vi_regs_write(VI_CSI_0_DPHY_LANE0, 0x0);
	}
	else if(idx == 1){
		imi_vi_regs_write(VI_CSI_1_DPHY_CTRL, wr_dat);
		imi_vi_regs_write(VI_CSI_1_DPHY_LANE0, 0x0);
	} else if(idx == 2){
		imi_vi_regs_write(VI_CSI_2_DPHY_CTRL, wr_dat);
		imi_vi_regs_write(VI_CSI_2_DPHY_LANE0, 0x0);
	}
#ifdef MIPI_TEST_CHIP
	{
		unsigned int disable_isp = 0;
		unsigned int debug_sel = 0;
		unsigned int dpu0_sel  = 0; //0:dvp  0, 1:mipi 0; 2:mipi 1, 3:mipi 2
		unsigned int dpu1_sel  = 0; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp  1
		unsigned int dpu2_sel  = 0; //0:mipi 1, 1:mipi 2, 2:dvp  1, 3:dvp  2
		unsigned int isp_sel   = 0; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp 2
		unsigned int iftester  = 4;//4:glue tester, 2: RX tester, 1:TX tester
		unsigned int zcal_rstz = 1;
		unsigned int cfg_top_ctrl = ((disable_isp & 1)<<0) | ((debug_sel & 7)<<1) | ((dpu0_sel & 3)<<4) | ((dpu1_sel & 3)<<6) | ((dpu2_sel & 3)<<8) | ((isp_sel & 3)<<10);
		cfg_top_ctrl |= ((iftester & 7)<<12) | ((zcal_rstz & 1)<<15); //only used for FPGA test-chip
		imi_vi_regs_write(VI_TOP_CTRL, cfg_top_ctrl );  //[11:10]: 2-mipi2, ... ,0-mipi0 ;
	}

	osc_freq_target = 438;
	imi_sendtestcode( idx, 0x05 );
	imi_sendtestdata( idx, cfgclkfreqrange ); //[7:0]
	imi_sendtestcode( idx, 0x06 );
	imi_sendtestdata( idx, hsfreqrange ); //[7:0]
	{
		unsigned int disable_isp = 0;
		unsigned int debug_sel = 0;
		unsigned int dpu0_sel  = 0; //0:dvp  0, 1:mipi 0; 2:mipi 1, 3:mipi 2
		unsigned int dpu1_sel  = 0; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp  1
		unsigned int dpu2_sel  = 0; //0:mipi 1, 1:mipi 2, 2:dvp  1, 3:dvp  2
		unsigned int isp_sel   = 0; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp 2
		unsigned int iftester  = 2;//4:glue tester, 2: RX tester, 1:TX tester
		unsigned int zcal_rstz = 1;
		unsigned int cfg_top_ctrl = ((disable_isp & 1)<<0) | ((debug_sel & 7)<<1) | ((dpu0_sel & 3)<<4) | ((dpu1_sel & 3)<<6) | ((dpu2_sel & 3)<<8) | ((isp_sel & 3)<<10);
		cfg_top_ctrl |= ((iftester & 7)<<12) | ((zcal_rstz & 1)<<15); //only used for FPGA test-chip
		imi_vi_regs_write( VI_TOP_CTRL          , cfg_top_ctrl );  //[11:10]: 2-mipi2, ... ,0-mipi0 ;
	}

#endif
	/*
	 * config DPHY PLL
	 * Write osc_freq_target
	 */
	imi_sendtestcode_offset ( idx, 0x00 );
	imi_sendtestcode        ( idx, 0xE2 );
	imi_sendtestdata        ( idx, osc_freq_target & 0xff ); //[7:0]
	imi_sendtestcode_offset ( idx, 0x00 );
	imi_sendtestcode        ( idx, 0xE3 );
	imi_sendtestdata        ( idx, (osc_freq_target >>8) & 0xf ); //[11:8]
	imi_sendtestcode_offset ( idx, 0x00 );
	imi_sendtestcode        ( idx, 0xE4 );
	imi_sendtestdata        ( idx, 0x01 );
}

unsigned int imi_readtestdout(unsigned int idx)
{
	unsigned int rd_dat, tb_testdout;
	switch (idx) {
		case (0):
			rd_dat = imi_mipi0_regs_read(MIPI_PHY_TEST_CTRL1);    //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
		case (1):
			rd_dat = imi_mipi1_regs_read(MIPI_PHY_TEST_CTRL1);    //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
		case (2):
			rd_dat = imi_mipi2_regs_read(MIPI_PHY_TEST_CTRL1);    //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
		default:
			rd_dat = imi_mipi0_regs_read(MIPI_PHY_TEST_CTRL1);    //7:0 phy_testdin, 15:8 phy_testdout, 16: phy_testen
			break;
	}
  tb_testdout = (rd_dat>>8) & 0xff;
  return tb_testdout;
}

void imi_startrstdphy(int idx, int mode)
{
	MIPI_DBG("%s : Enter\n", __FUNCTION__);
	imi_delay_fpga(5);

	switch (idx) {
		case (0):
			imi_mipi0_regs_write(MIPI_PHY_SHUTDOWNZ , 0x0); //tb_shutdownz = 1'b0 ; //0: phy_shutdownz, active low
			imi_mipi0_regs_write(MIPI_DPHY_RSTZ, 0x0); //tb_rstz      = 1'b0 ; //0: phy_rstz     , active low
			imi_mipi0_regs_write(MIPI_PHY_TEST_CTRL0, 0x1); //tb_testclr   = 1'b0 ; //0: phy_testclr, 1: phy_testclk
			imi_delay_fpga(5);
			imi_mipi0_regs_write(MIPI_PHY_TEST_CTRL0, 0x0); //tb_testclr   = 1'b0 ; //bit 0: phy_testclr, bit 1: phy_testclk
			break;
		case (1):
			imi_mipi1_regs_write(MIPI_PHY_SHUTDOWNZ , 0x0); //tb_shutdownz = 1'b0 ; //0: phy_shutdownz, active low
			imi_mipi1_regs_write(MIPI_DPHY_RSTZ, 0x0); //tb_rstz      = 1'b0 ; //0: phy_rstz     , active low
			imi_mipi1_regs_write(MIPI_PHY_TEST_CTRL0, 0x1); //tb_testclr   = 1'b0 ; //0: phy_testclr, 1: phy_testclk
			imi_delay_fpga(5);
			imi_mipi1_regs_write(MIPI_PHY_TEST_CTRL0, 0x0); //tb_testclr   = 1'b0 ; //bit 0: phy_testclr, bit 1: phy_testclk
			break;
		case (2):
			imi_mipi2_regs_write( MIPI_PHY_SHUTDOWNZ, 0x0); //tb_shutdownz = 1'b0 ; //0: phy_shutdownz, active low
			imi_mipi2_regs_write( MIPI_DPHY_RSTZ, 0x0); //tb_rstz      = 1'b0 ; //0: phy_rstz     , active low
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, 0x1); //tb_testclr   = 1'b0 ; //0: phy_testclr, 1: phy_testclk
			imi_delay_fpga(5);
			imi_mipi2_regs_write( MIPI_PHY_TEST_CTRL0, 0x0); //tb_testclr   = 1'b0 ; //bit 0: phy_testclr, bit 1: phy_testclk
			break;
		default:
			imi_mipi0_regs_write( MIPI_PHY_SHUTDOWNZ, 0x0); //tb_shutdownz = 1'b0 ; //0: phy_shutdownz, active low
			imi_mipi0_regs_write( MIPI_DPHY_RSTZ, 0x0); //tb_rstz      = 1'b0 ; //0: phy_rstz     , active low
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, 0x1); //tb_testclr   = 1'b0 ; //0: phy_testclr, 1: phy_testclk
			imi_delay_fpga(5);
			imi_mipi0_regs_write( MIPI_PHY_TEST_CTRL0, 0x0); //tb_testclr   = 1'b0 ; //bit 0: phy_testclr, bit 1: phy_testclk
			break;
	}
	if(idx == 0){
		imi_vi_regs_write( VI_CSI_0_DPHY_LANE0, 0x0);
	} else if(idx == 1){
		imi_vi_regs_write( VI_CSI_1_DPHY_LANE0, 0x0);
	} else if(idx == 2){
		imi_vi_regs_write( VI_CSI_2_DPHY_LANE0, 0x0);
	}
	MIPI_DBG("%s : Exit\n", __FUNCTION__);
}

void imi_relrstdphy(int idx, int freq, int cfg_clk_freq)
{
	unsigned int rd_dat;
	unsigned int stopstatedata_0;
	unsigned int stopstateclk;
	MIPI_DBG("%s : Enter, mipi_idx=%d\n", __FUNCTION__, idx);
	imi_setfreq(idx, freq, cfg_clk_freq); //changefreqrange(currentfreq); // PLL controlled by PHY

	imi_sendtestcode_offset (idx, 0x0 );
	imi_sendtestcode        (idx, 0x8 );
	imi_sendtestdata        (idx, 0x38);
	switch (idx) {
		case (0):
			imi_mipi0_regs_write( MIPI_PHY_SHUTDOWNZ, 0x1); //tb_shutdownz = 1'b1;
			imi_delay_fpga(5);
			imi_mipi0_regs_write( MIPI_DPHY_RSTZ, 0x1); //#(5) tb_rstz = 1'b1;
			imi_delay_fpga(5);                                          //#(DUMMY_DELAY);
			break;
		case (1):
			imi_mipi1_regs_write( MIPI_PHY_SHUTDOWNZ, 0x1); //tb_shutdownz = 1'b1;
			imi_delay_fpga(5);
			imi_mipi1_regs_write( MIPI_DPHY_RSTZ, 0x1); //#(5) tb_rstz = 1'b1;
			imi_delay_fpga(5);                                          //#(DUMMY_DELAY);
			break;
		case (2):
			imi_mipi2_regs_write( MIPI_PHY_SHUTDOWNZ, 0x1); //tb_shutdownz = 1'b1;
			imi_delay_fpga(5);
			imi_mipi2_regs_write( MIPI_DPHY_RSTZ, 0x1); //#(5) tb_rstz = 1'b1;
			imi_delay_fpga(5);                                          //#(DUMMY_DELAY);
			break;
		default:
			imi_mipi0_regs_write( MIPI_PHY_SHUTDOWNZ, 0x1); //tb_shutdownz = 1'b1;
			imi_delay_fpga(5);
			imi_mipi0_regs_write( MIPI_DPHY_RSTZ, 0x1); //#(5) tb_rstz = 1'b1;
			imi_delay_fpga(5);                                          //#(DUMMY_DELAY);
			break;
	}
	while(1) {
		MIPI_DBG("%s idx = %d: \n", __FUNCTION__, idx);
		switch (idx) {
			case (0):
				rd_dat = imi_mipi0_regs_read( MIPI_PHY_STOPSTATE);
				break;
			case (1):
				rd_dat = imi_mipi1_regs_read( MIPI_PHY_STOPSTATE);
				break;
			case (2):
				rd_dat = imi_mipi2_regs_read( MIPI_PHY_STOPSTATE);
				break;
			default:
				rd_dat = imi_mipi0_regs_read( MIPI_PHY_STOPSTATE);
				break;
		}
		stopstatedata_0 = rd_dat & 0x1;
		stopstateclk    = (rd_dat>>16) & 0x1;

		MIPI_DBG("%s stopstatedata %d, stopstateclk %d :\n", __FUNCTION__, stopstatedata_0, stopstateclk);
		if(stopstatedata_0 & stopstateclk) {
			;//sim_step(0x1111ead0); //lane ready
			break;
		}
		else
			imi_delay_fpga(5);
	}
	imi_delay_fpga(10);
	MIPI_DBG("%s : Exit \n", __FUNCTION__);
}

void imi_cal_check(int idx, int mode, int freq)
{

	MIPI_DBG("%s : Enter, mipi_idx = %d \n", __FUNCTION__, idx);
	unsigned int offset_test_lanes ;  //reg [4:0]   offset_test_lanes   ;

	unsigned int res_error         ;  //reg         res_error           ;
	unsigned int testdout          ;
	int done_flag;
	int err_flag;
	res_error = 0 ;
	// Resistor calibration
	imi_sendtestcode(idx, 0x00); //8'h00);
	imi_sendtestdata(idx, 0x02); //8'h02);
	imi_sendtestcode(idx, 0x22); //8'h22);

	testdout = imi_readtestdout(idx);
	res_error = testdout & 0x1;
	imi_sendtestcode(idx, 0x21);
	testdout = imi_readtestdout(idx);
	if((testdout & 0x80) == 0x0)
		res_error = 0x80;
	if(res_error)
		while(1){};//sim_fail(0xca000000+res_error);

	/*Offset calibration*/
	imi_sendtestcode(idx, 0x00); //8'h00);
	imi_sendtestdata(idx, 0x03); //8'h03);
	imi_sendtestcode(idx, 0x9D); //8'h9D);
#ifdef FOURLANES
	offset_test_lanes = (0x0<<3); //5'b00000; // clk + 4
#else
	if(LANE_NUM==2)    offset_test_lanes = (0x3<<3); //5'b11_000; // clk + 2
	if(LANE_NUM==1)    offset_test_lanes = (0x7<<2); //5'b11_100; // clk + 2
#endif

	testdout = imi_readtestdout(idx);

	done_flag = testdout & 0x1;
	err_flag  = (testdout & 0x1E) != 0x10 ; //Not Error
	if(done_flag && err_flag)
		offset_test_lanes = offset_test_lanes | 0x1;
	else
		while(1){};//        sim_fail(0xca010000+offset_test_lanes+(testdout<<8));

	imi_sendtestcode(idx, 0x00);
	imi_sendtestdata(idx, 0x05);
	imi_sendtestcode(idx, 0x9F);

	if(mode == DIRECT) {
		testdout = imi_readtestdout(idx);
		done_flag = testdout & 0x2;
		err_flag  = testdout & 0x4;
		if(done_flag && !err_flag)
			offset_test_lanes = offset_test_lanes | 0x2;
		else
			while(1){};
	}
	else
		offset_test_lanes = offset_test_lanes | 0x2;

	if(LANE_NUM > 1) {
		imi_sendtestcode(idx, 0x00);
		imi_sendtestdata(idx, 0x07);
		imi_sendtestcode(idx, 0x9F);
		testdout = imi_readtestdout(idx);
		done_flag = testdout & 0x2  ;
		err_flag  = testdout & 0x4 ;
		if(done_flag && !err_flag)
			offset_test_lanes = offset_test_lanes | 0x4;
		else
			while(1){};
	}
#ifdef FOURLANES
	imi_sendtestcode(idx, 0x00); //8'h00);
	imi_sendtestdata(idx, 0x09); //8'h09);
	imi_sendtestcode(idx, 0x2F); //8'h2F);
	{
		int done_flag = testdout & 0x8;
		int err_flag  = testdout & 0x10;
		if(done_flag && !err_flag)
			skew_test_lanes = skew_test_lanes | 0x4;
		else
			sim_fail(0xca030000+skew_test_lanes+(testdout<<4));
	}

	/* for the remaining lanes while in TX2L */
	imi_sendtestcode(idx, 0x00); //8'h00);
	imi_sendtestdata(idx, 0x0B); //8'h0B);
	imi_sendtestcode(idx, 0x2F); //8'h2F);

	int done_flag = testdout & 0x8;
	int err_flag  = testdout & 0x10;
	if(done_flag && !err_flag)
		skew_test_lanes = skew_test_lanes | 0x8;
	else
		sim_fail(0xca030000+skew_test_lanes+(testdout<<4));
#endif
	MIPI_DBG("%s : Exit\n", __FUNCTION__);
}

void imi_resetdphy(int idx, int mode, int freq, int cfg_clk_freq)
{

  MIPI_DBG("%s : Enter\n", __FUNCTION__);

  imi_startrstdphy(idx, mode);
  imi_relrstdphy(idx, freq, cfg_clk_freq);
  imi_cal_check(idx, mode, freq);

  MIPI_DBG("%s : Exit\n", __FUNCTION__);
}

void imi_rx_dphy_test(int idx, int mode, int freq, int cfg_clk_freq)
{
	//("****************** Starting Reception Tests ********************\n\n");
	/*set up csi2 host*/

	MIPI_DBG("%s :Mipi to dvp write regs, mipi_idx = %d\n", __FUNCTION__, idx);
	switch (idx) {
		case (0):
			imi_mipi0_regs_write( MIPI_CSI2_RESET, 0x1       );//CSi2_RESET  //
			imi_mipi0_regs_write( MIPI_IPI_MODE         , 0x00000100);//MODE
			imi_mipi0_regs_write( MIPI_IPI_VCID         , 0x00000000);//VCID        //bit[1:0] :virtual channel
			imi_mipi0_regs_write( MIPI_IPI_DATA_TYPE    , MIPI_DAT_TYPE);//DATA_TYPE   //bit[5:0] :data type, 0x2c:raw12;
			imi_mipi0_regs_write( MIPI_IPI_HSA_TIME     , 0x0000001f);//HSA
			imi_mipi0_regs_write( MIPI_IPI_HBP_TIME     , 0x0000001f);//HBP
			imi_mipi0_regs_write( MIPI_IPI_HSD_TIME     , 0x0000000a);//HSD
			imi_mipi0_regs_write( MIPI_IPI_HLINE_TIME   , 0x00000650);//HLINE_TIM
			imi_mipi0_regs_write( MIPI_IPI_VSA_LINES    , 0x00000002);//VSA
			imi_mipi0_regs_write( MIPI_IPI_VBP_LINES    , 0x00000002);//VBP
			imi_mipi0_regs_write( MIPI_IPI_VFP_LINES    , 0x0000000f);//VFP
			imi_mipi0_regs_write( MIPI_IPI_VACTIVE_LINES, 0x00000078);//VACTIVE_LINE//
			imi_mipi0_regs_write( MIPI_N_LANES          , LANE_NUM - 1); //Lane enable, 0: 1 lane, 1: 2 lanes, 2: 3 lanes, 3: 4 lanes
			break;
		case (1):
			imi_mipi1_regs_write( MIPI_CSI2_RESET, 0x1       );//CSi2_RESET  //
			imi_mipi1_regs_write( MIPI_IPI_MODE         , 0x00000100);//MODE
			imi_mipi1_regs_write( MIPI_IPI_VCID         , 0x00000000);//VCID        //bit[1:0] :virtual channel
			imi_mipi1_regs_write( MIPI_IPI_DATA_TYPE    , MIPI_DAT_TYPE);//DATA_TYPE   //bit[5:0] :data type, 0x2c:raw12;
			imi_mipi1_regs_write( MIPI_IPI_HSA_TIME     , 0x0000001f);//HSA
			imi_mipi1_regs_write( MIPI_IPI_HBP_TIME     , 0x0000001f);//HBP
			imi_mipi1_regs_write( MIPI_IPI_HSD_TIME     , 0x0000000a);//HSD
			imi_mipi1_regs_write( MIPI_IPI_HLINE_TIME   , 0x00000650);//HLINE_TIME
			imi_mipi1_regs_write( MIPI_IPI_VSA_LINES    , 0x00000002);//VSA
			imi_mipi1_regs_write( MIPI_IPI_VBP_LINES    , 0x00000002);//VBP
			imi_mipi1_regs_write( MIPI_IPI_VFP_LINES    , 0x0000000f);//VFP
			imi_mipi1_regs_write( MIPI_IPI_VACTIVE_LINES, 0x00000078);//VACTIVE_LINE//
			imi_mipi1_regs_write( MIPI_N_LANES          , LANE_NUM - 1); //Lane enable, 0: 1 lane, 1: 2 lanes, 2: 3 lanes, 3: 4 lanes
			break;
		case (2):
			imi_mipi2_regs_write( MIPI_CSI2_RESET, 0x1       );//CSi2_RESET  //
			imi_mipi2_regs_write( MIPI_IPI_MODE         , 0x00000100);//MODE
			imi_mipi2_regs_write( MIPI_IPI_VCID         , 0x00000000);//VCID        //bit[1:0] :virtual channel
			imi_mipi2_regs_write( MIPI_IPI_DATA_TYPE    , MIPI_DAT_TYPE);//DATA_TYPE   //bit[5:0] :data type, 0x2c:raw12;
			imi_mipi2_regs_write( MIPI_IPI_HSA_TIME     , 0x0000001f);//HSA
			imi_mipi2_regs_write( MIPI_IPI_HBP_TIME     , 0x0000001f);//HBP
			imi_mipi2_regs_write( MIPI_IPI_HSD_TIME     , 0x0000000a);//HSD
			imi_mipi2_regs_write( MIPI_IPI_HLINE_TIME   , 0x00000650);//HLINE_TIME
			imi_mipi2_regs_write( MIPI_IPI_VSA_LINES    , 0x00000002);//VSA
			imi_mipi2_regs_write( MIPI_IPI_VBP_LINES    , 0x00000002);//VBP
			imi_mipi2_regs_write( MIPI_IPI_VFP_LINES    , 0x0000000f);//VFP
			imi_mipi2_regs_write( MIPI_IPI_VACTIVE_LINES, 0x00000078);//VACTIVE_LINE//
			imi_mipi2_regs_write( MIPI_N_LANES          , LANE_NUM - 1); //Lane enable, 0: 1 lane, 1: 2 lanes, 2: 3 lanes, 3: 4 lanes
			break;
		default:
			imi_mipi0_regs_write( MIPI_CSI2_RESET, 0x1       );//CSi2_RESET  //
			imi_mipi0_regs_write( MIPI_IPI_MODE         , 0x00000100);//MODE
			imi_mipi0_regs_write( MIPI_IPI_VCID         , 0x00000000);//VCID        //bit[1:0] :virtual channel
			imi_mipi0_regs_write( MIPI_IPI_DATA_TYPE    , MIPI_DAT_TYPE);//DATA_TYPE   //bit[5:0] :data type, 0x2c:raw12;
			imi_mipi0_regs_write( MIPI_IPI_HSA_TIME     , 0x0000001f);//HSA
			imi_mipi0_regs_write( MIPI_IPI_HBP_TIME     , 0x0000001f);//HBP
			imi_mipi0_regs_write( MIPI_IPI_HSD_TIME     , 0x0000000a);//HSD
			imi_mipi0_regs_write( MIPI_IPI_HLINE_TIME   , 0x00000650);//HLINE_TIME
			imi_mipi0_regs_write( MIPI_IPI_VSA_LINES    , 0x00000002);//VSA
			imi_mipi0_regs_write( MIPI_IPI_VBP_LINES    , 0x00000002);//VBP
			imi_mipi0_regs_write( MIPI_IPI_VFP_LINES    , 0x0000000f);//VFP
			imi_mipi0_regs_write( MIPI_IPI_VACTIVE_LINES, 0x00000078);//VACTIVE_LINE//
			imi_mipi0_regs_write( MIPI_N_LANES          , LANE_NUM - 1); //Lane enable, 0: 1 lane, 1: 2 lanes, 2: 3 lanes, 3: 4 lanes
			break;
	}
	MIPI_DBG("%s : Exit\n", __FUNCTION__);
	imi_resetdphy(idx, mode, freq, cfg_clk_freq);
}

void imi_test_pattern_mode(struct Imi_ImageConfig_s *p)
{
	unsigned int disable_isp = 0;
	unsigned int debug_sel = 0;
	unsigned int dpu0_sel  = 0; //0:dvp  0, 1:mipi 0; 2:mipi 1, 3:mipi 2
	unsigned int dpu1_sel  = 0; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp  1
	unsigned int dpu2_sel  = 0; //0:mipi 1, 1:mipi 2, 2:dvp  1, 3:dvp  2
	unsigned int isp_sel   = 1; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp 2
	unsigned int iftester  = 2;//4:glue tester, 2: RX tester, 1:TX tester
	unsigned int zcal_rstz = 1;
	unsigned int cfg_top_ctrl = ((disable_isp & 1)<<0) | ((debug_sel & 7)<<1) | ((dpu0_sel & 3)<<4) | ((dpu1_sel & 3)<<6) | ((dpu2_sel & 3)<<8) | ((isp_sel & 3)<<10);
	cfg_top_ctrl |= ((iftester & 7)<<12) | ((zcal_rstz & 1)<<15); //only used for FPGA test-chip
	imi_vi_regs_write( VI_TOP_CTRL, cfg_top_ctrl );  //[11:10]: 2-mipi2, ... ,0-mipi0 ;

	imi_vi_regs_write( VI_MIPI_OUT_EN, 0x3f);
	imi_vi_regs_write( VI_CSI_0_FRAME_SIZE, (p->height + (p->width << 16)));
	imi_vi_regs_write( VI_CSI_1_FRAME_SIZE, (p->height + (p->width << 16)));
	imi_vi_regs_write( VI_CSI_2_FRAME_SIZE, (p->height + (p->width << 16)));

	imi_mipi0_regs_write( 0x0, 0x01);
	imi_mipi0_regs_write( 0x8, ((p->height << 16) + p->width));
	imi_mipi0_regs_write( 0xc, 0x77);
	imi_mipi0_regs_write( 0x30, 0x1);
}

int imi_Mipi_To_Dvp(bool_t on, struct Imi_ImageConfig_s *p)
{
	MIPI_DBG("Mipi to dvp enter: %s, enable = %d\n",__FUNCTION__, on);
	if(on){
		unsigned int mipi_mode = DIRECT;
		unsigned int cfg_clk_freq=24;
		unsigned int disable_isp = 0;
		unsigned int debug_sel = 0;
		unsigned int dpu0_sel  = 0; //0:dvp  0, 1:mipi 0; 2:mipi 1, 3:mipi 2
		unsigned int dpu1_sel  = 0; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp  1
		unsigned int dpu2_sel  = 0; //0:mipi 1, 1:mipi 2, 2:dvp  1, 3:dvp  2
		unsigned int isp_sel   = 0; //0:mipi 0, 1:mipi 1, 2:mipi 2, 3:dvp 2
		unsigned int iftester  = 2;//4:glue tester, 2: RX tester, 1:TX tester
		unsigned int zcal_rstz = 1;
		unsigned int cfg_top_ctrl = ((disable_isp & 1)<<0) | ((debug_sel & 7)<<1) | ((dpu0_sel & 3)<<4) | ((dpu1_sel & 3)<<6) | ((dpu2_sel & 3)<<8) | ((isp_sel & 3)<<10);
		cfg_top_ctrl |= ((iftester & 7)<<12) | ((zcal_rstz & 1)<<15); //only used for FPGA test-chip

		MIPI_DBG("%s, mipi_height%d x mipi_width %d\n",__FUNCTION__, p->height, p->width);
		imi_vi_regs_write(VI_TOP_CTRL, cfg_top_ctrl);  //[11:10]: 2-mipi2, ... ,0-mipi0 ;
		imi_vi_regs_write(VI_MIPI_OUT_EN, 0x3f);
		imi_vi_regs_write(VI_CSI_0_FRAME_SIZE, (p->height + (p->width << 16)));//31:16 Width 15:0 Height, //0x1000120);
		imi_vi_regs_write(VI_CSI_1_FRAME_SIZE, (p->height + (p->width << 16)));//31:16 Width 15:0 Height, //0x1000120);
		imi_vi_regs_write(VI_CSI_2_FRAME_SIZE, (p->height + (p->width << 16)));//31:16 Width 15:0 Height, //0x1000120);
		imi_vi_regs_write(0x4, 0xa0);

		MIPI_DBG("%s, mipi_clk = %d\n",__FUNCTION__, p->clk);
		//mipi config   real sensor
		imi_rx_dphy_test(0, mipi_mode, p->clk, cfg_clk_freq);
		MIPI_DBG("Mipi to dvp dphy test\n");

	}
	MIPI_DBG("Mipi to dvp exit: %s\n",__FUNCTION__);

	return 0;
}

RESULT Imi_MipitoDvpGetIss(ImiMipiToDvpIss_t *pImiMipiToDvpIss)
{
	RESULT result = RET_SUCCESS;
	if (pImiMipiToDvpIss != NULL) {
		MIPI_DBG("Mipi to dvp get iss enter: %s\n",__FUNCTION__);
		pImiMipiToDvpIss->pImi_Mipi_To_Dvp = imi_Mipi_To_Dvp;
		pImiMipiToDvpIss->pImi_GetMipiRegVaddr = imi_GetMipiRegVaddr;
	} else {
		result = RET_NULL_POINTER;
		MIPI_ERR("Mipi to dvp get iss ERR: %s\n",__FUNCTION__);
	}

	return result;
}

ImiMipiDrvConfig_t ImiMipiDrvConfig = {
	Imi_MipitoDvpGetIss,
	{
		imi_Mipi_To_Dvp,
		imi_GetMipiRegVaddr,
	},
	{
		0,
		0,
		0,
	}
};


