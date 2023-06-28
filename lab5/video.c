#include <lcom/lcf.h>

#include <machine/int86.h>

void *video_mem; /* frame-buffer VM address */
vbe_mode_info_t vmi_p;
int bytes_per_pixel;

int vg_mode(uint16_t mode) {

  reg86_t reg86;
  memset(&reg86, 0, sizeof(reg86));

  reg86.intno = 0x10;
  reg86.bx = BIT(14) | mode;
  reg86.ah = 0x4F;
  reg86.al = 0x02;

  if (sys_int86(&reg86)) {
    printf("set_vbe_mode: sys_int86() failed \n");
    return 1;
  }

  return 0;
}

void vg_init1(uint16_t mode) {

  vbe_get_mode_info(mode, &vmi_p);

  bytes_per_pixel = (vmi_p.BitsPerPixel + 7) / 8;

  int r;
  struct minix_mem_range mr;                                                        /* physical memory range */
  unsigned int vram_base = vmi_p.PhysBasePtr;                                       /* VRAM’s physical addresss */
  unsigned int vram_size = bytes_per_pixel * vmi_p.XResolution * vmi_p.YResolution; /* VRAM’s size, but you can use the frame-buffer size, instead */

  /* Allow memory mapping */
  mr.mr_base = (phys_bytes) vram_base;
  mr.mr_limit = mr.mr_base + vram_size;
  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);

  // Map memory
  video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);
  if (video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  vg_mode(mode);
}

int draw_pixel(uint16_t x, uint16_t y, uint32_t color) {

  char *adress = (char *) video_mem + (vmi_p.XResolution * y + x) * bytes_per_pixel;

  memcpy(adress, &color, bytes_per_pixel);
  return 0;
}

int(vg_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  for (unsigned int i = 0; i < len; i++) {
    if (draw_pixel(x + i, y, color) != 0) {
      return 1;
    }
  }
  return 0;
}

int(vg_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  for (unsigned int i = 0; i < height; i++) {
    if (vg_hline(x, y + i, width, color) != 0) {
      return 1;
    }
  }
  return 0;
}

uint32_t(R_First)(uint32_t first) {
  return (first >> vmi_p.RedFieldPosition) & ((1 << vmi_p.RedMaskSize) - 1);
}

uint32_t(R)(unsigned int w, uint8_t step, uint8_t first) {
  return (R_First(first) + w * step) % (1 << vmi_p.RedMaskSize);
}

uint32_t(G_First)(uint32_t first) {
  return (first >> vmi_p.GreenFieldPosition) & ((1 << vmi_p.GreenMaskSize) - 1);
}

uint32_t(G)(unsigned int w, uint8_t step, uint8_t first) {
  return (R_First(first) + w * step) % (1 << vmi_p.GreenMaskSize);
}

uint32_t(B_First)(uint32_t first) {
  return (first >> vmi_p.BlueFieldPosition) & ((1 << vmi_p.BlueMaskSize) - 1);
}

uint32_t(B)(unsigned int w, unsigned int h, uint8_t step, uint8_t first) {
  return (R_First(first) + w * step) % (1 << vmi_p.BlueMaskSize);
}

uint32_t(indexed_color)(uint16_t row, uint16_t col, uint8_t step, uint32_t first, uint8_t no_rectangles) {

  return (first + (row * no_rectangles + col) * step) % (1 << bytes_per_pixel);
}

uint32_t(direct_color)(uint32_t red, uint32_t green, uint32_t blue) {

  return blue | (green << vmi_p.GreenFieldPosition) | (red << vmi_p.RedFieldPosition);
}
