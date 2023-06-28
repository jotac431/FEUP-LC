#include <i8042.h>
#include <keyboard.h>
#include <lcom/lcf.h>

int hook_id = KB_IRQ;
uint8_t scancode = 0;
bool two_byte;

void(kbc_ih)(void) {
  uint8_t st;

  // read the status register
  util_sys_inb(STAT_REG, &st);

  // read scancode
  if (st & STAT_REG_OBF) {
    if ((st & (STAT_REG_PAR | STAT_REG_TIM)) == 0) {
      util_sys_inb(OUT_BUF, &scancode);
    }
  }
}

int(kbc_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(hook_id);
  if (sys_irqsetpolicy(KB_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id))
    return 1;
  return 0;
}

int(kbc_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_id))
    return 1;
  return 0;
}

bool is_make() {

  uint8_t mask = 0x80;
  return (scancode & mask) == 0;
}

bool is_two_byte() {
  return scancode == TWO_BYTE_SCANCODE;
}

int enable_interrupts() {
	uint8_t cmd_byte;

	if (kbc_write_command(KBC_READ_COM)) { 	// Sends Read Command
		return 1;
	}

	if (kbc_read_out_buf(&cmd_byte)) {			// Reads Command Byte
		return 1;
	}

	cmd_byte |= KBC_INT;						// Sets the Enable Interrupt Bit to 1

	if (kbc_write_command(KBC_WRITE_COM)) {	// Sends Write Command
		return 1;
	}
	
	if (kbc_write_argument(cmd_byte)) {			// Writes Command Byte
		return 1;
	}

	return 0;
}

int kbc_write_command(uint8_t cmd) {
	uint8_t st;

	util_sys_inb(STAT_REG, &st); 				// Check IN_BUF Status:
	while (st & STAT_REG_IBF) { 				// IF EMPTY write command
		tickdelay(micros_to_ticks(DELAY_US));	// IF NOT EMPTY wait for IN_BUF to be empty
		util_sys_inb(STAT_REG, &st);
	}

	if (st & STAT_REG_PAR || st & STAT_REG_TIM) { // Parity or Timeout error, inavlid data
		return 1;
	}
	
	if (sys_outb(KBC_CMD_REG, cmd)) { 			// Sends Command
		return 1;
	}

	return 0;
}

int kbc_read_out_buf(uint8_t *content) {
	uint8_t status;

	util_sys_inb(STAT_REG, &status); 				// Check OUT_BUF Status:
	while (!(status & STAT_REG_OBF)) {			// IF SET READ CMD BYTE
		tickdelay(micros_to_ticks(DELAY_US)); // IF NOT SET wait for OUT_BUF to have something
		util_sys_inb(STAT_REG, &status);
	}

	if (status & STAT_REG_PAR || status & STAT_REG_TIM) { // Parity or Timeout error, inavlid data
		return 1;
	}

	if (util_sys_inb(OUT_BUF, content)) {		// Reads OUT_BUF
		return 1;
	}

	return 0;
}

int kbc_write_argument(uint8_t arg) {
	uint8_t status;

	util_sys_inb(STAT_REG, &status); 				// Check IN_BUF Status:
	while (status & STAT_REG_IBF) { 				// IF EMPTY write argument
		tickdelay(micros_to_ticks(DELAY_US));	// IF NOT EMPTY wait for IN_BUF to be empty
		util_sys_inb(STAT_REG, &status);
	}

	if (status & STAT_REG_PAR || status & STAT_REG_TIM) { // Parity or Timeout error, inavlid data
		return 1;
	}

	if (sys_outb(KBC_CMD_ARG, arg)) { 			// Sends Argument
		return 1;
	}

	return 0;
}
