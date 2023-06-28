#include <lcom/lcf.h>

#include <machine/int86.h> // /usr/src/include/arch/i386


void *video_mem; /* frame-buffer VM address */
vbe_mode_info_t vmi_p;
int bytes_per_pixel;

int width = 0;
int height = 0;

int vg_mode(uint16_t mode) {

  reg86_t r;

  memset(&r, 0, sizeof(r));

  r.ax = 0x4F02;         // VBE call, function 02 -- set VBE mode
  r.bx = 1 << 14 | mode; // set bit 14: linear framebuffer
  r.intno = 0x10;
  if (sys_int86(&r) != OK) {
    printf("set_vbe_mode: sys_int86() failed \n");
    return 1;
  }
  return 0;
}

void vg_init1(uint16_t mode) {

  vbe_get_mode_info(mode, &vmi_p);

  width = vmi_p.XResolution;
  height = vmi_p.YResolution;

  bytes_per_pixel = (vmi_p.BitsPerPixel+7)/8;

  int r;
  struct minix_mem_range mr; /* physical memory range */
  unsigned int vram_base = vmi_p.PhysBasePtr;    /* VRAM’s physical addresss */
  unsigned int vram_size = bytes_per_pixel * vmi_p.XResolution * vmi_p.YResolution;    /* VRAM’s size, but you can use the frame-buffer size, instead */

  /* Allow memory mapping */
  mr.mr_base = (phys_bytes) vram_base;
  mr.mr_limit = mr.mr_base + vram_size;
  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);
  /* Map memory */
  video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);
  if (video_mem == MAP_FAILED)
    panic("couldn’t map video memory");

  vg_mode(mode);

}

int draw_pixel(uint16_t x, uint16_t y, uint32_t color){

  char * adress = (char *) video_mem + (vmi_p.XResolution * y + x)*bytes_per_pixel;

  memcpy(adress, &color, bytes_per_pixel);
  return 0;
}

int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){

  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      draw_pixel(x+j, y+i, color);
    }
    
  }
  return 0;
}

/*int get_color(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step, int row, int col, uint32_t * color){

  if (mode == 0x105){
    color = (first + (row * no_rectangles + col) * step) % (1 << vmi_p.BitsPerPixel);
  }
  else{

  }

}*/

uint8_t* loadXPM(xpm_map_t xpm, enum xpm_image_type type, xpm_image_t *img) {
  return xpm_load(xpm,type,img);
}

void print_xpm(uint16_t x, uint16_t y,  uint8_t* map, xpm_image_t *img){
  for(size_t i=0; i<img->height; i++){
      //memset(buffer + (x+(y+i)*h_res)*bytes_per_pixel,*(map+(i*img.width)*bytes_per_pixel),img.width*bytes_per_pixel);
      for(size_t j=0; j<img->width; j++){
        draw_pixel(x+j, y+i,*(map + (j+i*img->width)*bytes_per_pixel));
      }
  }
}

void draw_background(char const *xpm[]){

  uint32_t invColor = xpm_transparency_color(XPM_8_8_8);

  xpm_image_t backgroundImg;
  uint8_t *backgroundMap;
  backgroundMap = xpm_load(xpm, XPM_8_8_8, &backgroundImg);

  unsigned int img_height = backgroundImg.height;
  unsigned int img_width = backgroundImg.width;

  for (unsigned int h = 0; h < img_height; h++) {
    for (unsigned int w = 0; w < img_width; w++) {
      uint32_t c = *((uint32_t *) (backgroundMap + (w + h * backgroundImg.width) * 3));
      if (c != invColor) {
        draw_pixel(w, h, c);
      }
    }
  }
}

void draw_object(uint16_t x, uint16_t y, uint8_t *ballMap, xpm_image_t ballImg){

  uint32_t invColor = xpm_transparency_color(XPM_8_8_8);

  for (unsigned int h = 0; h < ballImg.height; h++) {
    for (unsigned int w = 0; w < ballImg.width; w++) {
      uint32_t c = *((uint32_t *) (ballMap + (w + h * ballImg.width) * 3));
      c &= 0x00ffffff;
      if (c != invColor) {
        draw_pixel(w+x, h+y, c);
      }
    }
  }
}

void draw_hline(uint16_t x, uint16_t y, uint16_t width, uint32_t color){

  for (int i = 0; i < width; i++)
  {
    draw_pixel(x + i, y, color);
  }
  
}

void draw_vline(uint16_t x, uint16_t y, uint16_t height, uint32_t color){

  for (int i = 0; i < height; i++)
  {
    draw_pixel(x, y + i, color);
  }
  
}

void draw_border(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){

  draw_hline(x, y, width, color);
  draw_vline(x, y, height, color);
  draw_hline(x, y + height, width, color);
  draw_vline(x + width, y, height, color);
}

void draw_black(){
  draw_rectangle(20,0,width-40, height, 0);
}
