/* ************************************************************************* */
/* Organizacion del Computador II                                            */
/*                                                                           */
/*             Biblioteca de funciones para operar imagenes BMP              */
/*                                                                           */
/*   Esta biblioteca permite crear, abrir, modificar y guardar archivos en   */
/*   formato bmp de forma sencilla. Soporta solamente archivos con header de */
/*   versiones info_header (40 bytes) y info_v5_header (124 bytes). Para la  */
/*   primera imagenes de 24 bits (BGR) y la segunda imagenes de 32 (ABGR).   */
/*                                                                           */
/*   bmp.h : headers de la biblioteca                                        */
/*   bmp.c : codigo fuente de la biblioteca                                  */
/*   example.c : ejemplos de uso de la biblioteca                            */
/*               $ gcc example.c bmp.c -o example                            */
/* ************************************************************************* */

#include "bmp.h"


void draw_diff(int size, float* m, char* ref_file_name, char* file_name){
		int i, j;
		BMPV5H* imgh = get_BMPV5H(size,size);
		BMP* bmp = bmp_create(imgh,0);
		BMP* bmp2 = bmp_read(ref_file_name);

		uint8_t* data = bmp_get_data(bmp);
		uint8_t* data2 = bmp_get_data(bmp2);
		float max_value = 0;
		int max_found = 0;
		float min_value = 0;
		int min_found = 0;
		float curr_value;
		for(j=0;j<size;j++) {
			for(i=0;i<size;i++) {
				curr_value = m[((i)+(size+2)*(j))];
				if(!min_found || curr_value < min_value){
					min_found = 1;
					min_value = curr_value;
				}
				if(!max_found || curr_value > max_value){
					max_found = 1;
					max_value = curr_value;
				}	
			}
		}

		float range = max_value - min_value;

		uint8_t curr_pixel;

		for(j=0;j<size;j++) {
			for(i=0;i<size;i++) {
				curr_value = (m[((i)+(size+2)*(j))] - min_value) / range;
				curr_pixel = abs((uint8_t)(curr_value*(255.0)) - data2[j*size*4+i*4+1]);
				data[j*size*4+i*4+0] = 0xff;
				data[j*size*4+i*4+1] = curr_pixel;
				data[j*size*4+i*4+2] = curr_pixel;
				data[j*size*4+i*4+3] = curr_pixel;
			}
		}
		printf("Terminó de cargar datos de %s de tamaño %d\n", file_name, size*size);
		int r = bmp_save(file_name, bmp);
		if(r==0)
			printf("Error al escribir el archivo\n");
		bmp_delete(bmp);
		bmp_delete(bmp2);
}

void draw_alpha(int size, float* m, char* file_name){
		int i, j;
		BMPV5H* imgh = get_BMPV5H(size,size);
		BMP* bmp = bmp_create(imgh,0);

		uint8_t* data = bmp_get_data(bmp);
		float max_value = 0;
		int max_found = 0;
		float min_value = 0;
		int min_found = 0;
		float curr_value;
		for(j=0;j<size;j++) {
			for(i=0;i<size;i++) {
				curr_value = m[((i)+(size+2)*(j))];
				if(!min_found || curr_value < min_value){
					min_found = 1;
					min_value = curr_value;
				}
				if(!max_found || curr_value > max_value){
					max_found = 1;
					max_value = curr_value;
				}	
			}
		}

		float range = max_value - min_value;
		uint8_t curr_pixel;

		for(j=0;j<size;j++) {
			for(i=0;i<size;i++) {
				curr_value = (m[((i)+(size+2)*(j))] - min_value) / range;
				curr_pixel = (uint8_t)(curr_value*(255.0));
				data[j*size*4+i*4+0] = 0xff;
				data[j*size*4+i*4+1] = curr_pixel;
				data[j*size*4+i*4+2] = curr_pixel;
				data[j*size*4+i*4+3] = curr_pixel;
			}
		}
		printf("Terminó de cargar datos de %s de tamaño %d\n", file_name, size*size);
		int r = bmp_save(file_name, bmp);
		if(r==0)
			printf("Error al escribir el archivo\n");
		bmp_delete(bmp);
}

/* ************************************************************************* */
BMPIH* get_BMPIH(uint32_t width, uint32_t height) {
  if(width%4!=0) return 0; // TODO: dont support padding
  BMPIH* new_bmp_info_ih = (BMPIH*)malloc(sizeof(BMPIH));
  new_bmp_info_ih->biSize   = sizeof(BMPIH);
  new_bmp_info_ih->biWidth  = width;
  new_bmp_info_ih->biHeight = height;
  new_bmp_info_ih->biPlanes = 1;
  new_bmp_info_ih->biBitCount = 24; //TODO: dont support other bitcount
  new_bmp_info_ih->biCompression = BI_RGB;
  new_bmp_info_ih->biSizeImage = width*height*(new_bmp_info_ih->biBitCount/8);
  new_bmp_info_ih->biXPelsPerMeter = 2952; // 75 dpi
  new_bmp_info_ih->biYPelsPerMeter = 2952; // 75 dpi
  new_bmp_info_ih->biClrUsed = 0;
  new_bmp_info_ih->biClrImportant = 0;
  return new_bmp_info_ih;
}

/* ************************************************************************* */
BMPV5H* get_BMPV5H(uint32_t width, uint32_t height) {
  if(width%4!=0) return 0; // TODO: dont support padding
  BMPV5H* new_bmp_info_v5h = (BMPV5H*)malloc(sizeof(BMPV5H));
  new_bmp_info_v5h->bV5Size   = sizeof(BMPV5H);
  new_bmp_info_v5h->bV5Width  = width;
  new_bmp_info_v5h->bV5Height = height;
  new_bmp_info_v5h->bV5Planes = 1;
  new_bmp_info_v5h->bV5BitCount = 32; //TODO: dont support other bitcount
  new_bmp_info_v5h->bV5Compression = BI_BITFIELDS;
  new_bmp_info_v5h->bV5SizeImage = width*height*(new_bmp_info_v5h->bV5BitCount/8);
  new_bmp_info_v5h->bV5XPelsPerMeter = 2952; // 75 dpi
  new_bmp_info_v5h->bV5YPelsPerMeter = 2952; // 75 dpi
  new_bmp_info_v5h->bV5ClrUsed      = 0;
  new_bmp_info_v5h->bV5ClrImportant = 0;
  new_bmp_info_v5h->bV5RedMask   = 0xff000000;
  new_bmp_info_v5h->bV5GreenMask = 0x00ff0000;
  new_bmp_info_v5h->bV5BlueMask  = 0x0000ff00;
  new_bmp_info_v5h->bV5AlphaMask = 0x000000ff;
  new_bmp_info_v5h->bV5CSType = LCS_sRGB;
  CIEXYZTRIPLE bV5Endpoints_ = {{0,0,0},{0,0,0},{0,0,0}};
  new_bmp_info_v5h->bV5Endpoints = bV5Endpoints_;
  new_bmp_info_v5h->bV5GammaRed   = 0;
  new_bmp_info_v5h->bV5GammaGreen = 0;
  new_bmp_info_v5h->bV5GammaBlue  = 0;
  new_bmp_info_v5h->bV5Intent = LCS_GM_GRAPHICS;
  new_bmp_info_v5h->bV5ProfileData = 0;
  new_bmp_info_v5h->bV5ProfileSize = 0;
  new_bmp_info_v5h->bV5Reserved = 0;
  return new_bmp_info_v5h;
}

/* ************************************************************************* */
BMP* bmp_create(void* info_header, int init_data) {
  unsigned int i;
  BMPIH* ih = (BMPIH*)info_header;
 
  // (1) creo la data area
  unsigned int data_size = ih->biSizeImage;
  uint8_t* new_bmp_data = (uint8_t*)malloc(data_size);
  if(init_data)
    for(i=0;i<data_size;i++)
      new_bmp_data[i] = 0;
  
  // (2) creo un nuevo fh
  BMPFH* new_bmp_fh = (BMPFH*) malloc(sizeof(BMPFH));
  new_bmp_fh->bfType[0] = 'B';
  new_bmp_fh->bfType[1] = 'M';
  new_bmp_fh->bfOffBits = ih->biSize + sizeof(BMPFH);  
  new_bmp_fh->bfSize = data_size + new_bmp_fh->bfOffBits;
  new_bmp_fh->bfReserved1 = 0;
  new_bmp_fh->bfReserved2 = 0;

  // (3) store information on a BMP struct
  BMP* bmp = (BMP*)malloc(sizeof(BMP));
  bmp->fh = new_bmp_fh;
  bmp->ih = info_header;
  bmp->data = new_bmp_data;
  
  return bmp;
}

/* ************************************************************************* */
BMP* bmp_copy(BMP* img, int copy_data) {
  unsigned int i;
  
  // (1) copy the fh
  BMPFH* new_bmp_fh = (BMPFH*) malloc(sizeof(BMPFH));
  (*new_bmp_fh) = (*(img->fh));
  
  // (2) copy the ih/v5h
  int info_header_size = new_bmp_fh->bfOffBits - sizeof(BMPFH);
  char* new_bmp_info = 0;
  if( info_header_size == sizeof(BMPIH) ) {
    BMPIH* new_bmp_info_ih = (BMPIH*)malloc(sizeof(BMPIH));
    (*new_bmp_info_ih) = *(BMPIH*)(img->ih);
    new_bmp_info = (char*)new_bmp_info_ih;
  } else
  if( info_header_size == sizeof(BMPV5H) ) {
    BMPV5H* new_bmp_info_v5h = (BMPV5H*)malloc(sizeof(BMPV5H));
    (*new_bmp_info_v5h) = *(BMPV5H*)(img->ih);
    new_bmp_info = (char*)new_bmp_info_v5h;
  }
  
  // (3) data area
  unsigned int data_size = ((BMPIH*)(new_bmp_info))->biSizeImage;
  uint8_t* new_bmp_data = (uint8_t*)malloc(data_size);
  if( copy_data )
    for(i=0;i<data_size;i++)
      new_bmp_data[i] = img->data[i];
  
  // (4) store information on a BMP struct
  BMP* bmp = (BMP*)malloc(sizeof(BMP));
  bmp->fh = new_bmp_fh;
  bmp->ih = new_bmp_info;
  bmp->data = new_bmp_data;
  
  return bmp;
}

/* ************************************************************************* */
BMP* bmp_read(char* src) {
  
  // (0) open file
  FILE* fsrc = fopen(src,"r");
  if(fsrc == 0){ return 0; } // Error al abrir el archivo 

  // (1) read bitmap file header
  BMPFH* bmp_fh = (BMPFH*) malloc(sizeof(BMPFH));  
  if(!fread(bmp_fh, sizeof(BMPFH), 1, fsrc)){ return 0; } // Error al leer el archivo 

  // (2) read bitmap info header (TODO: only support BMPV5H(130B) and BMPIH(40B))
  int info_header_size = bmp_fh->bfOffBits - sizeof(BMPFH);
  char* bmp_info = 0;
  if( info_header_size == sizeof(BMPIH) ) {
    bmp_info = malloc(sizeof(BMPIH));
  } else
  if( info_header_size == sizeof(BMPV5H) ) {
    bmp_info = malloc(sizeof(BMPV5H));
  }
  if(!bmp_info){ return 0; } // Error formato no soportado 
  if(!fread(bmp_info, info_header_size, 1, fsrc)){ return 0; } // Error al leer el archivo

  // (3) read bitmap data pixels
  int image_data_size = bmp_fh->bfSize - info_header_size - sizeof(BMPFH);
  uint8_t* bmp_data = (uint8_t*) malloc(image_data_size);
  if(!fread(bmp_data, image_data_size, 1, fsrc)){ return 0; } // Error al leer el archivo

  // (4) store information on a BMP struct
  BMP* bmp = (BMP*)malloc(sizeof(BMP));
  bmp->fh = bmp_fh;
  bmp->ih = bmp_info;
  bmp->data = bmp_data;
  
  fclose(fsrc);
  
  return bmp;
}

/* ************************************************************************* */
int bmp_save(char* dst, BMP* img) {
  int r=0,b;
  
  // (0) open file
  FILE* fdst = fopen (dst,"w+");
  if(fdst == 0){ return 0; } // Error al abrir el archivo 
  
  // (1) write bitmap file header
  b=fwrite(img->fh, sizeof(BMPFH), 1, fdst); r=b;
  if(!b){ return 0; } // Error al escribir el archivo

  // (2) write bitmap info header
  b=fwrite(img->ih, img->fh->bfOffBits-sizeof(BMPFH), 1, fdst); r=r+b;
  if(!b){ return 0; } // Error al escribir el archivo
  
  // (3) write bitmap data
  b=fwrite(img->data, img->fh->bfSize-img->fh->bfOffBits, 1, fdst); r=r+b;
  if(!b){ return 0; } // Error al escribir el archivo
  
  fclose(fdst);
  
  return r;
}

/* ************************************************************************* */
void bmp_delete(BMP* img) {
  free(img->fh);
  free(img->ih);
  free(img->data);
  free(img);
}

/* ************************************************************************* */
uint32_t* bmp_get_w(BMP* img) {
  return &(((BMPIH*)(img->ih))->biWidth);
}

/* ************************************************************************* */
uint32_t* bmp_get_h(BMP* img) {
  return &(((BMPIH*)(img->ih))->biHeight);
}

/* ************************************************************************* */
uint8_t* bmp_get_data(BMP* img) {
  return img->data;
}
/* ************************************************************************* */
uint16_t* bmp_get_bitcount(BMP* img) {
  return &(((BMPIH*)(img->ih))->biBitCount);
}
/* ************************************************************************* */
void bmp_resize(BMP* img, uint32_t w, uint32_t h, int resize_data) {  
  ((BMPIH*)(img->ih))->biWidth = w;
  ((BMPIH*)(img->ih))->biHeight = h; 
  ((BMPIH*)(img->ih))->biSizeImage = w*h*((((BMPIH*)(img->ih))->biBitCount)/8);
  img->fh->bfSize = ((BMPIH*)(img->ih))->biSizeImage + img->fh->bfOffBits;
  if( resize_data ) {
      free(img->data);
      img->data = malloc(((BMPIH*)(img->ih))->biSizeImage);
  }
}
