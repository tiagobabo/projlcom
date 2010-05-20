#include "video.h"

unsigned long getMemAddr(int mode) {
	__dpmi_regs regs;
	unsigned long dosbuf, address=0;
	dosbuf = __tb & 0xFFFFF; /* dosbuf ´e a ´area de transfer^encia */
	dosmemput(&address, 4, dosbuf+40); /* apaga bytes 40 a 43 dessa ´area*/
	regs.x.ax = 0x4F01; /* VBE get mode info */
	regs.x.di = dosbuf & 0xF; /* LSB da ´area de transfer^encia */
	regs.x.es = (dosbuf >> 4) & 0xFFFF; /* MSB da ´area de transfer^encia */
	regs.x.cx = mode; /* informa¸c~ao sobre o modo 105h */
	__dpmi_int(0x10, &regs); /* DOS int 10h */
	dosmemget(dosbuf+40, 4, &address); /* l^e o endere¸co da mem´oria v´ideo */
	return address;
}

char * enter_graphics(int mode, __dpmi_meminfo *map) {	
	char* video_mem;

	__djgpp_nearptr_enable();
	map->address = getMemAddr(mode);
	switch (mode) {
		case 0x4101:
			map->size=640*480;
			HRES=640;
			VRES=480;
			break;
		case 0x4103:
			map->size=800*600;
			HRES=800;
			VRES=600;
			break;
		case 0x4107:
			map->size=1280*1024;
			HRES=1280;
			VRES=1024;
			break;
		default:
			mode=0x4105;
			map->size=1024*768;
			HRES=1024;
			VRES=768;
			break;
	}
	__dpmi_physical_address_mapping(map);
	video_mem = (char*)(map->address + __djgpp_conventional_base);


	__dpmi_regs regs;
	regs.x.ax = 0x4F02;
	regs.x.bx = mode;
	__dpmi_int( 0x10, &regs );
	return video_mem;
}

void leave_graphics(__dpmi_meminfo *map) {
	__dpmi_free_physical_address_mapping(map); /* liberta mapeamento */
	__djgpp_nearptr_disable();

	__dpmi_regs regs;
	regs.x.ax = 0x0003; /* registo AX do CPU com valor 0x03 */
	__dpmi_int(0x10, &regs); /* gera interrup¸c~ao software 0x10, entrando no modo texto */
}

void set_pixel(int x, int y, int color, char *base) {

	if( x < HRES && y < VRES && x >= 0 && y >= 0 )
	{
	char *p= base+x+y*HRES;
	*p=color;
	}
}

int get_pixel(int x, int y, char *base) {
	
	if( x < HRES && y < VRES && x >= 0 && y >= 0 )
	return *(base+x+y*HRES);
	else
	return -1;
}

void clear_screen(char color, char *base){
	int i, j;
	
	for( i = 0; i < HRES; ++i )
		for( j = 0; j < VRES; ++j )
		 set_pixel(i, j, color, base);
}

void draw_line( int xi, int yi, int xf, int yf, int color, char *base ) {
	double restoX = 0.0, restoY = 0.0;

	while( sqrt( pow( yf - yi, 2 ) + pow( xf - xi, 2 ) ) >= 1 ) {
		if( restoX >= 1 ) { restoX--; xi++; }
		if( restoY >= 1 ) { restoY--; yi++; }
		if( restoX <= -1 ) { restoX++; xi--; }
		if( restoY <= -1 ) { restoY++; yi--; }

		set_pixel( xi, yi, color, base );
		
		double vecX = ( xf - xi );
		double vecY = ( yf - yi );
		double vecXN = vecX / sqrt( pow( vecX, 2 ) + pow( vecY, 2 ) );
		double vecYN = vecY / sqrt( pow( vecX, 2 ) + pow( vecY, 2 ) );

		restoX += vecXN - (int)vecXN;
		restoY += vecYN - (int)vecYN;

		xi += ceil(vecXN);
		yi += ceil(vecYN);
	}

	set_pixel( xf, yf, color, base );
}

