        SUBT    AB Functional Prototype hardware description > hardABFP/s
        ; Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; Hardware description of the AB1 functional prototype
        ;
        ; started:      900411  BJKnight
        ;                       JGSmith (various modifications)
        ;
        ; This file describes the memory layout of the AB1 functional 
        ; prototype board
        ; ---------------------------------------------------------------------

old_opt SETA    {OPT}
        OPT     (opt_off)

        ; ---------------------------------------------------------------------

        !       0,"Including hardABFP.s"
                GBLL    hardABFP_s
hardABFP_s      SETL    {TRUE}

        ; ---------------------------------------------------------------------

        ; System clock frequency

        [       {TRUE}
clockfreq       *       32000000        ; Better timing emulation
        |
clockfreq       *       45000000
        ]

        ; ---------------------------------------------------------------------
        ; RAM and ROM positions

SRAM_base	*	&00000000
SRAM_size	*	(4 * 1024)	; 4K of static RAM

staticRAMblk    *       SRAM_base       ; 4K of static RAM 
dataRAMblk      *       &02000000       ; Main RAM (up to 4M)
videoRAMblk     *       &00740000       ; Screen memory (256K)
RAM_base	*	dataRAMblk
ROMbase         *       &03400000       ; Start of ROM (52MB)
ROM_base	*	ROMbase		; an alternative symbol for the ROM

        ; ---------------------------------------------------------------------

ROM_size        *       (512 * 1024)    ; size of the ROM in ABFPs

        ; ---------------------------------------------------------------------
        ; IO controllers
codec_base      *       &00400060       ; Codec data register
codec_constat   *       &00400071       ; Codec control/status
LINK1_base      *       &00480000       ; Transputer link 0
LINK0_base      *       &00500000       ; Transputer link 1 (with reset line)
inmos_link_base *       LINK0_base      ; so server can reset board
ml_link_base	*	LINK1_base	; microlink substitute
timer_base      *       &00580000       ; 82C54 interval timer
floppy_base     *       &00600000       ; 72068 floppy disc controller
serial_base     *       &00680000       ; 72001 serial interface
FLASH_base      *       &00700000       ; Flash EPROM

        ; ---------------------------------------------------------------------
        ; Memory management
mmu_mode        *       &00400030       ; Mode register
mmumap_base     *       &00200000       ; 16 segment mapping registers

        ; ---------------------------------------------------------------------
        ; Lights and switches
        ; Three of the `lights' bits are stolen for other purposes
switches_base   *       &00400000       ; 8 switches: 1 = off
LED_base        *       &00400000       ; Only 5 LEDs (ls 5 bits): 1 = off.
screen_mode     *       &00400000       ; 2 bits control screen display
flash_mode      *       &00400000       ; 1 bit does something to flash EPROM

        ; ---------------------------------------------------------------------
        ; LCD screen
        ; The size is 640*400 pixels. There are two bit planes and each
        ; line of each plane takes 128 bytes (of which only the first 80
        ; are displayed).
LCD_base                *       videoRAMblk     ; top left of physical display
LCD_displaywidth        *       80              ; display width
LCD_planewidth          *       128             ; plane width
LCD_planes              *       2               ; number of planes
LCD_height              *       400             ; number of rasters

                        ;       number of bytes in a complete raster
LCD_stride              *       (LCD_planes * LCD_planewidth)

                        ;       number of bytes in physical screen
LCD_size                *       (LCD_height * LCD_stride)

                        ;       end address of the physical screen
LCD_end                 *       (LCD_base + LCD_size)

        ; ---------------------------------------------------------------------
        ; Interrupt control
irq_mask        *       &00400012       ; Mask for IRQ sources. 1 = enabled.
fiq_mask        *       &00400021       ; Mask for FIQ sources. 1 = enabled.
int_source      *       &00400010       ; Interrupt source register

frame_intclr    *       &00400040       ; Write here to clear frame int
timer_intclr    *       &00400050       ; Write here to clear timer int

        ; ---------------------------------------------------------------------
        ; Interrupt enable flags
        ; ----------------------
        ; These have the same positions in the IRQ and FIQ enable registers.
        ; These are write-only registers: 0 disables interrupt, 1 enables it.
        ; The functional prototype has an incomplete interrupt source
        ; register in which the bit positions are different.

int_serial      *       2_00000001      ; Serial line interface
int_floppy      *       2_00000010      ; Floppy disc interface
int_codec_rx    *       2_00000100      ; Codec reception (D to A)
int_codec_tx    *       2_00001000      ; Codec transmission (A to D)
int_timer0      *       2_00010000      ; Timer counter 0
int_pod_frame   *       2_00100000      ; Podule or LCD frame interrupt
int_link0       *       2_01000000      ; Transputer link 0
int_link1       *       2_10000000      ; Transputer link 1

        ; Interrupt source bits. 
        ; These are 1 when device is trying to interrupt (even if int masked)

intsrc_timer    *       2_00000001      ; Timer
intsrc_frame    *       2_00000010      ; LCD frame (VSYNC) 
intsrc_ext      *       2_00000100      ; External source

        ; ---------------------------------------------------------------------
        ; Inmos link adapter
        ; ------------------
        ; Set to generate IRQs

        ; These registers are given as offsets from LINK0_base and LINK1_base
                ^       0
LINK_read       #       4               ; RO: last 8bits of received data
LINK_write      #       4               ; WO: buff for next 8bits of send data
LINK_rstatus    #       4               ; RW: data and input present interrupt
LINK_wstatus    #       4               ; RW: buffer empty/output interrupt

LINK_data       *       (1 :SHL: 0)     ; data present flag/buffer empty (set)
LINK_intenable  *       (1 :SHL: 1)     ; suitable interrupt enable

        ; ---------------------------------------------------------------------
        ; LED control register
        ; The functional prototype boards have 5 LEDs mapped to this byte

LED_bit0        *       (1 :SHL: 0)     ; 0=LED 0 on; 1=LED 0 off
LED_bit1        *       (1 :SHL: 1)     ; 0=LED 1 on; 1=LED 1 off
LED_bit2        *       (1 :SHL: 2)     ; 0=LED 2 on; 1=LED 2 off
LED_bit3        *       (1 :SHL: 3)     ; 0=LED 3 on; 1=LED 3 off
LED_bit4        *       (1 :SHL: 4)     ; 0=LED 4 on; 1=LED 4 off

        ; The other bits in this register

flash_vpp       *       (1 :SHL: 5)     ; 1: apply Vpp to flash EPROM
screen_plane    *       (1 :SHL: 6)     ; Select bit plane for mono screen
screen_grey     *       (1 :SHL: 7)     ; 1: show both planes as 4 grey levels

        ; ---------------------------------------------------------------------
        ; Switches

switch_bit0     *       (1 :SHL: 0)     ; 1=switch 8 open; 0=switch 8 closed
switch_bit1     *       (1 :SHL: 1)     ; 1=switch 7 open; 0=switch 7 closed
switch_bit2     *       (1 :SHL: 2)     ; 1=switch 6 open; 0=switch 6 closed
switch_bit3     *       (1 :SHL: 3)     ; 1=switch 5 open; 0=switch 5 closed
switch_bit4     *       (1 :SHL: 4)     ; 1=switch 4 open; 0=switch 4 closed
switch_bit5     *       (1 :SHL: 5)     ; 1=switch 3 open; 0=switch 3 closed
switch_bit6     *       (1 :SHL: 6)     ; 1=switch 2 open; 0=switch 2 closed
switch_bit7     *       (1 :SHL: 7)     ; 1=switch 1 open; 0=switch 1 closed

        ; ---------------------------------------------------------------------
        ; Serial interface
        ; Not used directly by Executive: see 72001 data sheet for details

        ; ---------------------------------------------------------------------
        ; FlashEPROM interface
        ; See "intel" 28F010 data sheet.

flash_write_retries     *       (25)    ; number of write retry attempts
flash_erase_retries     *       (1000)  ; number of erase retry attempts

flash_manufacturer      *       (&89)   ; INTEL
flash_device            *       (&B4)   ; 28F010 FlashEPROM

; Vpp high commands
flash_READ              *       (&00)   ; read data
flash_ERASE             *       (&20)   ; erase location
flash_WRITE             *       (&40)   ; write location
flash_READID            *       (&90)   ; read device identity
flash_ERASEVERIFY       *       (&A0)   ; verify erased location
flash_WRITEVERIFY       *       (&C0)   ; verify written location
flash_RESET             *       (&FF)   ; reset (abort) write/erase operation

flash_write_timeout     *       10      ; micro-seconds min (25 maximum)
flash_verify_timeout    *       6       ; micro-seconds min
flash_erase_timeout     *       10      ; milli-seconds (9.5 min <-> 10.5 max)

        ; maximum FlashEPROM size supported by the Active Book hardware
FLASH_size		*       (8 :SHL: 20)

        ; ---------------------------------------------------------------------
        ; Timer (82C54)
        ; -------------
        ; Only counter 0 is wired to generate interrupts
        ; BYTE wide registers, WORD spaced
                ^       0
timer_ctr0_data #       4               ; Counter 0 data (read/write)
timer_ctr1_data #       4               ; Counter 1 data (read/write)
timer_ctr2_data #       4               ; Counter 2 data (read/write)
timer_control   #       4               ; Control register (write only)

        ; Control register bits
timer_con_bcd   *       2_00000001      ; 1 for BCD mode, 0 for binary

timer_con_mode0 *       2_00000000      ; Interrupt on terminal count
timer_con_mode1 *       2_00000010      ; Hardware retriggerable one-shot
timer_con_mode2 *       2_00000100      ; Rate generator
timer_con_mode3 *       2_00000110      ; Square wave mode
timer_con_mode4 *       2_00001000      ; Software-triggered strobe
timer_con_mode5 *       2_00001010      ; Hardware-triggered strobe

timer_con_clc   *       2_00000000      ; Counter-latch command
timer_con_rwlsb *       2_00010000      ; Read/write ls byte only
timer_con_rwmsb *       2_00100000      ; Read/write ms byte only
timer_con_rwlm  *       2_00110000      ; Read/write ls then ms byte

timer_con_sc0   *       2_00000000      ; Select counter 0
timer_con_sc1   *       2_01000000      ; Select counter 1
timer_con_sc2   *       2_10000000      ; Select counter 2
timer_con_rbcmd *       2_11000000      ; Read-back command

        ; Control register has different format for read-back command
timer_con_rbsc0 *       2_00000010      ; Select counter 0
timer_con_rbsc1 *       2_00000100      ; Select counter 1
timer_con_rbsc2 *       2_00001000      ; Select counter 2
timer_con_rbns  *       2_00010000      ; Don't latch status of counters
timer_con_rbnc  *       2_00100000      ; Don't latch count of counters

        ; ---------------------------------------------------------------------
        ; Memory management registers

        ; Mode register

mmumode_mapen   *       2_100           ; Enable mapping (else physical map)
mmumode_umode   *       2_000           ; Set user mode
mmumode_osmode  *       2_010           ; Set OS mode
mmumode_mode0   *       2_000           ; Set OS mode 0 or user mode 0
mmumode_mode1   *       2_001           ; Set OS mode 1 or user mode 1

        ; Segment mapping registers

mmumap_basemask *       &0001FFFF       ; Physical page base field [A25:9]
mmumap_baseshft *       0               ; Left shift to align base field
mmumap_limmask  *       &1FFE0000       ; Page limit field [A20:9]
mmumap_limshft  *       17              ; Left shift to align limit field

mmumap_up       *       &00000000       ; Ascending segment
mmumap_down     *       &20000000       ; Descending segment

mmumap_rwe      *       &00000000       ; Read/write/execute access
mmumap_re       *       &40000000       ; Read/execute access
mmumap_r        *       &80000000       ; Read access
mmumap_noacc    *       &C0000000       ; No access

        ; ---------------------------------------------------------------------
        ; Codec interface (3057)
        ; This device is not used directly by the executive, but this is a
        ; good place to document the registers.

        ; Control register bits

codec_cr_npdl   *       2_00000001      ; Set when not pulse dialling
codec_cr_offhk  *       2_00000010      ; 0: phone on hook; 1: off hook
codec_cr_ntrs   *       2_00000100      ; 0: reset tx fifo
codec_cr_nrrs   *       2_00001000      ; 0: reset rx fifo
codec_cr_ten    *       2_00010000      ; 1: enable transmission (A to D)
codec_cr_ren    *       2_00100000      ; 1: enable reception (D to A)
codec_cr_mic    *       2_01000000      ; 0: phone i/p; 1: microphone i/p
codec_cr_spkr   *       2_10000000      ; 0: phone o/p; 1: speaker o/p

        ; Status register bits

;                       2_00000001      ; Unused
codec_sr_ntff   *       2_00000010      ; 1: tx fifo not full
codec_sr_nthf   *       2_00000100      ; 1: tx fifo less than half full
codec_sr_ntef   *       2_00001000      ; 1: tx fifo not empty
codec_sr_nrff   *       2_00010000      ; 1: rx fifo not full
codec_sr_nrhf   *       2_00100000      ; 1: rx fifo less than half full
codec_sr_nref   *       2_01000000      ; 1: rx fifo not empty
codec_sr_nri    *       2_10000000      ; 0: phone ringing (incoming call)

        ; ---------------------------------------------------------------------
        ; MMUMAP
        ; Requires "r0" to contain the address of the memory management
        ; segment registers. "r1" is corrupted during this MACRO call.
        ;
        ;       seg:                    segment number (0..15)
        ;
        ;       pages:                  number of 512byte chunks
        ;
        ;       paddr:                  512byte aligned physical address
        ;
        ;       access: NONE            no access
        ;               READ            read only
        ;               EXECUTE         read/execute
        ;               WRITE           read/execute/write
        ;
        ;       updown: UP
        ;               DOWN

        MACRO
$label  MMUMAP  $seg,$pages,$paddr,$access="WRITE",$updown="UP"
$label
        LCLA    value
value   SETA    &00000000
        [       (($paddr :AND: &1FF) = 0)
        [       (($paddr :AND: (&3F :SHL: 26)) = 0)
value   SETA    (value :OR: (($paddr :SHR: 9) :AND: mmumap_basemask))
        |
        ASSERT  (1 = 0) ; physical address is too large
        ]
        |
        ASSERT  (1 = 0) ; physical address is NOT multiple of 512 bytes
        ]
        [       ("$updown" = "DOWN")
value   SETA    (value :OR: mmumap_down :OR: ($pages :SHL: mmumap_limshft))
        |
        [       ("$updown" = "UP")
value   SETA    (value :OR: mmumap_up :OR: ((4096 - $pages) :SHL: mmumap_limshft))
        |
        ASSERT  (1 = 0) ; invalid updown specifier "$updown" (UP,DOWN)
        ]
        ]
        [       ("$access" = "NONE")
value   SETA    (value :OR: mmumap_noacc)
        |
        [       ("$access" = "READ")
value   SETA    (value :OR: mmumap_r)
        |
        [       ("$access" = "EXECUTE")
value   SETA    (value :OR: mmumap_re)
        |
        [       ("$access" = "WRITE")
value   SETA    (value :OR: mmumap_rwe)
        |
        ASSERT  (1 = 0) ; invalid access "$access" (NONE,READ,EXECUTE,WRITE)
        ]
        ]
        ]
        ]
        LDR     r1,=value               ; load "value" into r1
        STR     r1,[r0,#($seg * 4)]     ; and store into the correct register
        MEND

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF hardABFP/s
