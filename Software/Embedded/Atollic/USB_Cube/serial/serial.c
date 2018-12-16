#include "serial.h"

DMA_HandleTypeDef hdma_usart2_rx;

void serial_initialize(serial_HandleTypeDef * hserial, UART_HandleTypeDef * huart_pass)
{
//  // Start by linking the appropriate buffer. Note, there are no guards against two serial objects using the same buffer
//  switch(hserial->huart->Instance){
//    case USART1 : hserial->pbuff = &serial_buffer_USART1[0]; break;
//    case USART2 : hserial->pbuff = &serial_buffer_USART2[0]; break;
//    case USART6 : hserial->pbuff = &serial_buffer_USART6[0]; break;
//    default : _Error_Handler(__FILE__, __LINE__); break;
//  }

  hserial->huart = huart_pass;	// Add this step here so that the user cannot miss it.

  if(hserial->huart->Instance == USART1){ hserial->pbuff = &serial_buffer_USART1[0]; }
  else if(hserial->huart->Instance == USART2){ hserial->pbuff = &serial_buffer_USART2[0]; }
  else if(hserial->huart->Instance == USART6){ hserial->pbuff = &serial_buffer_USART6[0]; }

  // Some temporary ones - this also shows that I should re-think how this works for the future (its not very flexible...)
  else if(hserial->huart->Instance == UART4){ hserial->pbuff = &serial_buffer_USART1[0]; }
  else if(hserial->huart->Instance == UART5){ hserial->pbuff = &serial_buffer_USART2[0]; }
  else if(hserial->huart->Instance == UART7){ hserial->pbuff = &serial_buffer_USART1[0]; }

  else{ _Error_Handler(__FILE__, __LINE__); }
  
  // Initialize the hardware uart according to the user's settings in huart.Init
  if (HAL_UART_Init(hserial->huart) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  
  // Setup the hardware on the microcontroller:
  serial_HAL_UART_MspInit(hserial->huart);
  
  // Initialize the state variables
  hserial->num_avail = 0;
  hserial->write_index = 0;
  hserial->read_index = 0;
  hserial->write_ok = 1;
  hserial->active = 1;
  hserial->transmit_complete = 1;
  
  // Begin listening
  HAL_UART_Receive_IT(hserial->huart, &(hserial->lock), serial_default_receive_size); // first dominoe: this will wait till one byte is received then perform the callback function which continues the chain reaction
}

void serial_clear(serial_HandleTypeDef * hserial)
{
	hserial->num_avail = 0;
	hserial->write_index = 0;
	hserial->read_index = 0;
	hserial->write_ok = 1;
}

uint16_t serial_read(serial_HandleTypeDef * hserial, uint8_t * pdata, uint16_t size)
{
  // Attempt to read 'size' bytes from the correct buffer, but stop if there were not that many available
  // Returns the number of bytes read

  if(size > hserial->num_avail)
  {
    size = hserial->num_avail;
  }
  
  uint16_t indi;
  for(indi = 0; indi < size; indi++)
  {
    *(pdata + indi) = *(hserial->pbuff + hserial->read_index);
    hserial->read_index++;
    hserial->write_ok = 1;
    hserial->num_avail--;
    if(hserial->read_index >= serial_default_buff_size)
    {
      hserial->read_index = 0;
    }
  }
  return indi;
}

void serial_discard(serial_HandleTypeDef * hserial, uint16_t size)
{
	uint8_t disc = 0;
	for(uint16_t indi = 0; indi < size; indi++)
	{
		serial_read(hserial, &disc, 1);
	}
}

uint8_t serial_peek(serial_HandleTypeDef * hserial)
{
	return *(hserial->pbuff + hserial->read_index);	// Simply return the value that would be read next, but don't chage the status of the data buffer
}

void serial_write(serial_HandleTypeDef * hserial, uint8_t * pdata, uint16_t size)
{
	while((hserial->transmit_complete) == 0){};		// Wait until it is ready...
	HAL_UART_Transmit_IT(hserial->huart, pdata, size);
	hserial->transmit_complete = 0;
}







//Print
void serial_print(serial_HandleTypeDef * hserial, uint8_t * pdata)
{
	uint32_t count = 0;
	while(*(pdata + count) != '\0')
	{
		serial_write(hserial, pdata+count++, 1);
	}
}

void serial_print_uint32_t(serial_HandleTypeDef * hserial, uint32_t val, uint8_t base, uint8_t min_digits)
{
	uint8_t character = '0';
	uint8_t leading_zeros = 1;
	uint8_t prefix[2] = {0x00,0x00};
	uint32_t BPD = 0x00;

	if(min_digits == 0){ min_digits = 1; } // To prevent confusion :)

	switch(base){
	case HEX :
		// Hexadecimal
		prefix[0] = '0';
		prefix[1] = 'x';
		serial_write(hserial, &prefix[0], 2);
		for(uint8_t indi = 0; indi < 8; indi++)
		{
			switch((val & (0xF << (4*(7-indi)))) >> (4*(7-indi)) ){
			case 0x0 : character = '0'; break;
			case 0x1 : character = '1'; break;
			case 0x2 : character = '2'; break;
			case 0x3 : character = '3'; break;
			case 0x4 : character = '4'; break;
			case 0x5 : character = '5'; break;
			case 0x6 : character = '6'; break;
			case 0x7 : character = '7'; break;
			case 0x8 : character = '8'; break;
			case 0x9 : character = '9'; break;
			case 0xA : character = 'A'; break;
			case 0xB : character = 'B'; break;
			case 0xC : character = 'C'; break;
			case 0xD : character = 'D'; break;
			case 0xE : character = 'E'; break;
			case 0xF : character = 'F'; break;
			}
			if((character != '0')){ leading_zeros = 0; }
			if( (!leading_zeros) || ((8-indi) <= min_digits) ){serial_write(hserial, &character, 1);}
		}
		break;

	case DEC :
		// Decimal
		BPD = serial_utility_DD((uint16_t)(val & 0x0000FFFF));
		for(uint8_t indi = 0; indi < 8; indi++)
		{
			switch( (BPD & (0x0F << (4*(7-indi)))  ) >> (4*(7-indi)) )
			{
				case 0x0 : character = '0'; break;
				case 0x1 : character = '1'; break;
				case 0x2 : character = '2'; break;
				case 0x3 : character = '3'; break;
				case 0x4 : character = '4'; break;
				case 0x5 : character = '5'; break;
				case 0x6 : character = '6'; break;
				case 0x7 : character = '7'; break;
				case 0x8 : character = '8'; break;
				case 0x9 : character = '9'; break;
			}
			if((character != '0')){ leading_zeros = 0; }
			if( (!leading_zeros) || ((8-indi) <= min_digits) ){serial_write(hserial, &character, 1);}
		}
		break;

	case BIN :
		// Binary
		prefix[0] = '0';
		prefix[1] = 'b';
		serial_write(hserial, &prefix[0], 2);
		for(uint8_t indi = 0; indi < 32; indi++)
		{
			switch((val & (0b1 << (31-indi))) >> (31-indi)){
			case 0x0 : character = '0'; break;
			case 0x1 : character = '1'; break;
			}
			if((character != '0')){ leading_zeros = 0; }
			if( (!leading_zeros) || ((32-indi) <= min_digits) ){serial_write(hserial, &character, 1);}
		}
		break;
	}
}


//void serial_print(serial_HandleTypeDef * hserial, int32_t val, uint8_t base, uint8_t min_digits)
//{
//	if(base == DEC)
//	{
//		if(val & 0x80000000)
//		{
//			uint8_t minus = '-';
//			uint32_t abs = (uint32_t)(((int32_t)0x80000000) - val)
//			serial_write(hserial, &minus, 1);
//			serial_print(hserial, abs, base, min_digits);
//		}
//	}
//	else
//	{
//		serial_print(hserial, (uint32_t)val, base, min_digits);
//	}
//
//}



// Println
void serial_println(serial_HandleTypeDef * hserial, uint8_t * pdata)
{
	uint8_t newl = NEWLINE;
	serial_print(hserial, pdata);
	serial_write(hserial, &newl, 1);
}

void serial_println_uint32_t(serial_HandleTypeDef * hserial, uint32_t val, uint8_t base, uint8_t min_digits)
{
	uint8_t newl = NEWLINE;
	serial_print_uint32_t(hserial, val, base, min_digits);
	serial_write(hserial, &newl, 1);
}

















uint32_t serial_utility_DD(uint16_t bin)
{
	uint32_t reg0 = 0;
	uint8_t reg1 = 0;

	reg0 |= bin; // Initialization

	// Loop over all required shifts
	for(uint8_t indi = 0; indi < 16; indi++)
	{
		// Check the nibbles in the scratch space to see if they need to be added to
		for(uint8_t indj = 0; indj < 5; indj++)
		{
			if(indj < 4){ if(((reg0 & (0x0F << (4*indj + 16))) >> (4*indj + 16)) > 4){ reg0 += (3 << (4*indj + 16)); } }
			else{ if(reg1 > 4){ reg1 += 3; } }
		}
		// Now shift left by one (all registers)
		reg1 = (reg1 << 1);
		reg1 |= ((reg0 & 0x80000000) >> 31);
		reg0 = (reg0 << 1);

	}

	// Now the result should exist in
	return ((reg1 << 16) | (reg0 >> 16));
}






/*
 * The next two functions, serial_HAL_UART_MspInit and serial_HAL_UART_MspDeInit, are
 * shamelessly copied from the STM32Cube generated code because they work well, however
 * they are specific to the AES/MatrixTail application. Not a problem for me, but good 
 * to note for the future
 */

void serial_HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA15     ------> USART1_TX
    PB3     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA0-WKUP     ------> USART2_CTS
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream7;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_6;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspInit 0 */

  /* USER CODE END USART6_MspInit 0 */
    /* USART6 clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();
  
    /**USART6 GPIO Configuration    
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART6 interrupt Init */
    HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspInit 1 */

  /* USER CODE END USART6_MspInit 1 */
  }
}

void serial_HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA15     ------> USART1_TX
    PB3     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA0-WKUP     ------> USART2_CTS
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspDeInit 0 */

  /* USER CODE END USART6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART6_CLK_DISABLE();
  
    /**USART6 GPIO Configuration    
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

    /* USART6 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspDeInit 1 */

  /* USER CODE END USART6_MspDeInit 1 */
  }
} 







/* UART Callback Functions */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart)
{
  //__NOP();
	serial_HandleTypeDef *temp_serial_ptr;

	// Get the corresponding serial handle
	if(huart->Instance == USART1){ temp_serial_ptr = &serial1; temp_serial_ptr->transmit_complete = 1; }
	else if(huart->Instance == USART2){ temp_serial_ptr = &serial2; temp_serial_ptr->transmit_complete = 1; }
	else if(huart->Instance == USART6){ temp_serial_ptr = &serial6; temp_serial_ptr->transmit_complete = 1; }

	// Temporary ones for the f767
	else if(huart->Instance == UART4){ temp_serial_ptr = &serial2; temp_serial_ptr->transmit_complete = 1; }
	else if(huart->Instance == UART5){ temp_serial_ptr = &serial1; temp_serial_ptr->transmit_complete = 1; }

	// Ugh another ugly temporary fix to a stupid system. You HAVE to redo the serial library
	else if(huart->Instance == UART7){ temp_serial_ptr = &serial1; temp_serial_ptr->transmit_complete = 1; }


	else{ _Error_Handler(__FILE__, __LINE__); }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
  //__NOP();

  serial_HandleTypeDef *temp_serial_ptr;

  // Get the corresponding serial handle
  if(huart->Instance == USART1){ temp_serial_ptr = &serial1; }
  else if(huart->Instance == USART2){ temp_serial_ptr = &serial2; }
  else if(huart->Instance == USART6){ temp_serial_ptr = &serial6; }

  // Temporary... seriously have to re-do this for flexibility
//  else if(huart->Instance == UART4){ temp_serial_ptr = &serial2; }
//  else if(huart->Instance == UART5){ temp_serial_ptr = &serial1; }


  // Yet another temporary patch. Will be re-doing this in the near future
  else if(huart->Instance == UART7){ temp_serial_ptr = &serial1; }

  else{ _Error_Handler(__FILE__, __LINE__); return; }	// return statement here will never be reached because of error handler while loop, however it fixes a compiler warning

  if(temp_serial_ptr->active)
  {
    if(temp_serial_ptr->write_ok)
    {
      *((temp_serial_ptr->pbuff) + temp_serial_ptr->write_index ) = temp_serial_ptr->lock;      // Lock will have been filled with new data by the HAL_Recieve function
      temp_serial_ptr->write_index++;
      temp_serial_ptr->num_avail++;
      if(temp_serial_ptr->write_index >= serial_default_buff_size)
      {
        temp_serial_ptr->write_index = 0;                             // Wrap the write index back to zero
      }
      if(temp_serial_ptr->write_index == temp_serial_ptr->read_index)
      {
        temp_serial_ptr->write_ok = 0;                                // If after incrementing the write pointer it lands on the read pointer then the buffer must be full
        temp_serial_ptr->num_avail = serial_default_buff_size;
      }
    }
    else
    {
        // This discards the value in lock, effectively blocking it from entering the buffer
    }

    HAL_UART_Receive_IT(temp_serial_ptr->huart, &(temp_serial_ptr->lock), serial_default_receive_size); // Call the receive function again to continue the chain reaction
  }
}
