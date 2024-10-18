/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash.c
  * @brief          : flash function
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "flash.h"
/*******************************************************************************
* Function Name  : doseFlashHasPackedMessage
* Description    : Does flash has packed messages   
* Input          : None
* Output         : 
* Return         : ture/false
*******************************************************************************/
bool doseFlashHasPackedMessage(void)
{
    uint16_t length;
    uint16_t getHead;    

    /*Is head matched*/ 
    getHead = (uint16_t)(*(uint16_t*)(STM32G0xx_FLASH_PAGE31_STARTADDR ));      
    if( EEPPROM_PACKAGEHEAD != getHead )
    {
        return false;
    }
    
    /*Is length zero*/
    length = (*(uint16_t*)(STM32G0xx_FLASH_PAGE31_STARTADDR+2));
    if( 0 == length)
    {
        return false;
    }
    
    return true;
}
/*******************************************************************************
* Function Name  : getValuablePackedMessageLengthofFlash
* Description    : Get valuable packed message length of flash 
* Input          : None
* Output         : 
* Return         : valuable length
*******************************************************************************/
uint16_t getValuablePackedMessageLengthofFlash( void )
{
    uint16_t length;
         
    /*Is head matched*/       
    if( EEPPROM_PACKAGEHEAD != (*(uint16_t*)(STM32G0xx_FLASH_PAGE31_STARTADDR )) )
    {
        return 0;
    }
    
    /*Get length*/
    length = (uint16_t)(*(uint16_t*)(STM32G0xx_FLASH_PAGE31_STARTADDR+2));   
    
    return length;
}
/*******************************************************************************
* Function Name  : readPackedMessageFromFlash
* Description    : Read packed message form flash
* Input          : buff:point to first location of received buffer.length:Maxmum length of reception
* Output         : 
* Return         : reception length
*******************************************************************************/
uint16_t readPackedMessageFromFlash( uint8_t *buff , uint16_t length)
{
    int i;
    uint16_t getLength;
    
    if( !doseFlashHasPackedMessage() )
        return 0;
    
    /*Get valuable length*/
    getLength = getValuablePackedMessageLengthofFlash();
    
    /*Read out message*/
    for(i=0;i<MIN(getLength,length);i++)
    {
        buff[i]= *(uint8_t*)(STM32G0xx_FLASH_PAGE31_STARTADDR+8+i);
    }     
    
    return MIN(getLength,length);
}
/*******************************************************************************
* Function Name  : isItOddNumber
* Description    : is input data an odd number?
* Input          : number:input data
* Output         : 
* Return         : true/false
*******************************************************************************/
bool isItOddNumber(uint16_t number)
{
    if(0 != number%8)
    {
        return true;
    }
    return false;
}
/*******************************************************************************
* Function Name  : Flash_eeprom_WriteWithPacked
* Description    : Write a group of datas to flash.
* Input          : buff:pointer of first data, length: write length
* Output         : 
* Return         : true/false
*******************************************************************************/
bool writeMessageToFlash( uint8_t *buff , uint16_t length)
{
    uint64_t temp;
    int i;
    FLASH_EraseInitTypeDef My_Flash;
    
    /*Protection*/
    if( (length+4) > STM32G0xx_PAGE_SIZE )
    {
        return false;
    }
    
    HAL_FLASH_Unlock();

    My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  
    My_Flash.Page        = 31;
    My_Flash.NbPages = 1;                        
    
    uint32_t PageError = 0;                    
    if (HAL_FLASHEx_Erase(&My_Flash, &PageError) != HAL_OK) {
        return false;
    }  

    
    temp = EEPPROM_PACKAGEHEAD |  (uint64_t)length << 16;    
    
    /*Write head*/
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, STM32G0xx_FLASH_PAGE31_STARTADDR, temp);
    
    
    /*Write datas*/
    for(i=0 ;i<length/8 ;i++)
    {
        temp = buff[8*i] | (uint64_t)buff[8*i+1]<<8 | (uint64_t)buff[8*i+2]<<16 | (uint64_t)buff[8*i+3]<<24\
        | (uint64_t)buff[8*i+4]<<32 | (uint64_t)buff[8*i+5]<<40 | (uint64_t)buff[8*i+6]<<48 | (uint64_t)buff[8*i+7]<<56;
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, STM32G0xx_FLASH_PAGE31_STARTADDR+8+8*i, temp);
    }  

    
    /*Read out and check*/
    for(i=0 ;i<length ;i++)
    {
        if( *(uint8_t*)(STM32G0xx_FLASH_PAGE31_STARTADDR+8+i) != buff[i] )
        {
            HAL_FLASH_Lock();
            return false;
        }
    }    
    
    HAL_FLASH_Lock();
    return true;    
}

