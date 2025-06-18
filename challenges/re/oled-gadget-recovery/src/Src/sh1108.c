/*
 * Copyright (c) 2025 Troy Callahan / Cybolt Solutions LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, **provided that the following attribution requirement is
 * met**:
 *
 *     ▶  All copies or substantial portions of the Software (source or binary)
 *        must retain this copyright notice, the permission notice, and a
 *        clear attribution to Troy Callahan / Cybolt Solutions LLC.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include "sh1108.h"

uint8_t sh1108_fRAMe_buffer_raw[SH1108_SIZE_FRAME] = {0x00};
uint8_t sh1108_page_buffer_raw[SH1108_SIZE_PAGE] = {0x00};

void sh1108_reset(){
    HAL_GPIO_WritePin(PORT_DC, PIN_DC, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORT_RES, PIN_RES, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(PORT_RES, PIN_RES, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(PORT_RES, PIN_RES, GPIO_PIN_SET);
}

void sh1108_init(){
    sh1108_reset();
    // I realize I could do this as one single message, but this is bootup - it doesn't have to be _that_ fast.
    HAL_GPIO_WritePin(PORT_DC, PIN_DC, GPIO_PIN_RESET); // Command mode
    for (uint8_t i = 0; i < sizeof(SH1108_INIT_SEQ); i++) {
        HAL_SPI_Transmit(&SPI_HANDLE, &SH1108_INIT_SEQ[i], 1, 100);
    }
    sh1108_frame_buffer_clear();
    sh1108_page_buffer_clear();
}

void sh1108_send_command(uint8_t command) {
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit(&SPI_HANDLE, &command, 1, 100);
}

// Function to send data to the OLED
void sh1108_send_data(uint8_t * data, uint16_t size) {
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit(&SPI_HANDLE, data, size, 100);
}

void sh1108_set_page_address(uint8_t add) {
    sh1108_send_command(0xB0);
    sh1108_send_command(0x1F & add);
}

void sh1108_set_column_address(uint8_t add) {
    sh1108_send_command((0x0F & add));
    sh1108_send_command((0x10 | (add >> 4)));
}

void sh1108_set_contrast(uint8_t contrast) {
    uint8_t txData[2] = {SH1108_CMD_DISPLAY_CONTRAST, contrast};
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit(&SPI_HANDLE, txData, 2, 100);
}

void sh1108_frame_buffer_clear(){
  memset(sh1108_fRAMe_buffer_raw, 0x00, SH1108_SIZE_FRAME);
}

void sh1108_page_buffer_clear(){
  memset(sh1108_page_buffer_raw, 0x00, SH1108_SIZE_PAGE);
}

void sh1108_frame_buffer_invert(void){
  uint32_t * cast = (uint32_t *)sh1108_fRAMe_buffer_raw;
  for(uint16_t i = 0x00; i < SH1108_SIZE_FRAME/4; i++){
    cast[i] = ~cast[i];
  }
}

void sh1108_send_page_buffer(uint8_t page_index){
  sh1108_set_page_address(page_index);
  sh1108_set_column_address(SH1108_COLUMN_START);
  sh1108_send_data(sh1108_page_buffer_raw, SH1108_SIZE_PAGE);
  sh1108_page_buffer_clear();
}

void sh1108_send_frame_buffer(){
  for (uint8_t page_index = SH1108_PAGE_START; page_index <= SH1108_PAGE_END; page_index++)
  {
    sh1108_set_page_address(page_index);
    sh1108_set_column_address(SH1108_COLUMN_START);
    sh1108_send_data(
      (uint8_t *)(sh1108_fRAMe_buffer_raw + (SH1108_SIZE_PAGE*page_index)),
      SH1108_SIZE_PAGE
    );
  }
  sh1108_frame_buffer_clear();
  return;
}

void sh1108_send_frame_buffer_no_clear(){
  for (uint8_t page_index = SH1108_PAGE_START; page_index <= SH1108_PAGE_END; page_index++)
  {
    sh1108_set_page_address(page_index);
    sh1108_set_column_address(SH1108_COLUMN_START);
    sh1108_send_data(
      (uint8_t *)(sh1108_fRAMe_buffer_raw + (SH1108_SIZE_PAGE*page_index)),
      SH1108_SIZE_PAGE
    );
  }
  return;
}

void sh1108_set_pixel_xy(uint8_t x, uint8_t y){
  uint8_t page = (x/SH1108_WIDTH_PAGE);
  uint8_t bit_index_in_page = (x%SH1108_WIDTH_PAGE);
  uint16_t byte_index = (page*SH1108_SIZE_PAGE)+y;
  sh1108_fRAMe_buffer_raw[byte_index] |= (1<<bit_index_in_page);
  return;
}

void sh1108_unset_pixel_xy(uint8_t x, uint8_t y){
  uint8_t page = (x/SH1108_WIDTH_PAGE);
  uint8_t bit_index_in_page = (x%SH1108_WIDTH_PAGE);
  uint16_t byte_index = (page*SH1108_SIZE_PAGE)+y;
  sh1108_fRAMe_buffer_raw[byte_index] &= ~ (1 << bit_index_in_page);
  return;
}

void sh1108_add_icon_to_buff(uint8_t x, uint8_t y, struct icon *icon_arg) {
    // Extract dimensions from the icon structure
    uint16_t width = icon_arg->width;
    uint16_t height = icon_arg->height;

    // Iterate over the pixels in the rectangular icon
    for (uint16_t y_pos = 0; y_pos < height; y_pos++) {
        for (uint16_t x_pos = 0; x_pos < width; x_pos++) {
            // Calculate the bit position in the packed data
            uint16_t pixel_index = y_pos * width + x_pos;
            uint16_t byte_index = pixel_index / 8;
            uint8_t bit_index = 7 - (pixel_index % 8); // MSB-first format

            // Extract the bit value
            uint8_t pixel_value = (icon_arg->data[byte_index] >> bit_index) & 0x01;

            // Set the pixel in the display buffer if it's within bounds
            if (pixel_value && (x + x_pos < 160) && (y + y_pos < 128)) {
                sh1108_set_pixel_xy(x + x_pos, y + y_pos);
            }
        }
    }

    return;
}

void sh1108_icon_bounce_infinite(struct icon* icon_param)
{
    uint8_t ball_x    = 0;
    uint8_t ball_y    = 0;
    int8_t  ball_dx   = 1; // Ball movement in x-direction
    int8_t  ball_dy   = 1; // Ball movement in y-direction

    for (uint8_t z = 0x00; z < 0x100; z++)
    { // loops forever
        // Update position
        ball_x += ball_dx;
        ball_y += ball_dy;

        // Check for collision with screen edges and reverse direction if needed
        if (ball_x == 0 || ball_x >= (SH1108_WIDTH - icon_param->width))
        {
            ball_dx = -ball_dx;
        }
        if (ball_y == 0 || ball_y >= (SH1108_HEIGHT - icon_param->height))
        {
            ball_dy = -ball_dy;
        }

        sh1108_add_icon_to_buff((ball_x),(ball_y), icon_param);
        // for(uint32_t delay = (((uint32_t)ADC1_Read())<<8); delay != 0x00; delay--){;}
        for(uint32_t delay = 0x1ffff; delay != 0x00; delay--){;}
        // sh1108_frame_buffer_invert();
        // Write the buffer to the display
        sh1108_send_frame_buffer();
    }
}

void sh1108_draw_horizontal_line(uint8_t x1, uint8_t x2, uint8_t y, uint8_t thickness)
{
    if (x1 > x2) {
        uint8_t temp = x1;
        x1 = x2;
        x2 = temp;
    }

    if (x1 >= SH1108_WIDTH || y >= SH1108_HEIGHT)
        return;

    if (x2 >= SH1108_WIDTH)
        x2 = SH1108_WIDTH - 1;

    for (uint8_t x = x1; x <= x2; x++) {
        for (uint8_t t = 0; t < thickness; t++) {
            uint8_t draw_y = y + t;
            if (draw_y < SH1108_HEIGHT)
                sh1108_set_pixel_xy(x, draw_y);
        }
    }
}

void sh1108_draw_vertical_line(uint8_t y1, uint8_t y2, uint8_t x, uint8_t thickness)
{
    if (y1 > y2) {
        uint8_t temp = y1;
        y1 = y2;
        y2 = temp;
    }

    if (x >= SH1108_WIDTH || y1 >= SH1108_HEIGHT)
        return;

    if (y2 >= SH1108_HEIGHT)
        y2 = SH1108_HEIGHT - 1;

    for (uint8_t y = y1; y <= y2; y++) {
        for (uint8_t t = 0; t < thickness; t++) {
            uint8_t draw_x = x + t;
            if (draw_x < SH1108_WIDTH)
                sh1108_set_pixel_xy(draw_x, y);
        }
    }
}

void sh1108_draw_box(uint8_t x, uint8_t y, uint8_t h, uint8_t w, uint8_t thickness)
{
    if (w == 0 || h == 0 || thickness == 0)
        return;

    // Bottom edge
    sh1108_draw_horizontal_line(x, x + w - 1, y, thickness);

    // Top edge (fixed at y + h - thickness)
    sh1108_draw_horizontal_line(x, x + w - 1, y + h - thickness, thickness);

    // Left edge
    sh1108_draw_vertical_line(y, y + h - 1, x, thickness);

    // Right edge (fixed at x + w - thickness)
    sh1108_draw_vertical_line(y, y + h - 1, x + w - thickness, thickness);
}
