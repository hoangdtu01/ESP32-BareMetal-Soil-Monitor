
typedef unsigned int uint32_t;

/* ================= GPIO ================= */
#define GPIO_ENABLE_W1TS (*(volatile uint32_t*)0x3FF44024)
#define GPIO_OUT_W1TS    (*(volatile uint32_t*)0x3FF44008) // Set
#define GPIO_OUT_W1TC    (*(volatile uint32_t*)0x3FF4400C) // Clear

/* ================= ADC ================= */
#define SENS_SAR_READ_CTRL   (*(volatile uint32_t*)0x3FF48800) 
#define SENS_SAR_MEAS_START1 (*(volatile uint32_t*)0x3FF48804) //file header chính thức của hãng
#define SENS_SAR_MEAS_CTRL   (*(volatile uint32_t*)0x3FF4880C) //Điều khiển việc lựa chọn kênh
#define SENS_SAR_MEAS_STATUS (*(volatile uint32_t*)0x3FF48810) //
#define SENS_SAR_MEAS_DATA1  (*(volatile uint32_t*)0x3FF4881C) //

/* ================= UART ================= */
#define UART0_FIFO    (*(volatile uint32_t*)0x3FF40000)
#define UART0_STATUS  (*(volatile uint32_t*)0x3FF4001C)
#define UART0_CLKDIV  (*(volatile uint32_t*)0x3FF40014)
#define UART0_CONF0   (*(volatile uint32_t*)0x3FF40020)
#define UART_TXCNT    ((UART0_STATUS >> 16) & 0xFF) // nó ở 23-16

/* ================= PIN ================= */
#define LCD_RS 21
#define LCD_E  22
#define LCD_D4 25
#define LCD_D5 26
#define LCD_D6 32
#define LCD_D7 33

#define RELAY  27
#define ADC_CH 6   // GPIO34

/* ================= DELAY ================= */
void delay_loop(volatile uint32_t t) {
  while (t--) __asm__ volatile("nop");
}

/* ================= GPIO ================= */
void gpio_out(uint32_t pin) {
  GPIO_ENABLE_W1TS = (1 << pin);
}
void gpio_hi(uint32_t pin) {
  GPIO_OUT_W1TS = (1 << pin);
}
void gpio_lo(uint32_t pin) {
  GPIO_OUT_W1TC = (1 << pin);
}

/* ================= UART ================= */
void uart_init() {
  //APB_CLK = 80 MHz
  //CLKDIV = 694
  UART0_CLKDIV = 694;      //80,000,000 / 694 = 115200
  UART0_CONF0 = (3 << 2); // 8 data bits
}
void uart_putc(char c) {
  while (UART_TXCNT >= 126);
  UART0_FIFO = c;
}
void uart_print(const char* s) {
  while (*s) uart_putc(*s++);
}

/* ================= ADC ================= */
void adc_init() {
  SENS_SAR_READ_CTRL |= (1 << 16); // Bật bit SAR1_DIG_FORCE
  SENS_SAR_MEAS_CTRL &= ~0xF; //Xóa 4 bit thấp nhất (SAR1_CH_SEL)
  SENS_SAR_MEAS_CTRL |= ADC_CH; //Ghi giá trị kênh 6=34
}
uint32_t adc_read() {
  SENS_SAR_MEAS_START1 |= 1; //Bật bit SAR1_MEAS_START_FORCE
  while (!(SENS_SAR_MEAS_STATUS & (1 << 31))); // Kiểm tra xem đã đo xong chưa (Bit 31)
  return SENS_SAR_MEAS_DATA1 & 0xFFF; // trả về 12 bit thấp
}

/* ================= LCD ================= */
void lcd_pulse() {
  gpio_hi(LCD_E);
  delay_loop(300);
  gpio_lo(LCD_E);
}
void lcd_write4(uint8_t d) {
  (d & 1) ? gpio_hi(LCD_D4) : gpio_lo(LCD_D4);
  (d & 2) ? gpio_hi(LCD_D5) : gpio_lo(LCD_D5);
  (d & 4) ? gpio_hi(LCD_D6) : gpio_lo(LCD_D6);
  (d & 8) ? gpio_hi(LCD_D7) : gpio_lo(LCD_D7);
  lcd_pulse();
}
void lcd_cmd(uint8_t c) {
  gpio_lo(LCD_RS);
  lcd_write4(c >> 4);
  lcd_write4(c & 0x0F);
  delay_loop(3000);
}
void lcd_data(uint8_t d) {
  gpio_hi(LCD_RS);
  lcd_write4(d >> 4);
  lcd_write4(d & 0x0F);
  delay_loop(3000);
}
void lcd_print(const char* s) {
  while (*s) lcd_data(*s++); // *s += 1
}
void lcd_set(uint8_t r, uint8_t c) {
  lcd_cmd(0x80 + (r ? 0x40 : 0) + c); // 0x80: 1000 0000
}
void lcd_init() {
  //Chờ nguồn ổn định (> 40ms)
  delay_loop(800000); 

  gpio_lo(LCD_RS); // Chế độ gửi lệnh


  lcd_write4(0x03); 
  delay_loop(100000); // Chờ > 4.1ms

  lcd_write4(0x03);
  delay_loop(5000);   // Chờ > 100us

  lcd_write4(0x03); //0011
  delay_loop(5000);

  //Chuyển sang chế độ 4-bit (Gửi 0x2)
  lcd_write4(0x02);  //0010
  delay_loop(5000);

  //N=1: Hiển thị 2 dòng. N=0: 1 dòng.
  //F=1: Font 5x10. F=0: Font 5x8. 
  lcd_cmd(0x28); // Function set: 4-bit, 2 lines: 0010 1000
  lcd_cmd(0x0C); // Display ON, Cursor OFF: 0000 1100
  lcd_cmd(0x06); // Entry mode: Increment cursor: 0000 0110
  lcd_cmd(0x01); // Clear display
  delay_loop(50000); // (~1.64ms)
}

/* ================= UTILS ================= */
int map_soil(uint32_t raw) {
  int m = (4095 - raw) * 100 / (4095 - 1500);
  if (m < 0) m = 0;
  if (m > 100) m = 100;
  return m;
}

/* ================= SETUP ================= */
void setup() {
  gpio_out(LCD_RS); gpio_out(LCD_E);
  gpio_out(LCD_D4); gpio_out(LCD_D5);
  gpio_out(LCD_D6); gpio_out(LCD_D7);
  gpio_out(RELAY);

  gpio_hi(RELAY); // relay OFF

  uart_init();
  adc_init();
  lcd_init();

  lcd_print("Do Am Dat:");
}

/* ================= LOOP ================= */
void loop() {
  uint32_t raw = adc_read();
  int moisture = map_soil(raw);

  lcd_set(1, 0);
  lcd_print("      ");
  lcd_set(1, 0);

  char buf[5];
  int i = 0;
  if (moisture == 0) buf[i++] = '0';
  while (moisture) {
    buf[i++] = moisture % 10 + '0';
    moisture /= 10;
  }
  while (i--) lcd_data(buf[i]);
  lcd_data('%');

  if (raw < 2500) {
    gpio_lo(RELAY);
    uart_print("ON\r\n");
  } else {
    gpio_hi(RELAY);
    uart_print("OFF\r\n");
  }

  delay_loop(1200000);
}
