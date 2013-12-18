/*> PCcard/h <*/
/*----------------------------------------------------------------------*/
/*                                                                      */
/*                              PCcard.h                                */
/*                              --------                                */
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.  */
/* Information derived from "PCMCIA PC CARD STANDARD" August 21st 1990  */
/* and "JEIDA MEMORY CARD INFORMATION STRUCTURE" Ver.4 June 1990.       */
/*                                                                      */
/************************************************************************/
/* NOTE: If this file is updated, then the command                      */
/*       "genhdr -p /hsrc/include/abcARM/asm" should be executed.       */
/*       The "genhdr" binary should be remade first to include any new  */
/*       definitions.                                                   */
/*----------------------------------------------------------------------*/

#ifndef __PCcard_h
#define __PCcard_h

/*----------------------------------------------------------------------*/
/*---- LEVEL 1 ---- Basic CARD compatibility ---------------------------*/
/*----------------------------------------------------------------------*/
/* This is the lowest-level of the standard. Any CARD that complies to  */
/* the standard must have at least a rudimentary "CARD Information      */
/* Structure" (CIS). This CIS must start at address zero of the CARDs   */
/* "attribute" memory space. The CIS structure contains little-endian   */
/* information.                                                         */
/* NOTE: The electrical specification of PC CARDs requires that         */
/*       information is placed only in the "even" byte addresses of     */
/*       attribute memory space. The "odd" byte locations are NOT used. */
/*       All the following structures treat the memory as consecutive   */
/*       locations (as is the case with common memory), however, in     */
/*       attribute memory they will be stored in consecutive "even"     */
/*       byte locations.                                                */
/*----------------------------------------------------------------------*/
/* The CIS is a variable length linked list of "TPL_hdr" structures.    */
/* The list can be terminated by a "TPL_LINK" value of 0xFF or with a   */
/* "TPL_hdr" with "TPL_CODE" value of 0xFF. The "TPL_LINK" is a         */
/* unsigned offset to the next "TPL_hdr" from the address of the        */
/* next byte (therefore a "TPL_LINK" value of 0x00 means there is no    */
/* data following the "TPL_hdr" structure). Note: The attribute memory  */
/* will require stepping onto the next "even" byte and adding           */
/* (2 * TPL_LINK) to get to the next structure.                         */

/* TPL_CODE                                                             */
/* --------                                                             */
/* Notes: Codes 0x80 to 0xFE are allocated to the vendor.               */
/*        "common" refers to the common memory on a CARD.               */
/*        "attr" refers to the attribute memory on a CARD.              */
typedef enum {
              CISTPL_NULL       = 0x00, /* null entry (ignore)          */
              CISTPL_DEVICE     = 0x01, /* common device information    */
              CISTPL_CHECKSUM   = 0x10, /* checksum control             */
              CISTPL_LONGLINK_A = 0x11, /* link to attribute memory     */
              CISTPL_LONGLINK_C = 0x12, /* link to common memory        */
              CISTPL_LINKTARGET = 0x13, /* link target                  */
              CISTPL_NO_LINK    = 0x14, /* no-link                      */
              CISTPL_VERS_1     = 0x15, /* level 1 version/product info */
              CISTPL_ALTSTR     = 0x16, /* alternate-language string    */
              CISTPL_DEVICE_A   = 0x17, /* attr device information      */
              CISTPL_JEDEC_C    = 0x18, /* common JEDEC programming info*/
              CISTPL_JEDEC_A    = 0x19, /* attr JEDEC programming info  */
              CISTPL_VERS_2     = 0x40, /* level 2 version info         */
              CISTPL_FORMAT     = 0x41, /* format                       */
              CISTPL_GEOMETRY   = 0x42, /* geometry of disk-like formats*/
              CISTPL_BYTEORDER  = 0x43, /* byte-order for memory formats*/
              CISTPL_DATE       = 0x44, /* initialisation timestamp     */
              CISTPL_BATTERY    = 0x45, /* battery replacement timestamp*/
              CISTPL_ORG        = 0x46, /* data organisation            */
              CISTPL_END        = 0xFF  /* end-of-list marker           */
             } CISTPL_CODES ;

typedef struct TPL_hdr {
                        byte TPL_CODE ; /* actually "CISTPL_CODES"   */
                        byte TPL_LINK ; /* link to next "TPL_hdr"    */
                       } TPL_hdr ;

/*----------------------------------------------------------------------*/
/* "TPL_CODE" processing                                                */
/*                                                                      */
/* The standard requires system software to be carefully coded to       */
/* prevent incompatibilities between systems.                           */
/*                                                                      */
/*      o       The read routine should first examine the TPL_CODE byte */
/*              and ignore the structure is the byte is not recognised  */
/*              (ie. by reading the link byte and stepping over any     */
/*              private data). NOTE: no bytes should be read from       */
/*              unrecognised structures since the area may contain      */
/*              active hardware registers.                              */
/*                                                                      */
/*      o       Upto 255 bytes of potential data may be present in a    */
/*              structure containing a TPL_LINK of 0xFF (end-of-chain   */
/*              marker).                                                */
/*                                                                      */
/*      o       Validation of long-link addresses and destination       */
/*              contents should only be performed when starting to      */
/*              process the chain linked to (ie. the long-link          */
/*              information should be recorded and not processed until  */
/*              the current chain end has been met).                    */
/*                                                                      */
/*      o       Long-links that to not point to a link-target should    */
/*              not generate any warning messages, since the CARD can   */
/*              be assumed to be uninitialised.                         */
/*                                                                      */
/*----------------------------------------------------------------------*/
/* CISTPL_NULL                                                          */
/*                                                                      */
/* This is purely a place-holder. IT HAS A NON-STANDARD STRUCTURE. It   */
/* consists solely of the "TPL_CODE" (ie. no TPL-LINK). The next        */
/* "TPL_hdr" follows immediately. Since it is ignored it can be used    */
/* to over-write existing data in programmable environments.            */
/*----------------------------------------------------------------------*/
/* CISTPL_END                                                           */
/*                                                                      */
/* This structure marks the end of the CIS structure chain. IT HAS A    */ 
/* NON-STANDARD STRUCTURE. It consists solely of the "TPL_CODE" (ie. no */
/* TPL-LINK).                                                           */
/* On reaching such a control code the processing software shall        */
/*                                                                      */
/*      o       continue processing at the address specified in any     */
/*              found long link structure.                              */
/*                                                                      */
/*      o       if a CISTPL_NO_LINK structure was encountered then      */
/*              terminate the processing.                               */
/*                                                                      */
/*      o       if processing the CIS structure chain starting at       */
/*              address 0 in attribute memory, then assume processing   */
/*              as if a CISTPL_LONGLINK_C with offset 0 was encountered.*/
/*                                                                      */
/*      o       if NOT processing the CIS structure chain starting at   */
/*              address 0 in attribute memory and NO long link          */
/*              structures were found, then terminate processing.       */
/*                                                                      */
/* The use of such a terminating structure rather than encoding the     */
/* list end in the TPL_LINK field of the last structure allows future   */
/* transparent additions to the list.                                   */
/* If this code is found in location 0 of the attribute memory then the */
/* CARD contains no CIS data. In this case, the device information must */
/* be obtained by an actual read/write access to the CARD. The access   */
/* time of common memory for such devices must be at least 250ns.       */
/*----------------------------------------------------------------------*/
/* CISTPL_LONGLINK_A                                                    */
/* CISTPL_LONGLINK_C                                                    */
/*                                                                      */
/* These structures are used to jump from one CIS structure chain to    */
/* another. The target chain may be in attribute or common memory (as   */
/* indicated by the control code). Only ONE long link structure shall   */
/* appear in a CIS structure chain. It need NOT be the last structure   */
/* in the chain (the rest of the chain being processed before the link  */
/* is taken). The long link should point to a "CISTPL_LINKTARGET" entry */
/* in the destination CIS chain. Invalid chain links are ignored.       */
/* The data is a 4byte unsigned offset into the relevant memory area.   */

typedef struct CISTPL_LONGLINK_struct {
                                       byte TPLL_ADDR[4] ;
                                      } CISTPL_LONGLINK_struct ;

/*----------------------------------------------------------------------*/
/* CISTPL_LINKTARGET                                                    */
/*                                                                      */
/* The link target structure is used for robustness. Every long link    */
/* must point to a valid link target structure. This structure contains */
/* a 3character identity string "CIS".                                  */

typedef struct CISTPL_LINKTARGET_struct {
                                         byte TFLTG_TAG[3] ;
                                        } CISTPL_LINKTARGET_struct ;

#define TPLTG_TAG_ID    (('C' << 0) | ('I' << 8) | ('S' << 16))

/*----------------------------------------------------------------------*/
/* CISTPL_NO_LINK                                                       */
/*                                                                      */
/* The attribute memory CIS of a RAM CARD must be kept small (for space */
/* economy). To save attribute memory space, processing software shall  */
/* assume the presence of a CISTPL_LONGLINK_C structure (with an offset */
/* of 0x00000000). This assumption can be over-ridden by placing an     */
/* explicit CISTPL_NO_LINK structure in the attribute memory CIS. A CIS */
/* structure chain can contain at most ONE CISTPL_NO_LINK. The no-link  */
/* and long-link structures are mutually exclusive.                     */
/*----------------------------------------------------------------------*/
/* CISTPL_CHECKSUM                                                      */
/*                                                                      */
/* For additional reliability (over the chain links) the CIS can        */
/* include a checksum structure. This structure has three fields:       */
/*      o       2byte offset to start of block (signed)                 */
/*      o       2byte length of block (unsigned)                        */
/*      o       1byte of expected checksum                              */
/*                                                                      */

typedef struct CISTPL_CHECKSUM_struct {
                                       byte TPLCKS_ADDR[2] ;
                                       byte TPLCKS_LEN[2] ;
                                       byte TPLCKS_CS ;
                                      } CISTPL_CHECKSUM_struct ;

/* "TPLCKS_ADDR" contains the offset to the region to be checksummed,   */
/* relative to the start of this "TPL_hdr" structure. It is a 2byte     */
/* signed integer.                                                      */
/* If the "CISTPL_CHECKSUM" structure appears in common memory space    */
/* then the checksum is simply calculated by adding the consecutive     */
/* bytes in area defined and compare the lo8bits against the stored     */
/* checksum value. If the structure appears in attribute memory space   */
/* then we the area is defined by (2 * TPLCKS_ADDR) as an offset and    */
/* (2 * TPLCKS_LEN) as a length. We sum the "even" bytes in this area   */
/* only and then compare the lo8bits against the stored checksum.       */
/* The "TPL_LINK" field must be at least 5 (to cover the private data). */
/*----------------------------------------------------------------------*/
/* CISTPL_VERS_1                                                        */
/*                                                                      */
/* This structure contains information on the version of the PC CARD    */
/* guideline the level 1 attribute information conforms to. Upto four   */
/* optional strings can be included, providing:                         */
/*      o       manufacturer information (name)                         */
/*      o       product information (product name)                      */
/*      o       additional product information 1 (LOT #)                */
/*      o       additional product information 2 (programming)          */
/* The end of the private structure data is marked with a 0xFF byte.    */

typedef struct CISTPL_VERS_1_struct {
                                     byte TPLLV1_MAJOR ;
                                     byte TPLLV1_MINOR ;
                                     byte TPLLV1_INFO[1] ;
                                    } CISTPL_VERS_1_struct ;

#define L1_format_major (0x04)  /* default for "TPLLV1_MAJOR".          */
#define L1_format_minor (0x00)  /* default for "TPLLV1_MINOR".          */

/*----------------------------------------------------------------------*/
/* CISTPL_ALTSTR                                                        */
/*                                                                      */
/* Several "TPL" structures contain ISO646 character strings.           */
/* Alternative string structures provide the ability to display these   */
/* strings in the host language environment. The strings are recorded   */
/* in "ISO 646 IRV" format.                                             */

typedef struct CISTPL_ALTSTR_struct {
                                     byte TPL_ALTSTR_ESC[1] ;
                                    } CISTPL_ALTSTR_struct ;

/* The string at "TPL_ALTSTR_ESC" is a NULL terminated ESCape sequence  */
/* used to select the character set for the following strings (the      */
/* leading ESCape character is NOT included).                           */
/* The remainder of this structure is taken up with NULL terminated     */
/* characters strings in the language defined by the leading ESCape     */
/* sequence. These strings refer (in sequence) to the strings found in  */
/* the most recent non-TPL_ALTSTR_ESC structures.                       */
/* The byte 0xFF marks the end of the strings.                          */
/*----------------------------------------------------------------------*/
/* CISTPL_DEVICE                                                        */
/* CISTPL_DEVICE_A                                                      */
/*                                                                      */
/* These structures contain information about the devices on the CARD.  */
/* The "CISTPL_DEVICE" (describing the common memory area) must be the  */
/* first structure in attribute memory. The "CISTPL_DEVICE_A"           */
/* describing the attribute memory is optional.                         */
/*                                                                      */
/* This structure contains multiple "Device Info" structures (upto a    */
/* limit of 0xFB bytes (see previous comments)). This list is           */
/* terminated by a 0xFF character.                                      */
/*                                                                      */
/* Device Info                                                          */
/*                                                                      */
/* These fields are split into two variable length byte sequences       */
/* describing the "Device ID" and the "Device Size".                    */
/*                                                                      */
/* Device ID                                                            */
/*                                                                      */
/* This indicates the device type and access time for a block of        */
/* memory. It has at least one byte of device type and speed            */
/* information and upto three optional bytes depending on the           */
/* information encoded in this first byte.                              */
/*                                                                      */
/*        7   6   5   4   3   2   1   0                                 */
/*      +---+---+---+---+---+---+---+---+                               */
/*      |DT3|DT2|DT1|DT0|WPS|DS2|DS1|DS0|                               */
/*      +---+---+---+---+---+---+---+---+                               */
/*        DT[3..0] = Device Type                                        */
/*        WPS      = Write Protect Switch (WPS) effect (0 = Effective)  */
/*        DS[2..0] = Device Speed                                       */
/*                                                                      */

typedef enum {
              DS_Reserved0      = 0,    /* undefined device speed       */
              DS_250ns          = 1,    /* 250ns device                 */
              DS_200ns          = 2,    /* 200ns device                 */
              DS_150ns          = 3,    /* 150ns device                 */
              DS_100ns          = 4,    /* 100ns device                 */
              DS_Reserved5      = 5,    /* reserved for future updates  */
              DS_Reserved6      = 6,    /* reserved for future updates  */
              DS_Extend         = 7     /* speed defined in extension   */
             } TPL_DeviceSpeed ;

typedef enum {
              DT_Null           = 0x0,  /* see comments below           */
              DT_MASK_ROM       = 0x1,  /* MASKed ROM                   */
              DT_OTPROM         = 0x2,  /* One Time Programmable ROM    */
              DT_EPROM          = 0x3,  /* UV EPROM                     */
              DT_EEPROM         = 0x4,  /* EEPROM                       */
              DT_FLASH          = 0x5,  /* Flash EPROM                  */
              DT_SRAM           = 0x6,  /* Static RAM                   */
              DT_DRAM           = 0x7,  /* Dynamic RAM                  */
              DT_IO             = 0xD,  /* IO device                    */
              DT_EXTEND         = 0xE,  /* type defined in extension    */
              DT_Reserved       = 0xF   /* reserved for future use      */
             } TPL_DeviceType ;

typedef struct DeviceInfo_struct {
#if 1
                                  /* ANSI does not allow char bitfields */
                                  byte DeviceInfo_bitfield ;
#else
                                  byte DeviceSpeed : 3,
                                       WPS         : 1,
                                       DeviceType  : 4 ;
#endif
                                 } DeviceInfo_struct ;

#define DeviceSpeed_mask        (0x7)
#define DeviceSpeed_shift       (0)
#define WPS_mask                (0x8)
#define WPS_shift               (3)
#define DeviceType_mask         (0xF)
#define DeviceType_shift        (4)
/* NOTE: The "DeviceType_mask" is a shift first, mask second style, whereas
 *	 all the others are mask first, then shift. This is done because even
 * 	 though the device type is a number (manifest) I use it as a bit index
 *	 in the bitmask of supported types. Even though this makes no real
 *	 difference it makes certain expressions shorter.
 */

/* If both speed and device type are extended then the extended device  */
/* speed comes first, followed immediately by the extended device type. */
/*                                                                      */
/* Extended Device Speed                                                */
/*                                                                      */
/*        7   6   5   4   3   2   1   0                                 */
/*      +---+---+---+---+---+---+---+---+                               */
/*      |EXT|MT3|MT2|MT1|MT0|EX2|EX1|EX0|                               */
/*      +---+---+---+---+---+---+---+---+                               */
/*        EXT      = 0 = defined; 1 = further extension byte follows    */
/*        MT[3..0] = mantissa                                           */
/*        EX[2..0] = exponent                                           */
/*                                                                      */
/* If the "EXT" bit is set then all subsequent bytes with the "EXT" bit */
/* set should be ignored. This guideline does not specify the format    */
/* of these extension bytes.                                            */
/* The device speed is calculated by (MT * (10 ^ EX)).                  */


typedef enum {
              DS_MT_Reserved    = 0,    /* reserved for future use      */
              DS_MT_1p0         = 1,    /* 1.0                          */
              DS_MT_1p2         = 2,    /* 1.2                          */
              DS_MT_1p3         = 3,    /* 1.3                          */
              DS_MT_1p5         = 4,    /* 1.5                          */
              DS_MT_2p0         = 5,    /* 2.0                          */
              DS_MT_2p5         = 6,    /* 2.5                          */
              DS_MT_3p0         = 7,    /* 3.0                          */
              DS_MT_3p5         = 8,    /* 3.5                          */
              DS_MT_4p0         = 9,    /* 4.0                          */
              DS_MT_4p5         = 10,   /* 4.5                          */
              DS_MT_5p0         = 11,   /* 5.0                          */
              DS_MT_5p5         = 12,   /* 5.5                          */
              DS_MT_6p0         = 13,   /* 6.0                          */
              DS_MT_7p0         = 14,   /* 7.0                          */
              DS_MT_8p0         = 15    /* 8.0                          */
             } TPL_MantissaSpeedExtension ;

typedef enum {
              DS_EX_1ns         = 0,    /* 1 nano-second                */
              DS_EX_10ns        = 1,    /* 10 nano-seconds              */
              DS_EX_100ns       = 2,    /* 100 nano-seconds             */
              DS_EX_1us         = 3,    /* 1 micro-second               */
              DS_EX_10us        = 4,    /* 10 micro-seconds             */
              DS_EX_100us       = 5,    /* 100 micro-seconds            */
              DS_EX_1ms         = 6,    /* 1 milli-second               */
              DS_EX_10ms        = 7     /* 10 milli-seconds             */
             } TPL_ExponentSpeedExtension ;

typedef struct SpeedExtension_struct {
#if 1
                                      /* ANSI does not allow char bitfields */
                                      byte SpeedExtension_bitfield ;
#else
                                      byte Exponent : 3,
                                           Mantissa : 4,
                                           Extend   : 1 ;
#endif
                                     } SpeedExtension_struct ;

#define DSExponent_mask         (0x07)
#define DSExponent_shift        (0)
#define DSMantissa_mask         (0x78)
#define DSMantissa_shift        (3)
#define SpeedExtend_shift       (7)
#define SpeedExtend_bit         (1 << (SpeedExtend_shift))

/* Extended Device Type                                                 */
/*                                                                      */
/* This field is used to extend the device type code of the Device ID   */
/* byte. If the "TypeExtend_bit" is set then another extension byte     */
/* follows this one. The meaning of bits 6..0 are currently undefined   */
/* and reserved for future use.                                         */

typedef struct TypeExtension_struct {
#if 1
                                     /* ANSI does not allow char bitfields */
                                     byte TypeExtension_bitfield ;
#else
                                     byte        : 7,   /* unused */
                                          Extend : 1 ;
#endif
                                    } TypeExtension_struct ;

#define TypeExtend_shift        (7)
#define TypeExtend_bit          (1 << (TypeExtend_shift))

/* Device Size                                                          */
/*                                                                      */
/* This field has the length of the address block described by the      */
/* Device ID field preceding.                                           */
/*                                                                      */
/*      +---+---+---+---+---+---+---+---+                               */
/*      |MT4|MT3|MT2|MT1|MT0|EX2|EX1|EX0|                               */
/*      +---+---+---+---+---+---+---+---+                               */
/*        MT[4..0] = (mantissa - 1) (1..32)                             */
/*        EX[1..0] = exponent (0..6 as 7 is reserved for future use)    */
/*                                                                      */
/* The size of the block (in bytes) is calculated by:                   */
/*      ((MT + 1) * 512 * (4 ^ EX))                                     */
/*                                                                      */

typedef struct DeviceSize_struct {
#if 1
                                  /* ANSI does not allow char bitfields */
                                  byte DeviceSize_bitfield ;
#else
                                  byte Exponent : 3,
                                       Mantissa : 5 ;
#endif
                                 } DeviceSize_struct ;

#define SizeExponent_mask       (0x07)
#define SizeExponent_shift      (0)
#define SizeMantissa_mask       (0xF8)
#define SizeMantissa_shift      (3)

/* If type and speed byte is 0x00 then the device is unspecified. If    */
/* the device size information is valid then the area is treated as a   */
/* NULL device (no writes or reads to that area should actually be      */
/* performed).                                                          */
/* A type and speed byte of 0xFF marks the end of the device            */
/* information structures.                                              */
/*----------------------------------------------------------------------*/
/* CISTPL_JEDEC_C                                                       */
/* CISTPL_JEDEC_A                                                       */
/*                                                                      */
/* These structures are optional. "CISTPL_JEDEC_C" describes            */
/* programmable devices in the common memory, while "CISTPL_JEDEC_A"    */
/* describes programmable devices in the attribute memory.              */
/*                                                                      */
/* Each entry in the JEDEC information structure corresponds to each    */
/* block of device described in the "CISTPL_DEVICE" and                 */
/* "CISTPL_DEVICE_A" structures.                                        */
/*                                                                      */
/* Each entry consists of two bytes:                                    */
/*      byte 0  : manufacturer ID assigned by JEDEC committee JC-42.3   */
/*      byte 1  : manufacturer specific device identity                 */

typedef struct JEDEC_struct {
                             byte manufacturerID ;
                             byte deviceID ;
                            } JEDEC_struct ;

/* The manufacturer ID byte must have an "odd" number of bits set in    */
/* byte. The MSB (7) is used as a parity bit to ensure this. The value  */
/* 0x00 is used to indicate "no JEDEC identifier for this device".      */
/*                                                                      */
/* A manufacturer code of 0xFF terminates the list.                     */
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*---- LEVEL 2 ----  Logical CARD structure ----------------------------*/
/*----------------------------------------------------------------------*/
/* These pieces of information are provided by the manufacturer who     */
/* provides the data stored in the CARD. It is assumed that the         */
/* referenced information is stored in common memory.                   */
/*----------------------------------------------------------------------*/
/* CISTPL_VERS_2                                                        */
/*                                                                      */

typedef struct CISTPL_VERS_2_struct {
                                     byte TPLLV2_VERS ;
                                     byte TPLLV2_COMPLY ;
                                     byte TPLLV2_DINDEX[2] ;
                                     byte TPLLV2_RSV[2] ;
                                     byte TPLLV2_VSPEC[2] ;
                                     byte TPLLV2_NHDR ;
                                     /* OEM and INFO strings follow */
                                     byte TPLLV2_OEM[1] ;
                                    } CISTPL_VERS_2_struct ;

/* TPLLV2_VERS          should be 0x00 to conform to this specification */
/* TPLLV2_COMPLY        should be 0x00 to conform to this standard      */
/* TPLLV2_DINDEX        2byte signed index of first data byte in common */
/* TPLLV2_RSV6          reserved (should be 0x00)                       */
/* TPLLV2_RSV7          reserved (should be 0x00)                       */
/* TPLLV2_VSPEC         vendor specific (information should be ignored) */
/* TPLLV2_NHDR          number of CIS copies (should be 0x01 in this    */
/*                      version of the standard, since no specification */
/*                      is provided)                                    */
/* TPLLV2_OEM           is a NULL terminated manufacturer name string   */
/* TPLLV2_INFO          is a NULL terminated product information string */
/* Note: The "PCMCIA" claim they will maintain a registry of vendor     */
/*       names.                                                         */
/*----------------------------------------------------------------------*/
/* CISTPL_DATE                                                          */
/*                                                                      */
/* This structure encodes the date and time at which the memory CARD    */
/* was formatted.                                                       */

typedef struct CISTPL_encoded_TIME {
#if 1
                                    /* ANSI does not allow char bitfields */
                                    byte TIMElo_bitfield ;
                                    byte TIMEhi_bitfield ;                  
#else
                                    byte seconds : 5,
                                         minlo   : 3 ;
                                    byte minhi   : 3,
                                         hour    : 5 ;
#endif
                                   } CISTPL_encoded_TIME ;

/* byte 0 */
#define seconds_mask    (0x1F)
#define seconds_shift   (0)
#define minlo_mask      (0xE0)
#define minlo_shift     (5)
/* byte 1 */
#define minhi_mask      (0x07)
#define minhi_shift     (0)
#define hour_mask       (0xF8)
#define hour_shift      (3)

/* The seconds within a minute are calculated by "seconds * 2".         */
/* The minutes are calculated by "(minhi * 8) + minlo".                 */
/* The hours are held directly in the "hour" field.                     */

typedef struct CISTPL_encoded_DATE {
#if 1
                                    /* ANSI does not allow char bitfields */
                                    byte DATElo_bitfield ;
                                    byte DATEhi_bitfield ;                  
#else
                                    byte day     : 5,
                                         monthlo : 3 ;
                                    byte monthhi : 1,
                                         year    : 7 ;
#endif
                                   } CISTPL_encoded_DATE ;

/* byte 0 */
#define day_mask        (0x1F)
#define day_shift       (0)
#define monthlo_mask    (0xE0)
#define monthlo_shift   (5)
/* byte 1 */
#define monthhi_mask    (0x01)
#define monthhi_shift   (0)
#define year_mask       (0xFE)
#define year_shift      (1)

/* The day is held directly in the "day" field.                         */
/* The month is calculated by "(monthhi * 8) + monthlo".                */
/* The year is calculated by "year + 1980".                             */

typedef struct CISTPL_DATE_struct {
                                   CISTPL_encoded_TIME time ;
                                   CISTPL_encoded_DATE date ;
                                  } CISTPL_DATE_struct ;

/* If the initialisation time and date fields are both zero, then the   */
/* time of initialisation was not known when the CARD was formatted.    */
/*----------------------------------------------------------------------*/
/* CISTPL_BATTERY                                                       */
/*                                                                      */
/* This structure contains the two date structures. The first describes */
/* the last occasion when the battery in the CARD was replaced. The     */
/* second date structure encodes the date at which the battery should   */
/* next be replaced.                                                    */

typedef struct CISTPL_BATTERY_struct {
                                      CISTPL_encoded_DATE last ;
                                      CISTPL_encoded_DATE next ;
                                     } CISTPL_BATTERY_struct ;

/*----------------------------------------------------------------------*/
/*---- Level 2 ---- data recording format structures -------------------*/
/*----------------------------------------------------------------------*/
/* If the format of the memory CARD is disk-like then the               */
/* "CISTPL_FORMAT" structure may be followed by a "CISTPL_GEOMETRY"     */
/* structure.                                                           */
/* If the format of the memory CARD is memory-like then the             */
/* "CISTPL_FORMAT" structure may be followed by a "CISTPL_BYTEORDER"    */
/* structure.                                                           */
/*----------------------------------------------------------------------*/
/* CISTPL_FORMAT                                                        */
/*                                                                      */
/* This structure defines the data organisation of a specific region    */
/* (or partition) of the common memory.                                 */

typedef enum {
              disk_like         = 0x00,
              memory_like       = 0x01
             } TPLFMT_TYPE_codes ;
/* The codes 0x02->0x7F are reserved for future expansion. The codes    */
/* 0x80->0xFF are reserved for vendor specific definitions.             */

typedef enum {
              EDC_none          = 0,
              EDC_checksum      = 1,
              EDC_CRC           = 2,
              EDC_PCC           = 3
             } TPLFMT_EDC_codes ;
/* The codes 4->7 are reserved for future expansion. The codes 8->15    */
/* are reserved for vendor specific error detection definitions.        */
/* NOTE: "EDC_checksum" and "EDC_CRC" are only applicable to disk-like  */
/*       systems. "EDC_PCC" is only applicable to memory-like systems.  */
/* The "EDC_checksum" and "EDC_PCC" are simple additive sums (ignoring  */
/* overflow) and modulo-256.                                            */
/* The "EDC_CRC" is the "HDLC CRC" (otherwise known as the "CRC-CCITT"  */
/* or "SDLC" algorithm. The data to be checked is considered as a       */
/* serial bit-stream, with the low-order bit of the first byte taken as */
/* the first bit of the stream. This bit-stream is conceptually taken   */
/* as the coefficients of a polynomial in xn (where n is the number of  */
/* bits in the stream, and where the first bit is the coefficient of    */
/* the term x(n-1). This polynomial is divided (modulo 2) by the        */
/* polynomial x16 + x12 + x5 + 1, leaving a remainder of order 15 or    */
/* less (the initial remainder being set to all ones). The one's        */
/* complement of this remainder is the error check code; it is          */
/* recorded with the complemented coefficient of x15 as its LSB, and    */
/* with the complemented coefficient of x0 as its MSB.                  */
/* This algorithm also has a convenient property: when the check code   */
/* is appended to the data stream, and the algorithm is run on the      */
/* result, the remainder will always be:                                */
/*      x12 + x11 + x10 + x8 + x3 + x2 + x1 + x0                        */
/* assuming that neither the data or the CRC have been corrupted.       */

typedef struct TPLFMT_EDC_struct {
#if 1
                                  /* ANSI does not allow char bitfields */
                                  byte EDC_bitfield ;
#else
                                  byte EDC_len    : 3,  /* "disk-like" only */
                                       EDC_method : 4,  /* TPLFMT_EDC_codes */
                                       EDC_extend : 1 ; /* reserved */
#endif
                                 } TPLFMT_EDC_struct ;

#define EDC_len_mask            (0x07)
#define EDC_len_shift           (0)
#define EDC_method_mask         (0x78)
#define EDC_method_shift        (3)
#define EDC_extend_mask         (0x80)
#define EDC_extend_shift        (7)

typedef struct TPLFMT_memlike {
#if 1
                               /* ANSI does not allow char bitfields */
                               byte control_bitfield ;
#else
                               byte control_addr : 1,  /* physical address */
                                    control_auto : 1,  /* host address */
                                                 : 6 ; /* reserved bits */
#endif
                               byte reserved ;         /* should be 0x00 */
                               byte address[4] ;       /* as control bits */
                               byte EDC_PCC_result ;   /* when EDC_PCC used */
                               byte EDC_reserved[3] ;  /* should be 0x00 */
                              } TPLFMT_memlike ;

#define control_addr_shift      (0)
#define control_addr_mask       (1 << control_addr_shift)
#define control_auto_shift      (1)
#define control_auto_mask       (1 << control_auto_shift)

typedef struct TPLFMT_disklike {
                                byte block_size[2] ;   /* see below     */
                                byte nblocks[4] ;      /* # of blocks   */
                                byte EDC_location[4] ; /* see below     */
                               } TPLFMT_disklike ;
/* The "block_size" is a power-of-2 byte value between 128 and 2048.    */
/* The following ((nblocks * (block_size + EDC_len)) <= TPLFMT_NBYTES)  */
/* must be TRUE.                                                        */
/* The "EDC_location" if it exists (depends on "TPL_LINK" offset in the */
/* "TPL_hdr" for the "CISTPL_FORMAT_struct" structure), points to a     */
/* table of "nblocks" entries, each of "EDC_len" bytes.                 */
/* If the "EDC_location" does not exist, or the value is 0x00000000     */
/* then the the error check value is inter-leaved with the data blocks. */

typedef struct CISTPL_FORMAT_struct {
                                     byte TPLFMT_TYPE ; /* TPLFMT_TYPE_codes */
                                     byte TPLFMT_EDC ;  /* TPLFMT_EDC_struct */
                                     byte TPLFMT_OFFSET[4] ;
                                     byte TPLFMT_NBYTES[4] ;
                                     union {
                                            TPLFMT_memlike  memory_info ;
                                            TPLFMT_disklike disk_info ;
                                           } TPLFMT_EXTEND ;
                                    } CISTPL_FORMAT_struct ;

/*----------------------------------------------------------------------*/
/* CISTPL_GEOMETRY                     only used in "disk-like" systems */
/*                                                                      */
/* This structure is used for disk-like partitions when they need to be */
/* supported on host environments that treat all external storage       */
/* devices has having cylinders, tracks and sectors.                    */

typedef struct CISTPL_GEOMETRY_struct {
                                       byte nsecs ;     /* per track    */
                                       byte ntracks ;   /* per cylinder */
                                       byte ncyls[2] ;  /* in partition */
                                      } CISTPL_GEOMETRY_struct ;

/* The value ((ncyls * ntracks * nsecs) <= TPLFMT_NBYTES)               */

/*----------------------------------------------------------------------*/
/* CISTPL_BYTEORDER                  only used in "memory-like" systems */
/*                                                                      */
/* This structure is used to describe the memory layout for memory-like */
/* partitions.                                                          */

typedef enum {
              little_endian     = 0,
              big_endian        = 1
             } BYTEORDER_order ;
/* Values betweem 0x80 and 0xFF (inclusive) are vendor specific. All    */
/* other values are reserved for future use.                            */

typedef enum {
              byte0_LSB         = 0,
              byte0_MSB         = 1
             } BYTEORDER_mapping ;
/* Values betweem 0x80 and 0xFF (inclusive) are vendor specific. All    */
/* other values are reserved for future use.                            */

typedef struct CISTPL_BYTEORDER_struct {
                                        byte order ;    /* for numeric  */
                                        byte mapping ;  /* on data-bus  */
                                       } CISTPL_BYTEORDER_struct ;

/* If a CISTPL_BYTEORDER structure is not present then the default is   */
/* little-endian byte 0 as LSB.                                         */
/*----------------------------------------------------------------------*/
/*---- Level 3 ---- data organisation ----------------------------------*/
/*----------------------------------------------------------------------*/
/* CISTPL_ORG                                                           */
/*                                                                      */
/* This structure contains information about the logical organisation   */
/* of a memory partition.                                               */

typedef struct CISTPL_ORG_struct {
                                  byte data_org ;       /* see below    */
                                  byte description[1] ; /* NULL term.   */
                                 } CISTPL_ORG_struct ;

typedef enum {
              filesystem        = 0,    /* "description" of filesystem  */
              application       = 1,    /* "description" of application */
              executableROM     = 2     /* "description" of executable  */
             } ORG_codes ;
/* Values in the range 0x80 to 0xFF (inclusive) are vendor specific.    */
/* All other values are reserved for future use.                        */

/* Allocations: (currently known about)                                 */
/* type         description (0x00 terminated)   usage                   */
/* ---------------------------------------------------------------------*/
/* filesystem   "DOS"                           DOS FAT filesystem      */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*---- Level 4 ---- system/vendor specific information -----------------*/
/*----------------------------------------------------------------------*/

/**************** OUTSIDE THE SCOPE OF THIS HEADER FILE *****************/

/*----------------------------------------------------------------------*/
/*-- Active Book (vendor specific) information -------------------------*/
/*----------------------------------------------------------------------*/
/* CIS : structure codes 0x80 .. 0xFE					*/

/**** UNUSED ****/

/*----------------------------------------------------------------------*/
/* CISTPL_VERS_2 : 16bits of information				*/

/* We must ensure the "Vendor name" string matches our desired value.	*/
/* If it does we can interpret the vendor specific bytes in this	*/
/* structure.								*/
/*									*/
/* NOTE: The Level 1 product information structure (if present) will	*/
/* 	 probably encode information about the CARD manufacturer, and	*/
/*	 not the CARD initialiser/formatter.				*/
/*									*/
/* Since we have already identified ourselves as the initiator of this	*/
/* CARD we do not need any extra identification in these fields.	*/
/*									*/
/* First byte of vendor specific information :				*/

#define	AB_CARD	(0xAB)	/* Active Book Company "Active Book" hardware	*/
/* This value can be used to encode other Active Book Company products	*/
/* that may use the same CARD formats, but contain completely different	*/
/* internal hardware.							*/

/* Since we only have 8bits of information left, rather than encoding	*/
/* the information in bitfields (and hence having redundant bits for	*/
/* the majority of the life of this format) I have decided just to	*/
/* utilise 256 hardware/processor pair descriptions.			*/
typedef enum {
              AB_generic	= 0x00,	/* hardware not important	*/
              AB1_issue1	= 0x01	/* issue1 - Hercules 1		*/
             } ABC_hardware ;

/* The "TPLLV2_OEM" field contains a NULL terminated manufacturer ID	*/
#define ABOEM_identity	"Active Book Company"

/*----------------------------------------------------------------------*/
/* CISTPL_FORMAT : format types 0x80 .. 0xFF				*/

/**** UNUSED ****/

/*----------------------------------------------------------------------*/
/* CISTPL_FORMAT : EDC identifiers 0x08 .. 0x0F				*/

/**** UNUSED ****/

/*----------------------------------------------------------------------*/
/* CISTPL_BYTEORDER : order identifiers 0x80 .. 0xFF			*/

/**** UNUSED ****/

/*----------------------------------------------------------------------*/
/* CISTPL_BYTEORDER : mapping identifiers 0x80 .. 0xFF			*/

/**** UNUSED ****/

/*----------------------------------------------------------------------*/
/* CISTPL_ORG : organisation identifiers 0x80 .. 0xFF			*/

typedef enum {
              ROMITEM_structure	= 0x80,		/* RO filesystem	*/
	      RAMFS_structure	= 0x81,		/* RW filesystem	*/
	      HEAP_structure	= 0x82		/* volatile heap	*/
             } ABCORG ;

/* Unique identifier strings should also be included for these		*/
/* organisation identifiers. (see CISTPL_ORG info. above)		*/
#define ROMITEM_string	"ABCROMITEM"
#define RAMFS_string	"ABCRAMFS"
#define HEAP_string	"ABCHEAP"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#endif /* __PCcard_h */

/*----------------------------------------------------------------------*/
/*> EOF PCcard/h <*/
