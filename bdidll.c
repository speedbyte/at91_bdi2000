/*************************************************************************
|  COPYRIGHT (c) 2000 BY ABATRON AG
|*************************************************************************
|
|  PROJECT NAME: BDI Communication Driver
|  FILENAME    : bdilnk.c
|
|  COMPILER    : GCC
|
|  TARGET OS   : LINUX
|  TARGET HW   : PC
|
|  PROGRAMMER  : Abatron / RD
|  CREATION    : 27.03.00
|
|*************************************************************************
|
|  DESCRIPTION :
|  Data link functions for communication with BDI
|
|
|*************************************************************************
|
|
|  UPDATES     :
|
|  DATE      NAME  CHANGES
|  -----------------------------------------------------------
|  Latest update
|  ...
|  13.13.97  aba   Bla bla ...
|  ...
|  First update
|
|*************************************************************************/

/*************************************************************************
|  INCLUDES
|*************************************************************************/

#include <unistd.h>
#include <sys/times.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>

#include "bdierror.h"
#include "bdicmd.h"
#include "bdidll.h"


/*************************************************************************
|  DEFINES
|*************************************************************************/

#define DLE     16
#define STX      2
#define ETX      3

/* Frame control */
#define FRAME_COUNT_FIELD               (3<<6)
#define FRAME_LENGTH_MASK               7
#define FRAME_TYPE_MASK                 (7<<3)
#define FRAME_LNK_TYPE                  (0<<3)
#define FRAME_ATT_TYPE                  (1<<3)
#define FRAME_STD_TYPE                  (2<<3)

/* link management commands */
#define LNK_RESET                       1
#define LNK_ECHO                        2
#define LNK_SET_BAUDRATE                3


/*************************************************************************
|  MACROS
|*************************************************************************/

/*************************************************************************
|  TYPEDEFS
|*************************************************************************/

typedef struct {BOOL    connected;
                int     fd;
                DWORD   asynBaudrate;
                BYTE    frameType;
                BYTE    frameCount;
                DWORD   repeatCount;
               } BDI_ChannelT;


/*************************************************************************
|  LOCALS
|*************************************************************************/

/* channel info */
static  BDI_ChannelT    channelInfo;

/* frame buffers */
static  BYTE            txFrame[BDI_MAX_FRAME_SIZE];
static  BYTE            rxFrame[BDI_MAX_FRAME_SIZE];


/****************************************************************************
 ****************************************************************************

    BDI_Append___ :

    Host independent helper function to append a numeric value to a buffer.
    The bytes will be stored in the link format order (Motorola byte order).

     INPUT  : value     value
              buffer    pointer to buffer
     OUTPUT : RETURN    pointer to next byte after the stored value
     INOUT  :

 ****************************************************************************/

static BYTE* BDI_AppendLong(DWORD value, BYTE* buffer)
{
  *buffer++ = (BYTE)(value>>24);
  *buffer++ = (BYTE)(value>>16);
  *buffer++ = (BYTE)(value>>8);
  *buffer++ = (BYTE)value;
  return buffer;
} /* BDI_AppendLong */


/****************************************************************************
 ****************************************************************************

    BDI_Extract___ :

    Host independent helper function to extract a numeric value from a buffer.
    The value must be stored in link format (Motorola byte order).

     INPUT  : buffer    pointer to buffer
     OUTPUT : value     pointer to variable where to store the value
              RETURN    pointer to next byte after the extracted value
     INOUT  :

 ****************************************************************************/

static BYTE* BDI_ExtractLong(DWORD* value, BYTE* buffer)
{
  DWORD x;
  x = (DWORD)*buffer++;
  x = (x<<8) + (DWORD)*buffer++;
  x = (x<<8) + (DWORD)*buffer++;
  x = (x<<8) + (DWORD)*buffer++;
  *value = x;
  return buffer;
} /* BDI_ExtractLong */


/****************************************************************************
 ****************************************************************************
    BDI_DoDelay
    Helper function to delay an amount of time

     INPUT:  delay      the delay time in ms
     OUTPUT: -
 ****************************************************************************/

void BDI_DoDelay(DWORD delay)
{
  usleep(1000 * delay);
} /* BDI_DoDelay */


/****************************************************************************
 ****************************************************************************
    BDI_IPAddrMotorola
    BDI_IPAddrIntel
    Helper function to convert an IP address.

     INPUT:  ipAddress      the IP address as string (e.g. "151.120.25.101")
     OUTPUT: RETURN         the IP address as DWORD
 ****************************************************************************/

DWORD BDI_IPAddrMotorola(const char* ipAddress)
{
  DWORD     netAddr;
  char      number[4];
  char      c;
  int       fields;
  int       digits;
  int       value;

  /* get the four numbers */
  netAddr = 0;
  fields  = 0;
  for (;;) {
    if (fields == 4) return INADDR_NONE;
    digits = 0;
    c = *ipAddress++;
    while (isdigit(c)) {
      if (digits < 3) {
        number[digits++] = c;
      } /* if */
      else return INADDR_NONE;
      c = *ipAddress++;
    } /* while */
    number[digits] = 0;
    value = atoi(number);
    if ((value < 0) || (value > 255)) return INADDR_NONE;
    netAddr <<= 8;
    netAddr += (DWORD)value;
    fields++;
    if ((c != '.') && (c != 0)) return INADDR_NONE;
    if (c == 0) break;
  } /* while */
  if (fields != 4) return INADDR_NONE;
  return netAddr;
} /* BDI_IPAddrMotorola */


DWORD BDI_IPAddrIntel(const char* ipAddress)
{
  DWORD     netAddr;
  char      number[4];
  char      c;
  int       fields;
  int       digits;
  int       value;

  /* get the four numbers */
  netAddr = 0;
  fields  = 0;
  for (;;) {
    if (fields == 4) return INADDR_NONE;
    digits = 0;
    c = *ipAddress++;
    while (isdigit(c)) {
      if (digits < 3) {
        number[digits++] = c;
      } /* if */
      else return INADDR_NONE;
      c = *ipAddress++;
    } /* while */
    number[digits] = 0;
    value = atoi(number);
    if ((value < 0) || (value > 255)) return INADDR_NONE;
    netAddr >>= 8;
    netAddr += ((DWORD)value << 24);
    fields++;
    if ((c != '.') && (c != 0)) return INADDR_NONE;
    if (c == 0) break;
  } /* while */
  if (fields != 4) return INADDR_NONE;
  return netAddr;
} /* BDI_IPAddrIntel */


/****************************************************************************
 ****************************************************************************
                Asynchronous Communication Functions
 ****************************************************************************
 ****************************************************************************/

/****************************************************************************
    SetBaudrate

     INPUT:  channel        pointer to channel info
             baudrate       sets the baudrate of the communication port
     OUTPUT: return         error code
 ****************************************************************************/

static int AsynSetBaudrate(BDI_ChannelT* channel, DWORD baudrate)
{
  struct termios        tios;
  speed_t               speed;

  /* get terminal attributes */
  if (tcgetattr(channel->fd, &tios) < 0) return BDI_ASYN_SETUP;

  /* set new baudrate */
  channel->asynBaudrate = baudrate;
  if      (baudrate ==   9600) speed = B9600;
  else if (baudrate ==  19200) speed = B19200;
  else if (baudrate ==  38400) speed = B38400;
  else if (baudrate ==  57600) speed = B57600;
  else if (baudrate == 115200) speed = B115200;
  else                         speed = B9600;
  cfsetospeed(&tios, speed);
  cfsetispeed(&tios, speed);

  /* set terminal attributes */
  if (tcsetattr(channel->fd, TCSAFLUSH, &tios) < 0) return BDI_ASYN_SETUP;

  return BDI_OKAY;
} /* AsynSetBaudrate */


/****************************************************************************
    Flush the receive queue of the communication port

     INPUT:  channel    pointer to channel info
     OUTPUT:
 ****************************************************************************/

static void AsynFlushRxQueue(BDI_ChannelT* channel)
{
  tcflush(channel->fd, TCIFLUSH);
} /* AsynFlushRxQueue */


/****************************************************************************
    AsynOpen
    Opens the asynchronous communication channel

     INPUT:  channel        pointer to channel info
             port           the communication port e.g. "COM1"
     OUTPUT: return         error code
 ****************************************************************************/

static int AsynOpen(BDI_ChannelT* channel, const char* port)
{
  int                   fd;
  struct termios        tios;

  /* open device */
  fd = open(port, O_RDWR | O_NONBLOCK | O_NOCTTY);
  if (fd < 0) {
    perror("Error opening serial device");
    if (errno == EACCES) {
      fprintf(stderr, "Root permissions may be required to open %s\n", port);
    } /* if */
    return BDI_ASYN_SETUP;
  } /* if */
  channel->fd = fd;

  /* get terminal attributes */
  if (tcgetattr(fd, &tios) < 0) return BDI_ASYN_SETUP;

  /* set terminal attributes */
  channel->asynBaudrate = 9600;
  tios.c_iflag = 0;
  tios.c_oflag = 0;
  tios.c_cflag = CLOCAL | CREAD | CS8;
  tios.c_lflag = 0;
  tios.c_cc[VMIN]  = 0;
  tios.c_cc[VTIME] = 1;
  cfsetospeed(&tios, B9600);
  cfsetispeed(&tios, B9600);
  if (tcsetattr(fd, TCSAFLUSH, &tios) < 0) return BDI_ASYN_SETUP;

  return BDI_OKAY;
} /* AsynOpen */


/****************************************************************************
    Closes the communication port

     INPUT:  channel    pointer to channel info
     OUTPUT:
 ****************************************************************************/

static void AsynClose(BDI_ChannelT* channel)
{
  close(channel->fd);
} /* AsynClose */


/****************************************************************************
    Reads a byte from the communication port

     INPUT:  channel        pointer to channel info
             timeout        timeout in milliseconds
     OUTPUT: c              received byte
             return         0 = okay, else error
 ****************************************************************************/

static int AsynReadChar(BDI_ChannelT* channel, BYTE *c, DWORD timeout)
{
  struct tms    tbuf;
  DWORD         startTime;

  if (read(channel->fd, c, 1) == 1) return BDI_OKAY; /* high speed exit */
  timeout = (timeout * sysconf(_SC_CLK_TCK)) / 1000L;
  startTime = times(&tbuf);
  for (;;) {
    if ((times(&tbuf) - startTime) > timeout)            return BDI_ASYN_RX_TIMEOUT;
    if (read(channel->fd, c, 1) == 1)                    return BDI_OKAY;
  }
} /* AsynReadChar */


/****************************************************************************
    Writes a block to the communication port

     INPUT:  channel        pointer to channel info
             count          number of bytes to send
             data           the data to send
     OUTPUT: return         0 = okay, else error

 ****************************************************************************/

static int AsynWriteBlock(BDI_ChannelT* channel, int count, BYTE* data)
{
  int written;

  while (count > 0) {
    written = write(channel->fd, data, count);
    if (written > 0) {
      count -= written;
      data  += written;
    } /* if */
    else if ((written == -1) && (errno == EAGAIN)) {
      continue;
    } /* else if */
    else {
      return BDI_ASYN_TX_ERROR;
    } /* else */
  } /* while */
  return BDI_OKAY;
} /* AsynWriteBlock */


/****************************************************************************
    Sends a BDI frame

     INPUT:  channel        pointer to channel info
             count          number of bytes to send
             frame          the frame to send
     OUTPUT: return         0 = okay, else error

     OUTPUT:
 ****************************************************************************/

static int AsynSendFrame(BDI_ChannelT* channel, int count, BYTE* frame)
{
  BYTE  bcc;
  BYTE  txChar;
  BYTE* buffer;
  BYTE  asynTxBuffer[BDI_MAX_FRAME_SIZE];

  buffer = asynTxBuffer;

  /* send start sequence */
  *buffer++ = DLE;
  *buffer++ = STX;

  /* send frame data */
  bcc = 0;
  while (count--) {
    txChar    = *frame++;
    *buffer++ = txChar;
    bcc      ^= txChar;
    if (txChar == DLE) *buffer++ = DLE;
  } /* while */

  /* send end sequence */
  *buffer++ = DLE;
  *buffer++ = ETX;
  *buffer++ = bcc;
  if (bcc == DLE) *buffer++ = DLE;

  /* send prepared frame */
  return AsynWriteBlock(channel, buffer - asynTxBuffer, asynTxBuffer);
} /* AsynSendFrame */


/****************************************************************************
    Gets a BDI frame

     INPUT:  channel        pointer to channel info
             count          the maximal number of byte to receive
             timeout        the maximal time to wait for the frame in ms
     OUTPUT: frame          the received frame
             return         the size of the received frame or error

     OUTPUT:
 ****************************************************************************/

static int AsynWaitFrame(BDI_ChannelT* channel, int count, BYTE* frame, DWORD timeout)
{
  BYTE  bcc;
  BYTE* putPtr;
  int   result;
  int   rxCount;
  BYTE  rxChar;

  /* wait for start sequence */
  for (;;) {
    result = AsynReadChar(channel, &rxChar, timeout);
    if (result == BDI_ASYN_RX_TIMEOUT) return BDI_ASYN_RX_TIMEOUT;
    if ((result == BDI_OKAY) && (rxChar == DLE)) {
      result = AsynReadChar(channel, &rxChar, timeout);
      if (result == BDI_ASYN_RX_TIMEOUT) return BDI_ASYN_RX_TIMEOUT;
      if ((result == BDI_OKAY) && (rxChar == STX)) break;
    } /* if */
  } /* for */

  /* receive frame data */
  bcc        = 0;
  rxCount    = 0;
  putPtr     = frame;
  for (;;) {

    /* get next char */
    result = AsynReadChar(channel, &rxChar, timeout);
    if (result != BDI_OKAY) return result;

    /* process link escape char */
    if (rxChar == DLE) {
      result = AsynReadChar(channel, &rxChar, timeout);
      if (result != BDI_OKAY) return result;

      /* process end sequence */
      if (rxChar == ETX) {
        result = AsynReadChar(channel, &rxChar, timeout);
        if (result != BDI_OKAY) return result;
        if (rxChar == DLE) {
          result = AsynReadChar(channel, &rxChar, timeout);
          if (result != BDI_OKAY) return result;
          if (rxChar != DLE)      return BDI_ASYN_RX_FORMAT;
        } /* if */
        if (rxChar == bcc) return rxCount;
        else               return BDI_ASYN_RX_BCC;
      } /* if */
      else if (rxChar != DLE) return BDI_ASYN_RX_FORMAT;
    } /* if */

    /* store data in receive buffer */
    if (rxCount < count) {
      bcc      ^= rxChar;
      *putPtr++ = rxChar;
      rxCount++;
    } /* if */
    else return BDI_ASYN_RX_OVERFLOW;
  } /* for */

} /* AsynWaitFrame */


/****************************************************************************
    Reset Link
    Search the current baudrate, reset the link and set new baudrate.

     INPUT:  channel        pointer to channel info
             baudrate       the requested baudrate
     OUTPUT: return         error code

     OUTPUT:
 ****************************************************************************/

#define NBR_OF_BAUDRATES     5

/* SLIP special codes */
#define SLIP_END        192
#define SLIP_ESC        219
#define SLIP_ESC_END    220
#define SLIP_ESC_ESC    221
#define SLIP_ESC_ABA    222     /* activate BDI-HS loader */

static int AsynResetLink(BDI_ChannelT* channel, DWORD baudrate)
{
  static const DWORD rateTable[NBR_OF_BAUDRATES] = {9600,19200,38400,57600,115200};

  int   topRate;
  int   rate;
  int   rxCount;
  int   txCount;
  int   result;
  BYTE* txPtr;
  BYTE* rxPtr;
  BYTE  slipEsc[2];
  DWORD confirmedRate;

  /* check if 115200 is supported */
  topRate = NBR_OF_BAUDRATES - 1;
  if (AsynSetBaudrate(channel, rateTable[topRate]) != BDI_OKAY) topRate--;

  /* adjust requested rate if too high */
  if (baudrate > rateTable[topRate]) baudrate = rateTable[topRate];
  result = AsynSetBaudrate(channel, baudrate);

  /* send SLIP escape */
  slipEsc[0] = SLIP_ESC;
  slipEsc[1] = SLIP_ESC_ABA;
  if (result == BDI_OKAY) result = AsynWriteBlock(channel, sizeof slipEsc, slipEsc);
  BDI_DoDelay(300);

  /* try to connect with requested baudrate */
  txPtr    = txFrame;
  *txPtr++ = FRAME_LNK_TYPE;
  *txPtr++ = 1;
  *txPtr++ = LNK_RESET;
  txCount  = txPtr - txFrame;
  rxCount  = 0;
  if (result == BDI_OKAY) result  = AsynSendFrame(channel, txCount, txFrame);
  if (result == BDI_OKAY) rxCount = AsynWaitFrame(channel, sizeof rxFrame, rxFrame, 200);
  if (rxCount == txCount) return BDI_OKAY;

  /* try all baudrates */
  for (rate=0; rate<=topRate; rate++) {
    result = AsynSetBaudrate(channel, rateTable[rate]);
    if (result == BDI_OKAY) result  = AsynWriteBlock(channel, sizeof slipEsc, slipEsc);
    BDI_DoDelay(300);
    if (result == BDI_OKAY) result  = AsynSendFrame(channel, txCount, txFrame);
    if (result == BDI_OKAY) rxCount = AsynWaitFrame(channel, sizeof rxFrame, rxFrame, 200);
    if (rxCount == txCount) break;
  } /* for */
  if (rxCount != txCount) return BDI_ERR_NO_RESPONSE;

  /* change baudrate */
  txPtr    = txFrame;
  *txPtr++ = FRAME_LNK_TYPE;
  *txPtr++ = 5;
  *txPtr++ = LNK_SET_BAUDRATE;
  txPtr    = BDI_AppendLong(baudrate, txPtr);
  txCount  = txPtr - txFrame;
  rxCount  = 0;
  result = AsynSendFrame(channel, txCount, txFrame);
  if (result == BDI_OKAY) rxCount = AsynWaitFrame(channel, sizeof rxFrame, rxFrame, 200);
  if (rxCount != txCount) return BDI_ASYN_SETUP;

  /* give BDI time to change baudrate */
  BDI_DoDelay(300);

  /* extract confirmed baudrate */
  rxPtr = BDI_ExtractLong(&confirmedRate, rxFrame+3);

  /* check if success */
  txPtr    = txFrame;
  *txPtr++ = FRAME_LNK_TYPE;
  *txPtr++ = 1;
  *txPtr++ = LNK_RESET;
  txCount  = txPtr - txFrame;
  rxCount  = 0;
  result = AsynSetBaudrate(channel, confirmedRate);
  if (result == BDI_OKAY) result  = AsynSendFrame(channel, txCount, txFrame);
  if (result == BDI_OKAY) rxCount = AsynWaitFrame(channel, sizeof rxFrame, rxFrame, 200);
  if (rxCount == txCount) return BDI_OKAY;

} /* AsynResetLink */


/****************************************************************************
 ****************************************************************************
                Common Communication Functions
 ****************************************************************************
 ****************************************************************************/


/****************************************************************************
 ****************************************************************************

    BDI_Open:

     Opens the connection to the BDI.

     INPUT  : port          a string with the port name (e.g. /dev/com1)
              baudrate      the baudrate to connect
     OUTPUT : RETURN        0 if okay or a negativ number if error

 ****************************************************************************/

int BDI_Open(const char* port, DWORD baudrate)
{
  int           result;

  result = AsynOpen(&channelInfo, port);
  if (result != BDI_OKAY) return result;
  result = AsynResetLink(&channelInfo, baudrate);
  if (result != BDI_OKAY) {
    AsynClose(&channelInfo);
    return result;
  } /* if */

  channelInfo.connected     = TRUE;
  channelInfo.frameCount    = 0;
  channelInfo.repeatCount   = 0;
  channelInfo.frameType     = FRAME_STD_TYPE;
  return BDI_OKAY;
} /* BDI_Open */


/****************************************************************************
 ****************************************************************************

    BDI_Close:

     Closes the connection to the BDI.

     INPUT  : -
     OUTPUT : -

 ****************************************************************************/

void BDI_Close(void)
{
  if (channelInfo.connected) {
    AsynClose(&channelInfo);
    channelInfo.connected = FALSE;
  } /* if */
} /* BDI_Close */


/****************************************************************************
 ****************************************************************************

    BDI_Transaction:

     Executes a command / answer transaction with the BDI
     commandTime: The time the command needs to execute, not the transfer time.
                  The time is used to calculate the timeout before the command
                  is repeated.

     INPUT  : commandLength the length of the command block
              commandData   the command <code,parameter>
              answerSize    the size of the answer buffer
              commandTime   the time in ms the command needs to execute
     OUTPUT : answerData    the answer <code,parameter>
              RETURN        the length of the answer block
                            or a negativ number if error.

 ****************************************************************************/

int  BDI_Transaction(      int       commandLength,
                     const void     *commandData,
                           int       answerSize,
                           void     *answerData,
                           DWORD     commandTime)
{
        BYTE*           framePtr;
  const BYTE*           commandPtr;
        BYTE*           answerPtr;
        BYTE            frameControl;
        int             txFrameLength;
        int             rxFrameLength;
        int             repeats;
        int             result;
        int             rxCount;
        DWORD           answerTimeout;

  if (!channelInfo.connected) return BDI_ERR_NOT_CONNECTED;

  /* build frame */
  if (commandLength > (sizeof txFrame - 2)) return BDI_ERR_INVALID_PARAMETER;
  txFrameLength = commandLength + 2;
  framePtr      = txFrame;
  frameControl  = (BYTE)(channelInfo.frameType | (commandLength>>8));
  frameControl |= (channelInfo.frameCount<<6);
  channelInfo.frameCount++;

  commandPtr  = commandData;
  *framePtr++ = frameControl;
  *framePtr++ = (BYTE)commandLength;
  while (commandLength--) *framePtr++ = *commandPtr++;

  /* do transaction */
  repeats   = 0;
  while (repeats < 3) {

    /* send command frame */
    AsynFlushRxQueue(&channelInfo); /* attention frames will be repeated by BDI */
    result = AsynSendFrame(&channelInfo, txFrameLength, txFrame);
    repeats++;

    /* wait for answer */
    if (result == BDI_OKAY) {
      answerTimeout = (DWORD)(txFrameLength + 1500);  /* total number of characters */
      answerTimeout = answerTimeout * 10000L / channelInfo.asynBaudrate + commandTime + 200L;
      rxFrameLength = AsynWaitFrame(&channelInfo, sizeof rxFrame, rxFrame, answerTimeout);

      /* check received frame */
      if (rxFrameLength > 0) {
        rxFrameLength -= 2;
        rxCount = 256 * (rxFrame[0] & FRAME_LENGTH_MASK) + rxFrame[1];
        if (rxFrameLength == rxCount) {
          if (rxFrameLength > answerSize) return BDI_ERR_ANSWER_TOO_BIG;
          framePtr  = rxFrame + 2;
          answerPtr = answerData;
          while (rxFrameLength--) *answerPtr++ = *framePtr++;
          return rxCount;
        } /* if */
        channelInfo.repeatCount++;
      } /* if */

      else {
        channelInfo.repeatCount++;
      } /* if */

    } /* if */
  } /* while */

  return BDI_ERR_NO_RESPONSE;
} /* BDI_Transaction */

