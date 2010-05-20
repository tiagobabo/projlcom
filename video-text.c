#include "video-text.h"

/** Calculate memory offset for writing at position x, y
*/
ulong calcMemPosition(int x, int y)
{
	return VIDEO_TEXT + y * (WIDTH * 2) + x * 2;
}


int getCharAt(int x, int y)
{
	if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return -1;
	
	_farsetsel(_dos_ds);
	return _farnspeekb(calcMemPosition(x, y));
}


int getAttributesAt(int x, int y)
{
	if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return -1;
	
	_farsetsel(_dos_ds);
	return _farnspeekb(calcMemPosition(x, y) + 1);
}


/** Print character ch at position x,y with attributes
* return true if sucessfull
*/
Bool printCharAt(char ch, int x, int y, char attributes)
{
	if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
	
	_farsetsel(_dos_ds);
	_farnspokeb(calcMemPosition(x, y), ch);
	_farnspokeb(calcMemPosition(x, y) + 1, attributes);
	return true;
}


/** Clear screen with background color
*/
void fillScreen(char background)
{
	int i, j;
	for(i = 0; i < WIDTH; i++)
		for(j = 0; j < HEIGHT; j++)
			printCharAt(0, i, j, background);
}


//print line
void printLineAt(char ch, int xi, int yi, int xf, int yf, char attributes)
{
	//obter o comprimento da linha em x
	int x_size = xf - xi;
	if(x_size < 0) x_size = -x_size;
	
	//obter o comprimento da linha em y
	int y_size = yf - yi;
	if(y_size < 0) y_size = -y_size;
	
	int i;
	if(x_size > y_size) //dimensao maior e a horizontal
	{
		if(xf - xi < 0) //orientar a recta no sentido positivo de x
		{
			int aux = xi;
			xi = xf;
			xf = aux;
			aux = yi;
			yi = yf;
			yf = aux;
		}
		
		for(i = 0; i <= x_size; i++)
		{
			//regra de 3 simples; se x_size esta para y_size, entao i esta para y_point
			double y_point = (double)y_size * (double)i / (double)x_size;
			
			//desenhar pixel
			if(yf - yi < 0) printCharAt(ch, xi + i, yi - (int)(y_point + 0.5), attributes);
			else printCharAt(ch, xi + i, yi + (int)(y_point + 0.5), attributes);
		}
	}
		
	else //dimensao maior e a vertical
	{
		if(yf - yi < 0) //orientar a recta no sentido positivo de y
		{
			int aux = xi;
			xi = xf;
			xf = aux;
			aux = yi;
			yi = yf;
			yf = aux;
		}
		
		for(i = 0; i <= y_size; i++)
		{
			//regra de 3 simples; se y_size esta para x_size, entao i esta para x_point
			double x_point = (double)x_size * (double)i / (double)y_size;
			
			//desenhar pixel
			if(xf - xi < 0) printCharAt(ch, xi - (int)(x_point + 0.5), yi + i, attributes);
			else printCharAt(ch, xi + (int)(x_point + 0.5), yi + i, attributes);
		}
	}		
}


/** Print character array str at position x,y with attributes
* return true if sucessfull
*/
Bool printStringAt(char *str, int x, int y, char attributes)
{
	int i = 0;
	while(str[i] != 0)
	{
		if(!printCharAt(str[i], x + i, y, attributes)) return false;
		i++;
	}
		
	return true;
}


/* print character array str at position x, y with attributes
if n_char >= 0, it will print exactly n_char characters
returns the number of chars printed
*/
int printSizedStringAt(char *str, int n_char, int x, int y, char attributes)
{
	Bool has_size = (n_char >= 0);
	int count = 0;
	
	while(str[count] != 0)
	{
		printCharAt(str[count], x + count, y, attributes);
		count++;
		if(has_size && count >= n_char) break;
	}
	
	if(has_size)
	{
		while(count < n_char)
		{
			printCharAt(0, x + count, y, attributes);
			count++;
		}
	}
	
	return count;
}


/** Print integer num at position x,y with attributes.
* No C library function is used.
* return true if sucessfull
*/
Bool printIntAt(int num, int x, int y, char attributes)
{
	int i = 0;
	
	//se o numero for negativo, imprimir '-'
	if(num < 0)
	{
		if(!printCharAt('-', x, y, attributes)) return false;
		i++;
		num = 0 - num;
	}
	
	//descobrir a ordem de grandeza do numero
	int ordem = 1;
	while(num / ordem >= 10) ordem *= 10;
	
	//da esquerda para a direita, imprimir os algarismos
	while(1)
	{
		int digito = num / ordem;
		if(!printCharAt(digito + 0x30, x + i, y, attributes)) return false;
		num -= digito * ordem;
		if(ordem == 1) break;
		ordem /= 10;
		i++;
	}

	return true;
}


/** Draw rectangular frame with left upper corner at position x,y
* with width and height dimensions, decorated with title attributes.
* return true if sucessfull
*/
Bool drawFrame(char *title, char attributes, int x, int y, int width, int height)
{
	int x_fin = x + width - 1;
	int y_fin = y + height - 1;
	
	if(!printCharAt(0xC9, x, y, attributes)) return false;			//canto superior esquerdo
	if(!printCharAt(0xBB, x_fin, y, attributes)) return false;		//canto superior direito
	if(!printCharAt(0xC8, x, y_fin, attributes)) return false;		//canto inferior esquerdo
	if(!printCharAt(0xBC, x_fin, y_fin, attributes)) return false;	//canto inferior direito
	
	int i = x + 1;
	for( ; i < x_fin; i++)
	{
		printCharAt(0xCD, i, y, attributes);						//margem superior
		printCharAt(0xCD, i, y_fin, attributes);					//margem inferior	
	}
	
	i = y + 1;
	for( ; i < y_fin; i++)
	{
		printCharAt(0xBA, x, i, attributes);						//margem esquerda
		printCharAt(0xBA, x_fin, i, attributes);					//margem direita
	}
	
	//verificar se o titulo cabe na moldura
	int title_size = 0;
	while(title[title_size] != 0) title_size++;
	if(title_size > width - 2) return false;
	
	printStringAt(title, x + 1, y + 1, attributes);					//titulo
	return true;
}


Bool drawTable(char attributes, int x, int y, int n_lin, int n_col)
{
	int x_fin = x + n_lin * 2;
	int y_fin = y + n_col * 2;
	
	if(!printCharAt(0xC9, x, y, attributes)) return false;			//canto superior esquerdo
	if(!printCharAt(0xBB, x_fin, y, attributes)) return false;		//canto superior direito
	if(!printCharAt(0xC8, x, y_fin, attributes)) return false;		//canto inferior esquerdo
	if(!printCharAt(0xBC, x_fin, y_fin, attributes)) return false;	//canto inferior direito
	
	int i, j;
	for(j = y + 1; j < y_fin; j += 2)
		for(i = x + 1; i < x_fin; i += 2)
			printCharAt(0, i, j, attributes); // <espaco>
			
	for(j = y; j <= y_fin; j += 2)
		for(i = x + 1; i <= x_fin; i += 2)
			printCharAt(0xCD, i, j, attributes); // =
			
	for(j = y + 1; j <= y_fin; j += 2)
		for(i = x; i <= x_fin; i += 2)
			printCharAt(0xBA, i, j, attributes); // ||
	
	for(j = y + 2; j < y_fin; j += 2)
		for(i = x + 2; i < x_fin; i += 2)
			printCharAt(0xCE, i, j, attributes); // -|-
	
	for(j = y + 2; j < y_fin; j += 2)
		printCharAt(0xCC, x, j, attributes); //  |-
		
	for(j = y + 2; j < y_fin; j += 2)
		printCharAt(0xB9, x_fin, j, attributes); //  -|
	
	for(i = x + 2; i < x_fin; i += 2)
		printCharAt(0xCB, i, y, attributes); //  t

	for(i = x + 2; i < x_fin; i += 2)
		printCharAt(0xCA, i, y_fin, attributes); // _
	
	return true;
}
