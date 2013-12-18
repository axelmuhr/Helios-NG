/* $Header: prototol.h,v 1.1 91/01/07 17:31:49 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/obsolete/prototol.h,v $ */

/*------------------------------------------------------------------------*/
/*                                                       source/MLStub.h  */
/*------------------------------------------------------------------------*/

/* This protocol #define's the protocol information for communications    */
/*   over the microlink                                                   */

/* The microlink protocol is designed in such a way that the information  */
/*   which refers to message framing and so-forth is independent of the   */
/*   information pertaining to the functionality of the message. This     */
/*   allows the protocol functionality to be extended within the framing  */
/*   constraints without having to alter low-level software. In addition  */
/*   it allows later verions of software to communicate with earlier      */
/*   versions of software with fewer teething problems.                   */

/* The constraint of the protocol, that is the part of the protocol which */
/*    is implemented by lower-level software and to which upwards-        */
/*    compatibility should be repected is as follows:                     */

/* Each message consists of a sequence of bytes. The messages are divided */
/*    into three classes:                                                 */
/*                                                                        */
/* 1. Short:    Message and header consist of a single byte.              */
/* 2. Long:     Message and header comprise 2,3, or 5 bytes.              */
/* 3. Extended: Message and header comprise between 3 and 34 bytes.       */
/*                                                                        */
/* The short message consist of a byte whose highest bit is cleared, bits */
/*   6,5,4, and 3 are a field indicating the message type, and the bottom */
/*   three bits comprise further data about the message.                  */
/* The long message consists of a header byte whose upper bit is set to   */
/*   '1'. The next five bits as above encode the message type of long     */
/*   message and the bottom two bits are used to encode the total message */
/*   length:                                                              */
/*   00 => Two bytes including header                                     */
/*   01 => Three bytes including header                                   */
/*   10 => Five bytes including header                                    */
/* The extended message has a header byte the same format as for a long   */
/*  message except that the message types refer to different messages     */
/*  and the bottom two bits are set to 1. The next byte should have it's  */
/*  upper three bits set to zero (otherwise a framing error will occur)   */
/*  and the lower five bits are set to equal the total length of the      */
/*  message less two.                                                     */

/* To implement the microlink protocol, this server module calls routines */
/*   in the executive via. routines in abclib.                            */
/* The transmit process is straightforward: The message is placed in a    */
/*    buffer and passed to the executive: The executive infers from the   */
/*    first byte or two in the message it's length to transmit it. The    */
/*    buffer is transmitted and then the executive returns. If there      */
/*    was an error transmitting the message the executive returns a       */
/*    negative number and the server should try again.                    */
/* The receive process is slightly more complicated: In order to receive  */
/*    a message the server passes a 'type' to the executive, which is     */
/*    basically the first byte of the message expected back, except that  */
/*    in the case of a short message, the lower four bits are variable in */
/*    the message expected back. It also passes the address of a buffer   */
/*    in which to store the returned message. The executive prepares to   */
/*    place any message of that type into the buffer provided. It         */
/*    returns a handle. That handle can be used in another routine which  */
/*    blocks until a message is actually received of that type into the   */
/*    buffer supplied. The routines refered to above are called           */
/*    ML_SetUpRx and ML_WaitForRx.                                        */
/* There is another way to prepare to receive a message over the          */
/*    microlink more suited to continuous event stream type messages.     */
/*    The routine ML_RegisterHandler(...) can be called to register the   */
/*    address of a function which can be used to deal with incomming      */
/*    messages of a particular type. The handler is a function accpeting  */
/*    two parameters: A pointer to the buffer in the executive into which */
/*    the message has been read, and an arbitrary word 'arg' which was    */
/*    passed to the 'register handler' function which can, say, be used   */
/*    as a pointer to some form of context. The handler will be called in */
/*    SVC mode with interrupts off and so is very sensitive.              */
/* For each 'handler' registers a handle for it is returned, and it can   */
/*    be used in ML_DetatchHandler which is used to remove the handler    */
/*    from the queue, and can be called from without or within the        */
/*    handler.                                                            */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkProtocol_h
#define MicrolinkProtocol_h

/*------------------------------------------------------------------------*/
/*                                                      Types of message  */
/*------------------------------------------------------------------------*/

/* Since the low-level executive microlink driver is not concerned with   */
/*   details of the functions on the microlink protocol, it is safe and   */
/*   sensible to define the function codes here with the microlink server */
/*   where they can be augmented without hacking into the executive.      */

/* The messages are defined below. Some of the funciton codes may be      */
/*    repeated in the executive which does in fact have to recognise      */
/*    various events such as power down and so-forth (that is the reason  */
/*    why the low-level drivers were hacked into the executive in the     */
/*    first place.                                                        */

/*------------------------------------------------------------------------*/
/*                              Masking length-bits out of the type-byte  */
/*------------------------------------------------------------------------*/

/* It is safe when registering a handler or expecting a message from the  */
/*   executive to mask-out the bits indicating message length from the    */
/*   expected messsage header-byte before passing that type to the        */
/*   executive.                                                           */
/* This applies to message headers which are for long or extended         */
/*   messages: FOr short messages there is no length field.               */

#define MLMaskLength(h) ((h)&~0x03)

/*------------------------------------------------------------------------*/
/*                                             Transmitted short messages */
/*------------------------------------------------------------------------*/

/* Below are defined various ways of putting together single byte short   */
/*    messages for transmission from Hercules to the Microconroller       */

/* Ask Microcontroller to test itself. Reply MLYhermt.                    */
# define ASQhermt 0x10

/* After a break of the microcontroller line, the microcontroller sends   */
/*   an MSQbreak message, and we (Hercules) respond with this message     */
/*   The parameter to the macro should be '1' if we issued the break, 0   */
/*   otherwise. Actually, this sequence is dealt with at the level of the */
/*   executive functions.                                                 */
# define ASYbreak(who) (0x18|(who))

/* Call to read the internal temperature. Reply MLYtemp.                  */
# define ASQtemp  0x20

/* Call to turn on/off the speaker. Reply MSYack. (sw) should be 0 for    */
/*    'off' and 1 for 'on'.                                               */
# define ASQspkrc(sw) ((0x28)|(sw))

/* Call to turn the microphone on/off. Reply MSYack. As above.            */
# define ASQmicrc(sw) ((0x30)|(sw))

/* Call to enable the flash-EPROM programming voltage for a short period, */
/*   the time in seconds is supplied as the 3-bit parameter. If the three */
/*   bits are zero the call turns the voltage off.                        */
# define ASQflshv(tm) ((0x38)|(tm))

/* Call to request that the power be removed. When the microcontroller    */
/*    pwers-up Hercules again it will initiate a power-on reset.          */
# define ASQpores 0x40

/* Call to request the real-time clock value. Used to re-set the Hercules */
/*   clock when appropriate. Reply MLYrtclk.                              */
# define ASQrdrtc 0x48

/* Call to read the status of the phone interface. Reply MLYphist.        */
# define ASQphist 0x50

/* Call to request the status of the power supply. Reply MLYpsust.        */
# define ASQpsust 0x58

/* Call to turn the disc power supply on or off. 'sw' must be 1 for on or */
/*   0 for off.                                                           */
# define ASQdiscp(sw) ((0x60)|(sw))

/*------------------------------------------------------------------------*/
/*                                             Transmitted long messages  */
/*------------------------------------------------------------------------*/

/* The following macros are used to form long messages for transmission.  */
/* The first parameter to the macro forms the pointer to the buffer into  */
/*   which the message is to be written, and the next parameters describe */
/*   more about the message if required.                                  */

/* Call to assert the Hercules software version number to the             */
/*   microcontroller in response to a microcontroller MSQhercv reuest.    */
/* 'minv' should be the minor version number range 0..255. 'majv' should  */
/*   be the major version number range 0..31. 'tag' should be a tag which */
/*   is returned in the reply from the Microcontroller.                   */
# define ASYhercv(buf,maj,min,tag)           \
( (buf)[0] = 0x85,                           \
  (buf)[1] = min,                            \
  (buf)[2] = ((byte)(min)|((byte)(tag)<<5))  \
)

/* Call to assert sucess/failure of a hercules self-test process. The    */
/*   reply status should be zero for 'test sucessfull' and non-zero for  */
/*   'test failed'.                                                      */
# define ALYherct(buf,sta)       \
( (buf)[0] = 0x88,               \
  (buf)[1] = (sta)               \
)

/* Call  to set the contrast level in the LCD. Reply MSYack              */
/* (val) is the contrast. 0=>LCD off, 1=> low, 255=>high                 */
#define ALQcontr(buf,val)        \
( (buf)[0] = 0x8C,               \
  (buf)[1] = (val)               \
)

/* Call to set the brightness of the LCD. Not implemented on AB1 but    */
/*   never mind ...                                                     */
/* If implemented, val ranges from 0 (off) to 255 (bright)              */
#define ALQbrigh(buf,val)        \
( (buf)[0] = 0x90,               \
  (buf)[1] = (val)               \
)

/* Call to set the state of the user LEDs. These LED's are situated    */
/*   near the touchpad. The bits state are represented by 'sw0' and    */
/*   'sw1'. Reply 'MSYack'.                                            */
#define ALQuleds(buf,sw0,sw1)                   \
( (buf)[0] = 0x98,                              \
  (buf)[1] = (((byte)(sw0))|(((byte)(sw1))<<1)) \
)

/* Call to set the rate at which DRAM's should be refreshed.           */
/* Reply 'MSYack'                                                      */
#define ALQrfrsh(buf,rate)                     \
( (buf)[0] = 0x9C,                             \
  (buf)[1] = (rate)                            \
)

/* Call to set the 'wakeup' conditions, ie. if Hercules gets put to    */
/*   sleep, under what circumstances should it be woken up again.      */
/* Details on this call yet to be decided.                             */
/* Reply 'MSYack'                                                      */
#define ALQwupco(buf)                          \
( (buf)[0] = 0xA0/*|(size info)*/,             \
  (buf)[1] = /*data*/ 0                        \
  /* ... etc ... */                            \
)

/* Call to write data into the EEPROM. (addr) should be the address    */
/*   from which to start writing the data, this should be basically    */
/*   a short-word aligned address. (val) should be the 16-bit value to */
/*   write into the givenm address.                                    */
/* Reply 'MSYack'                                                      */
#define ALQwreep(buf,addr,val)                  \
( (buf)[0] = 0xA6,                              \
  (buf)[1] = (addr),                            \
  (buf)[2] = (byte)((val)&0xFF),                \
  (buf)[3] = (byte)((val)>>8),                  \
  (buf)[4] = 0                                  \
)

/* Call to read data from the EEPROM. (addr) should be the address     */
/*   of the short word (short-word-aligned) from which to read the     */
/*   short-word to be returned.                                        */
/* Reply MLYeeval                                                      */
#define ALQrdeep(buf,addr)                      \
( (buf)[0] = 0xA8,                              \
  (buf)[1] = (addr)                             \
)

/* Call to make a simple 'beep'-type sound from the speaker.           */
/* The nature of (freq) and (dur) are yet to be defined.               */
/* Reply MSYack.                                                       */
#define ALQsound(buf,freq,dur)                  \
( (buf)[0] = 0xB1,                              \
  (buf)[1] = (freq),                            \
  (buf)[2] = (dur)                              \
)

/* Call to set the stylus rate and resolution. This affects the rate   */
/*   at which the digitizer co-ordinates are scanned and the thinning  */
/*   amounts: 'rate' should be the maximum number of characters per    */
/*   second that the Microcontroller should report, whereas 'del'      */
/*   should be the minimum x- or y- movements before a  co-ordinate is */
/*   transmitted.                                                      */
#define ALQstylc(buf,rate,del)                  \
( (buf)[0] = 0xB1,                              \
  (buf)[1] = rate,                              \
  (buf)[2] = del                                \
)

/* Call to issue a command to the I2C bus. Details to be decided        */
#define ASQi2com(buf,b2)                        \
( (buf)[0] = (0xB4/*|(no of bytes)*/),          \
  (buf)[1] = b2                                 \
)

/* Drive the byte-on-demand codec interface                             */
/* Details to be decided.                                               */
#define ALQcodec(buf,b2)                       \
( (buf)[0] = (0xB8/*|(no of bytes)*/),         \
  (buf)[1] -= b2                               \
)

/* Call to take the phone off the hook for a moment ...                 */
/* If 'per' is non-zero then the phone receiver is 'lifted' by Hermes   */
/*    for 'per' seconds and then put down again. Hercules should        */
/*    send a new request before 'per' seconds are up if it wants to     */
/*    keep the receiver up: Thus the receiver is automatically 'put     */
/*    down' if Hercules needs to power-down for some reason.            */
/* If 'per' is zero the receiver is 'put-down'                          */
#define ALQhook(buf,per)                       \
( (buf)[0] = 0xBC,                             \
  (buf)[1] = per                               \
)

/* Call to initiate dialling of a number on the phone-line. The reply  */
/*   MSYack is sent possibly before dialling is complete. The buffer   */
/*   numbers to be dialled should be entered into the buffer by using  */
/*   the second macro with the position in the list first and the      */
/*   digit to be entered second. All positions 0 .. 7 must be filled   */
/*   and specify numbers in sequence. ALQdialPad is the digit value    */
/*   corresponding to 'don't dial a digit' in order to specify shorter */
/*   numbers. ALQdialPause is used in place of a digit to indicate a   */
/*   four-second pause in the dialling. Longer numbers can be acheived */
/*   by stringing requests together. Poll the phone-line status with   */
/*   ASQphist to find out when the dialling is finished.               */
/* There are two codes that can be used as the message header, the     */
/*   first initiates pulse-dialing and the second initiates tone-      */
/*   dialing.                                                          */
#define ALQdialp 0xC2
#define ALQdialt 0xC6
#define ALQdialpPutDigit(buf,pos,dig)                             \
( (pos&1) ? ( (buf)[(pos)>>1]&=0xF0, (buf)[(pos)>>1]|=(dig)       \
          : ( (buf)[(pos)>>1]&=0x0F, (buf)[(pos)>>1]|=((dig)<<4)  \
)
/* Use 0-9 for 'dig' to indicate digits to be dialled. Otherwise use   */
/*    to following numbers for special codes:                          */
#define ALQdialPad   0x0F
#define ALQdialPause 0x0A

 
/* Call to set the time stored in the real-time-clock. The time to set */
/*    it to is to be the number of seconds elapsed since the start of  */
/*    1970. It should be a 32-bit unsigned number                      */
#define ALQstrtc(buf,secs)                    \
( (buf)[0] = 0xCA,                            \
  (buf)[1] = ((secs)>> 0)&0xFF,               \
  (buf)[2] = ((secs)>> 8)&0xFF,               \
  (buf)[3] = ((secs)>>16)&0xFF,               \
  (buf)[4] = ((secs)>>24)*0xFF                \
)

/*------------------------------------------------------------------------*/
/*                                               Received short messages  */
/*------------------------------------------------------------------------*/

/* There is a macro for each message type-code which should be passed to  */
/*   the low-level executive software in order to indicate that it is     */
/*   that particular type of message that is expected. The nessage type   */
/*   basically gives the encoded message byte-code with the three bits    */
/*   corresponding to the returned message data set to zero: The macro    */
/*   MSQvalue() will return the value of this three-bit code.             */

#define MSQvalue(byt) ((byt)&0x03)

/* Message to request the version number of the Hercules software.        */
/*    Hercules should reply with the message ALYhercv.                    */
#define MSQhercv 0x08

/* Message to activate the Hercules self-test. Hercules should reply with */
/*   the message ALYherct                                                 */
#define MSQherct 0x10

/* Message sent immediately after a break operation: Hercules responds    */
/*   with ASYbreak. The message value will be 0 if the Microcontroller    */
/*   denies issuing the break, 1 otherwise.                               */
/* This message should be dealt with by the low-level executive functions */
#define MSQbreak 0x18

/* Message sent to aknowledge an earlier Hercules request. Most Hercules  */
/*   messages expect the following reply in order to aknowledge the       */
/*   request.                                                             */
/* The three-bits returned in the value field encoded as given in the     */
/*   macros which come afterwards.                                        */
/* Hercules should expect one of these replys within 20mS of sending the  */
/*   request.                                                             */
#define MSYack        0x20
#define MSYackSuccess 0x00 /* 000 => Sucess with operation                */
#define MSYackInvalid 0x01 /* 001 => Parameters in request were no good   */
#define MSYackRetry   0x02 /* 010 => Try again with same message soon     */
#define MSYackFailed  0x03 /* 011 => Operation failed and futile          */

/* Power failure alarm message. The microcontroller send this signal      */
/*   within 35mS of Hercules being forced off due to power failure.       */
/* The executive is likely to trap this message and immediately save the  */
/*   registers into RAM somewhere and wait for the fatal moment.          */
#define MSEpfail 0x18

/* Incomming phone call detected. This is issued by the microcontroller   */
/*   when it notices the phone ringing. Hercules will typically then      */
/*   answer the phone using the 'take the phone off the hook' type of     */
/*   message and initiate the CODEC to sample the incomming noises.       */
#define MSEiring 0x30

/* Card door is being opened/closed. The microcontroller issues this      */
/*   request whenever the card door gets opened or closed. The value in   */
/*   the value filed of the reply is 0 when the card door is closed, and  */
/*   1 when the card door is opened.                                      */
#define MSEcdoor 0x38

/* Case top is lowered/closed message. The microcontroller issues this    */
/*   message whenever the state of the case door changes. The value       */
/*   returned is 1 when raised, 0 when lowered.                           */
#define MSEtopsw 0x40

/* The following message is issued whenever the test-switch changes state */
/* The value field is 0 if off, 1 if on.                                  */
#define MSEtstsw 0x48

/*------------------------------------------------------------------------*/
/*                                                Received long messages  */
/*------------------------------------------------------------------------*/

/* For each type of long-message to be received, the macros give the      */
/*   type code of the message. Also provided are macros to read the       */
/*   message information out of the receive buffer when the message has   */
/*   been received.                                                       */

/* Hermes self-test result message. The microcontroller sends this result */
/*   in response to the ASQhermt message. The status is 0 if the test was */
/*   sucessful, and non-zero if the test failed, in which case it returns */
/*   some sort of error status information.                               */
#define MLYhermt            0x88
#define MLYhermtResult(buf) ((buf)[1])

/* Internal temperature response. The microcontroller returns this        */
/*   message in response to the ASQtemp message. The byte value returned  */
/*   is the temperature encoded in some way.                              */
#define MLYtemp             0x8C
#define MLYtempValue(buf)   ((buf)[1])

/* Value read from EEPROM device. This message is returned in response    */
/*    to the 'ALQrdeep' message. It contains the value read from the      */
/*    EEPROM.                                                             */
#define MLYeeval            0x94
#define MLYeevalValue(buf)  (((buf)[1])|(((buf)[2])<<8))

/* Internal keyboard key event. The following message is sent when a key  */
/*   is pressed or released on the internal keyboard. '...Pos' is 1       */
/*   if the key is now down or 0 if the key is now up. '...Cod' is the    */
/*   microcontroller-assigned code for the key in question.               */
#define MLEexkey            0x98
#define MLEexkeyPos(buf)    (!!((buf)[1]&0x80))

/* Mouse movement/button events: The following event will be implemented  */
/*   whenever the microcontroller happens to implement a mouse interface: */
/*   The X and Y values are relative movements since the last time a      */
/*   packet was transmitted.                                              */
#define MLEmouse             0xA2
#define MLEmouseXmov(buf)    ((signed char)((buf)[1]))
#define MLEmouseYmov(buf)    ((signed char)((buf)[2]))
#define MLEmouseButtons(buf) ((signed char)((buf)[3]))
/* The button state is encoded as follows: */
/* 000 => No change                        */
/* 001 => Left   button released           */
/* 010 => Left   button pressed            */
/* 100 => Middle button released           */
/* 101 => Middle button pressed            */
/* 110 => Right  button released           */
/* 111 => Right  button pressed            */

/* Touch pad position reporting event: This event is regularly sent to   */
/*   report the current position recorded on the touchpad, unless the    */
/*   'pressed' flag is off, in which case it reports the last position   */
/*   prevalent when the touch-pad was being touched.                     */
#define MLEtppos                            0xA5
#define MLEtpposPressed(buf) (!!((buf)[1]&0x80))
#define MLEtpposXpos(buf)    ((buf)[1]&0x1F)
#define MLEtpposYpos(buf)    ((buf)[2]&0x1F)


/* Touch pad button up/down event: This event is sent to report whenever */
/*   one of the buttons near the touchpad has been pressed or released.  */
/*   The 'Position' is 0 for 'button released' and 1 for 'button pressed'*/
/*   The 'Button' is the code for the particular button pressed/released */
#define MLEtpbut              0xA8
#define MLEtpbutPosition(buf) (!!((buf)[1]&0x80))
#define MLEtpbutButton(buf)   ((buf)[1]&0x07)

/* Real time colck value reply: The Microcontroller sends this reply to  */
/*   ASQrdrtc from Hercules. The result consists of the real-time clock  */
/*   value as measured in seconds elapsed since the beggining of the     */
/*   year 1970.                                                          */
#define MLYrtclk                0xAE
#define MLYrtclkTime(buf) \
( ((buf)[1]) | ((buf)[2]<<8) | ((buf)[3]<<16) | ((buf)[4]<<24) )

/* Phone interface status response. This message is issued as a response */
/*  to the ASQphist request and provides details of the current          */
/*  operations on the phone-line. The format of this package has yet to  */
/*  be decided.                                                          */
#define MLYphist 0xB0

/* Power supply status reply. This message is issued by the micro-       */
/*   controller in response to the request ASQpsust. It indicates the    */
/*   current status of the power supply.                                 */
/* The subsequent symbols are as follows:                                */
/* FlashVol : Is a voltage being supplied to the flash EPROM             */
/* NicadChg : Are the NICAD batteries being charged at full rate         */
/* DiscOn   : Is power being supplied to the floppy disc drive           */
/* ExtPwr   : Is external power being supplied                           */
/* LithPwr  : Are lithium batteries supplied                             */
/* Battery  : What type of main-bnatteries are supplied ...              */
/* NoBattery      :   No     batteries loaded                            */
/* NicadBattery   : Nicad    batteries loaded                            */
/* DryCellBattery : Dry cell batteries loaded                            */
/* LithState : State of the lithium batteries (if present)               */
/* MainState : State of the main (nicad or dry) batteries (if present)   */
/* The battery state is represented on a scale of 255 down to 0 rating   */
/*    from fully charged down to exhausted. The microcontroller may have */
/*    to make appropriate estimations of these values depending on what  */
/*    it knows.                                                          */
#define MLYpsust 0xB6
#define MLYpsustFlashVol(buf) (!!((buf)[1]&0x40))
#define MLYpsustNicadChg(buf) (!!((buf)[1]&0x20))
#define MLYpsustDiscOn(buf)   (!!((buf)[1]&0x10))
#define MLYpsustExtPwr(buf)   (!!((buf)[1]&0x08))
#define MLYpsustLithPwr(buf)  (!!((buf)[1]&0x04))
#define MLYpsustBattery(buf)  (!!((buf)[1]&0x03))
#define MLYpsustNoBattery                    0x00
#define MLYpsustNicadBattery                 0x01
#define MLYpsustDryCellBattery               0x02
#define MLYpsustLithState(buf) (!!((buf)[2]))
#define MLYpsustMainSTate(buf) (!!((buf)[3]))


/* Major Microcontroller error event: If something goes wrong in the     */
/*   Microcontroller that the software can detect, and it can still send */
/*   events down the microlink, then it sends the following event        */
/*   containing a 32-bit error code, which Hercules software should then */
/*   display in some way or other.                                       */
#define MLEerror 0xBA
#define MLEerrorCode(buf) \
((buf)[1]|((buf)[2]<<8)|((buf)[3]<<16)|((buf)[4]<<24)

/*------------------------------------------------------------------------*/
/*                                                      End of Interlock  */
/*------------------------------------------------------------------------*/

#endif

