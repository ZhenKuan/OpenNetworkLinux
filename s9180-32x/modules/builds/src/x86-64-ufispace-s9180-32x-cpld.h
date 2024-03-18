/* header file for i2c cpld driver of ufispace_s9180_32x
 *
 * Copyright (C) 2024 UfiSpace Technology Corporation.
 * Melo Lin <melo.lin@ufispace.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef UFISPACE_S9180_32X_CPLD_H
#define UFISPACE_S9180_32X_CPLD_H

/* CPLD device index value */
enum cpld_id {
    cpld1
};

/* MB CPLD registers */
/* MB CPLD - Group 1. Gerneral Control */
#define CPLD_REG_00              0x00
#define CPLD_REG_07              0x07
#define CPLD_REG_01              0x01
#define CPLD_REG_41              0x41
#define CPLD_REG_42              0x42
#define CPLD_REG_43              0x43
#define CPLD_REG_45              0x45
/* MB CPLD - Group 2. LED Status */
#define CPLD_REG_80              0x80    
#define CPLD_REG_81              0x81
#define CPLD_REG_82              0x82
#define CPLD_REG_83              0x83
#define CPLD_REG_84              0x84
#define CPLD_REG_85              0x85
#define CPLD_REG_86              0x86
#define CPLD_REG_87              0x87
#define CPLD_REG_88              0x88
#define CPLD_REG_90              0x90
#define CPLD_REG_91              0x91
#define CPLD_REG_92              0x92
#define CPLD_REG_93              0x93
#define CPLD_REG_94              0x94
#define CPLD_REG_NONE            0xff



/* MASK */
#define MASK_ALL                 (0xFF)
#define MASK_NONE                (0x00)
#define MASK_HB                  (0b11110000)
#define MASK_LB                  (0b00001111)

#define MASK_BIT0                (0b00000001)
#define MASK_BIT1                (0b00000010)
#define MASK_BIT2                (0b00000100)
#define MASK_BIT3                (0b00001000)
#define MASK_BIT4                (0b00010000)
#define MASK_BIT5                (0b00100000)
#define MASK_BIT6                (0b01000000)
#define MASK_BIT7                (0b10000000)

#define MASK_BIT1_0              (0b00000011)
#define MASK_BIT2_1              (0b00000110)
#define MASK_BIT3_2              (0b00001100)
#define MASK_BIT4_3              (0b00011000)
#define MASK_BIT5_4              (0b00110000)
#define MASK_BIT6_5              (0b01100000)
#define MASK_BIT7_6              (0b11000000)

#define MASK_BIT2_0              (0b00000111)
#define MASK_BIT3_1              (0b00001110)
#define MASK_BIT4_2              (0b00011100)
#define MASK_BIT5_3              (0b00111000)
#define MASK_BIT6_4              (0b01110000)
#define MASK_BIT7_5              (0b11100000)

#define MASK_BIT3_0              MASK_LB
#define MASK_BIT4_1              (0b00011110)
#define MASK_BIT5_2              (0b00111100)
#define MASK_BIT6_3              (0b01111000)
#define MASK_BIT7_4              MASK_HB

#define MASK_BIT4_0              (0b00011111)
#define MASK_BIT5_1              (0b00111110)
#define MASK_BIT6_2              (0b01111100)
#define MASK_BIT7_3              (0b11111000)

#define MASK_BIT5_0              (0b00111111)
#define MASK_BIT6_1              (0b01111110)

#define MASK_BIT6_0              (0b01111111)
#define MASK_BIT7_1              (0b11111110)

#define MASK_BIT7_0              MASK_ALL

/* common manipulation */
#define INVALID(i, min, max)    ((i < min) || (i > max) ? 1u : 0u)

#endif
