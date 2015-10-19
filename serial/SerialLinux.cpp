static const char *RcsId = "$Header: /users/chaize/newsvn/cvsroot/Communication/SerialLine/src/SerialLinux.cpp,v 1.10 2013-04-03 09:27:29 jensmeyer Exp $";
//+=============================================================================
//
// file :         SerialLinux.cpp
//
// description : THIS FILE CONTAINS ALL THE METHODS FOR
//
//						LINUX PLATFORM
//
// project :      TANGO Device Server
//
// $Author: jensmeyer $
//
// $Revision: 1.10 $
//
// $Log: not supported by cvs2svn $
// Revision 1.9  2010/12/07 10:07:55  pascal_verdier
// Replace int by unsigned int to remove warnings.
//
// Revision 1.8  2010/12/07 09:08:49  pascal_verdier
// Moved to Pogo-7 code generator.
//
// Revision 1.7  2010/02/03 13:25:23  pascal_verdier
// State management added for linux.
//
//
//-=============================================================================
//=====================================================
//
//			THIS FILE CONTAINS ALL THE METHODS FOR
//
//						LINUX PLATFORM
//
//=====================================================

#include <Serial.h>

//+------------------------------------------------------------------
/**
*      method: Serial::open_desc
*
*      description:    Open the device desciptors and update member variable
*
*/
//+------------------------------------------------------------------
void Serial::open_desc ( void )
{
 const char  *dd_name;
 int         desc;
 char        tab[]="Serial::open_desc(): ";

 //
 // create the associated device descriptor filename
 //
 dd_name = this->serialdevice.serialline;
 INFO_STREAM << tab << "serial line to be opened " << dd_name << endl;

 //
 // open the serial line once for reading but not as a controlling tty
 //
 desc = open(dd_name,O_RDONLY | O_NOCTTY);
 if (desc < 0)
 {
  _success = false;
  set_state(Tango::FAULT);

  TangoSys_MemStream out_stream;
  out_stream << "error opening " << dd_name
             << " in O_RDONLY, errno=" << errno
             << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_open",
        out_stream.str(),
        (const char *)tab);
 }
 this->serialdevice.serialin = desc;

 //
 // open the serial line once for writing but not as a controlling tty
 //
 desc = open(dd_name,O_WRONLY | O_NOCTTY);
 if (desc < 0)
 {
  _success = false;
  set_state(Tango::FAULT);
  TangoSys_MemStream out_stream;
  out_stream << "error opening " << dd_name
             << "in O_WRONLY, errno=" << errno
             << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_open",
        out_stream.str(),
        (const char *)tab);
 }
 this->serialdevice.serialout = desc;

 _success = true;
 set_state(Tango::OPEN);
 return;
}

//+------------------------------------------------------------------
/**
*	method:	Serial::dev_ser_set_parameter_linux
*
*	description:	method to execute "DevSerSetParameter"
*	Set serial line parameters
*
* @param	argin	device parameters in pair
*
*/
//+------------------------------------------------------------------
void Serial::dev_ser_set_parameter_linux(const Tango::DevVarLongArray *argin)
{
 struct termios         termin;
 struct termios         termout;
 unsigned int           i;

 short                  timeout;
 float                  timeoutl;
 short                  parity;
 unsigned int           par;
 short                  charlength;
 unsigned int           cs;
 short                  stopbits;
 unsigned int           stopb;
 long                   baudrate;
 speed_t                speed;
 char                   tab[]="Serial::dev_ser_set_parameter(): ";



 INFO_STREAM << tab << "entering... !" << endl;

 INFO_STREAM << "   " << "param = new value" << endl;
 for(i=0; i<argin->length(); i+=2)
  INFO_STREAM << "   " << std::ios::dec << (*argin)[i] << " = "
        << std::ios::dec << (*argin)[i+1] << endl;

 //
 // First get the existing serial line setup from the file descriptors opened
 // for input and for output
 //
 if (tcgetattr(this->serialdevice.serialin, &termin) < 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "tcgetattr() error on in line, errno: " << errno
             << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_tcgetattr",
        out_stream.str(),
        (const char *)tab);
 }


 if (tcgetattr(this->serialdevice.serialout, &termout) < 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "tcgetattr() error on out line, errno: " << errno
             << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_tcgetattr",
        out_stream.str(),
        (const char *)tab);
 }

 //
 // Setup default arbitrary behaviour:
 //
 //     TODO: seems dangerous to start from the current settings instead
 //     of from blank settings, some flags not set here may be forced from
 //     the ouside using stty.
 //

        //
        // Avoid any side effect as the two descriptors have been opened
        // on the same physical device.
        //
 termin.c_oflag         = 0;
 termout.c_iflag        = 0;

        //
        // Read return immediately
        //
 termin.c_cc[VMIN]      = 0;
 termin.c_cc[VTIME]     = 0;

 termout.c_cc[VMIN]     = 0;    // should not be needed for out line
 termout.c_cc[VTIME]    = 0;

        //
        // Non canonical mode, i.e. raw mode
        //
 termin.c_lflag         &= ~ICANON;
 termout.c_lflag        &= ~ICANON;     // should not be needed for out line

        //
        // Do not echo
        //
 termin.c_lflag         &= ~(ECHO | ECHOCTL | ECHONL | ECHOE | ECHOKE | ECHOK);
 termout.c_lflag        &= ~(ECHO | ECHOCTL | ECHONL | ECHOE | ECHOKE | ECHOK);

        //
        // Do not activate implementation-dependent processing of characters
        //
 termin.c_lflag         &= ~(IEXTEN);
 termout.c_lflag        &= ~(IEXTEN);

        //
        // Control characters don't cause corresponding signals
        // (INTR -> SIGINT, QUIT -> SIGQUIT, SUSP -> SIGTSTP)
        //
 termin.c_lflag         &= ~ISIG;
 termout.c_lflag        &= ~ISIG;       // should not be needed for out line

        //
        // Ignore modem control lines
        //
 termin.c_cflag         |= CLOCAL;
 termout.c_cflag        |= CLOCAL;

        //
        // Allow receiving characters
        //
 termin.c_cflag         |= CREAD;

        //
        // Ignore hardware flow control (RTS and CTS lines)
        //
 termin.c_cflag         &= ~(CRTSCTS | HUPCL);
 termout.c_cflag        &= ~(CRTSCTS | HUPCL);

        //
        // Ignore break conditions received
        //
 termin.c_iflag         |= IGNBRK;
 termin.c_iflag         &= ~BRKINT;

        //
        // Ignore bytes received with parity error
        //
 termin.c_iflag         |= IGNPAR;
 termin.c_iflag         &= ~INPCK;

        //
        // XON/XOFF switched off
        //
 termin.c_iflag         &= ~(IXON | IXOFF);

        //
        // Do not convert upper-case characters to lower-case ones and
        // vice-versa
        //
 termin.c_iflag         &= ~IUCLC;
 termout.c_oflag        &= ~OLCUC;

        //
        // Do not convert \n -> \r or \r -> \n and do not ignore \r
        //
 termin.c_iflag         &= ~ICRNL;
 termin.c_iflag         &= ~INLCR;
 termin.c_iflag         &= ~IGNCR;
 termout.c_oflag        &= ~ONOCR;

        //
        // Do not convert \n -> \r+\n or \r -> \n
        //
 termout.c_oflag        &= ~ONLCR;
 termout.c_oflag        &= ~OCRNL;

        //
        // Do not force column to 0 if \r or \n are sent
        //
 termout.c_oflag        &= ~ONLRET;

        //
        // Do not expand tabs to 8 spaces
        //
 termout.c_oflag        &= ~XTABS;

        //
        // Delete character forced to zero
        //
 termin.c_cc[VERASE]    = 0;    // not used in non canonical mode
 termout.c_cc[VERASE]   = 0;    // not used in non canonical mode

        //
        // Reprint line character forced to zero
        //
 termin.c_cc[VERASE]    = 0;    // not used in non canonical mode
 termin.c_cc[VREPRINT]  = 0;    // not used in non canonical mode

 termout.c_cc[VERASE]   = 0;    // not used in non canonical mode
 termout.c_cc[VREPRINT] = 0;    // not used in non canonical mode

 //
 // Read device parameters passed
 //
 for (i=0; i<argin->length(); i=i+2)
 {
  switch ((*argin)[i])
  {
   case SL_TIMEOUT :
        //
        // Setup the serial line for timeouts if timeout > 0
        //
        // Only modifies the timeout for the file descriptor opened
        // for input
        //
        // The units of this timeout are milli-seconds.
        // Under Linux the internal units are 1/10th of a second, so
        // a conversion has first to be done.
        //
        timeout = (*argin)[i+1];
        if (timeout > 0)
        {
         timeoutl = ((float)timeout * 10.0) / 1000.0;

         //
         // In fact, the timeout on read() calls is not handle using
         // this VTIME parameter but using the select() command.
         //
         termin.c_cc[VTIME]  = (cc_t)timeoutl;

                // should not be done, yet done previously
         this->serialdevice.timeout = timeout;
         INFO_STREAM << "Serial::SerSetParameter(): timeout=" << timeout
               << "(mS) " << float (termin.c_cc[VTIME]) << "(1/10s)"
               << endl;
        }
        break;

   case SL_PARITY :
        //
        // setup up parity according to input parameter
        //
        parity = (*argin)[i+1];
        switch(parity)
        {
         case SL_EVEN   : par = PARENB;            break;
         case SL_ODD    : par = (PARENB | PARODD); break;
         case SL_NONE   : par = 0;                 break;
         default        : par = 0;                 break;
        }

        termin.c_cflag  &= ~PARENB;     // erase previous settings
	termin.c_cflag  &= ~PARODD;
        termin.c_cflag  |= par;

        termout.c_cflag &= ~PARENB;     // erase previous settings
	termout.c_cflag &= ~PARODD;
        termout.c_cflag |= par;

        INFO_STREAM << "Serial::SerSetParameter(): parity=" << parity
              << "(none=" << SL_NONE
              << " odd="  << SL_ODD
              << " even=" << SL_EVEN << ") code=" << par << endl;
        break;

   case SL_CHARLENGTH :
        //
        // setup up no. of data bits per character according to input parameter
        //
        charlength = (*argin)[i+1];
        switch(charlength)
        {
         case SL_DATA5  : cs = CS5; break;
         case SL_DATA6  : cs = CS6; break;
         case SL_DATA7  : cs = CS7; break;
         case SL_DATA8  : cs = CS8; break;
         default        : cs = CS8; break;
        }

        termin.c_cflag  &= ~CSIZE;      // erase previous settings
        termin.c_cflag  |= cs;

        termout.c_cflag &= ~CSIZE;      // erase previous settings
        termout.c_cflag |= cs;


        INFO_STREAM << "Serial::SerSetParameter(): charlength=" << charlength
              << "(data8=" << SL_DATA8
              << " 7=" << SL_DATA7
              << " 6=" << SL_DATA6
              << " 5=" << SL_DATA5 << ") code=" << cs << endl;
        break;

   case SL_STOPBITS :
        //
        // setup up no. of stop bits per character according to input parameter
        //
        stopbits = (*argin)[i+1];
        switch(stopbits)
        {
         case SL_STOP1  : stopb = 0;      break;
         case SL_STOP2  : stopb = CSTOPB; break;
         default        : stopb = 0;      break;
        }

        termin.c_cflag  &= ~CSTOPB;     // erase previous settings
        termin.c_cflag  |= stopb;

        termout.c_cflag &= ~CSTOPB;     // erase previous settings
        termout.c_cflag |= stopb;

        INFO_STREAM << "Serial::SerSetParameter(): stopbits=" << stopbits
              << "(stop1=" << SL_STOP1
              << " stop2=" << SL_STOP2 << ") code=" << stopb << endl;
        break;

   case SL_BAUDRATE :
        //
        // convert BAUDRATE to speed_t values understood by termios structure
        //
        baudrate = (*argin)[i+1];
        switch (baudrate)
        {
         case 4000000 : speed =B4000000; break;
         case 3500000 : speed =B3500000; break;
         case 3000000 : speed =B3000000; break;
         case 2500000 : speed =B2500000; break;
         case 2000000 : speed =B2000000; break;
         case 1500000 : speed =B1500000; break;
         case 1152000 : speed =B1152000; break;
         case 1000000 : speed =B1000000; break;
         case 921600  : speed = B921600; break;
         case 576000  : speed = B576000; break;
         case 500000  : speed = B500000; break;
         case 460800  : speed = B460800; break;
         case 230400  : speed = B230400; break;
         case 115200  : speed = B115200; break;
         case 57600   : speed =  B57600; break;
         case 38400   : speed =  B38400; break;
         case 19200 : speed = B19200; break;
         case 9600  : speed =  B9600; break;
         case 4800  : speed =  B4800; break;
         case 2400  : speed =  B2400; break;
         case 1800  : speed =  B1800; break;
         case 1200  : speed =  B1200; break;
         case 600   : speed =   B600; break;
         case 300   : speed =   B300; break;
         case 150   : speed =   B150; break;
         case 134   : speed =   B134; break;
         case 110   : speed =   B110; break;
         case 75    : speed =    B75; break;
         case 50    : speed =    B50; break;

         default    :
            TangoSys_MemStream out_stream;
            out_stream << "invalid baudrate" << ends;

			ERROR_STREAM << tab << out_stream.str() << endl;
            Tango::Except::throw_exception(
               (const char *)"Serial::error_argin",
               out_stream.str(),
               (const char *)tab);
        }

        termin.c_cflag  &= ~CBAUD;      // erase previous speed settings
        termin.c_cflag  |= speed;

        termout.c_cflag &= ~CBAUD;      // erase previous speed settings
        termout.c_cflag |= speed;

        INFO_STREAM << "Serial::SerSetParameter(): baud rate=" << baudrate
              << " code=" << speed << endl;
        break;

   case SL_NEWLINE :
        //
        // Setup new NEWLINE character
        //
        // This character settings is ignore in non canonical mode, i.e
        // raw mode (current mode used here, see above ICANON)
        //
        termin.c_cc[VEOL]  = (char)(*argin)[i+1];
        termout.c_cc[VEOL] = (char)(*argin)[i+1];

        INFO_STREAM << "Serial::SerSetParameter(): newline="
              << int (termin.c_cc[VEOL]) << endl;

        // Update object current settings
        this->serialdevice.newline = (char)(*argin)[i+1];
        break;

   default :

        break;

  } // end of switch((*argin)[i])
 }  // end of for(i)

 //
 // Now do it ! i.e. modify the open device descriptor
 //
 if ((tcsetattr(this->serialdevice.serialin, TCSAFLUSH, &termin)) < 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "tcsetattr() error on in line, errno: " << errno
             << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_tcsetattr",
        out_stream.str(),
        (const char *)tab);
 }



 if ((tcsetattr(this->serialdevice.serialout, TCSAFLUSH, &termout)) < 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "tcsetattr() error on out line, errno: " << errno
             << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_tcsetattr",
        out_stream.str(),
        (const char *)tab);
 }

}

//+------------------------------------------------------------------
/**
*	method:	Serial::dev_status
*
*	description:	method to execute "DevStatus"
*	Command to read the device status.
*	The method dev_status() override a DeviceImpl class virtual method.
*
* @return	Status descrition
*
*/
//+------------------------------------------------------------------
Tango::ConstDevString Serial::dev_status()
{
	struct termios termin;
	struct termios termout;
	static char    append[128];
	static char    mess[4096];
	static char    **status;
	const char 		*str;
	//TODO: TEMPORARY MODIFICATION FOR DEBUG
	//char         eol = '\r';     // end of line used for returned message
	char           eol = '\n';     // end of line used for returned message


	//	POGO has generated a method core with argout allocation.
	//	If you would like to use a static reference without copying,
	//	See "TANGO Device Server Programmer's Manual"
	//		(chapter 5.3)
	//------------------------------------------------------------
	/* TODO: why the code generated by POGO gives a compilation error
	Tango::DevString	argout = Tango::DeviceImpl::dev_status();
	*/
	INFO_STREAM << "Serial::dev_status(): entering... !" << endl;

	//	Add your own code to control device here
  if ( !_success )
  {
    set_state(Tango::FAULT);
    return "Failed to open serial com port.";
  }
  else
  {

	  // Compose string to return with serial configuration parameters
	  sprintf(mess,"The serial line has following configuration : %c%c", eol, eol);

	  if (this->serialdevice.serialline != NULL)
	  {
		  sprintf(append,"descriptor=%s%c",this->serialdevice.serialline, eol);
	  }
	  else
	  {
		  sprintf(append,"descriptor=none%c", eol);
	  }
	  strcat(mess,append);

	  // Current parameters defined in the internal structure
	  sprintf(append,"%cParameters defined internally:%c", eol, eol);
	  strcat(mess,append);

	  sprintf(append,"timeout=%d%c",this->serialdevice.timeout, eol);
	  strcat(mess,append);

	  sprintf(append,"baudrate=%ld%c",this->serialdevice.baudrate, eol);
	  strcat(mess,append);

	  switch(this->serialdevice.parity)
	  {
		  case SL_EVEN: str = "even"; break;
		  case SL_ODD : str = "odd";  break;
		  case SL_NONE: str = "none"; break;
		  default     : str = "????"; break;
	  }
	  sprintf(append,"parity=%s%c", str, eol);
	  strcat(mess,append);

	  switch(this->serialdevice.charlength)
	  {
		  case SL_DATA5: str = "5"; break;
		  case SL_DATA6: str = "6"; break;
		  case SL_DATA7: str = "7"; break;
		  case SL_DATA8: str = "8"; break;
		  default      : str = "?"; break;
	  }
	  sprintf(append,"charlength=%s%c",str, eol);
	  strcat(mess,append);

	  switch(this->serialdevice.stopbits)
	  {
		  case SL_STOP1: str = "1"; break;
		  case SL_STOP2: str = "2"; break;
		  default      : str = "?"; break;
	  }
	  sprintf(append,"stopbits=%s%c",str, eol);
	  strcat(mess,append);

	  sprintf(append,"newline=%d%c",this->serialdevice.newline, eol);
	  strcat(mess,append);

	  // Current parameters of the input terminal
	  // (see file /usr/include/asm/termbits.h for values decoding)
	  sprintf(append,"%cParameters of the input terminal:%c", eol, eol);
	  strcat(mess,append);

	  // TODO : the same for win32 !
	  if (tcgetattr(this->serialdevice.serialin, &termin) < 0)
	  {
		  TangoSys_MemStream out_stream;
		  out_stream << "tcsetattr() error on in line, errno: " << errno
			  << ends;

		  INFO_STREAM << "Serial::dev_ser_set_parameter(): ";
		  INFO_STREAM << out_stream.str() << endl;
		  Tango::Except::throw_exception(
			  (const char *)"Serial::error_tcgetattr",
			  out_stream.str(),
			  (const char *)"Serial::dev_status");
	  }
	  strcat(mess,decode_parameters(termin, eol));

	  // Current parameters of the output terminal
	  // (see file /usr/include/asm/termbits.h for values decoding)
	  sprintf(append,"%cParameters of the output terminal:%c", eol, eol);
	  strcat(mess,append);

	  if (tcgetattr(this->serialdevice.serialin, &termout) < 0)
	  {
		  TangoSys_MemStream out_stream;
		  out_stream << "tcsetattr() error on out line, errno: " << errno
			  << ends;

		  INFO_STREAM << "Serial::dev_ser_set_parameter(): ";
		  INFO_STREAM << out_stream.str() << endl;
		  Tango::Except::throw_exception(
			  (const char *)"Serial::error_tcgetattr",
			  out_stream.str(),
			  (const char *)"Serial::dev_status");
	  }
	  strcat(mess,decode_parameters(termout, eol));

    set_state(Tango::OPEN);
  }
	// Return the string
	Tango::DevString argout = mess;
	return argout;
}

//+------------------------------------------------------------------
/**
*      method: Serial::retry_read_string
*
*      description:
*	     read a string from the serialline device in mode raw,
*			 if first reading attempt successfull, retry to read "nretry"
*      times; if no more data found exit on timeout without error.
*	     Useful in case of long strings with no fixed lenght ( > 64 bytes)
*			 Very unlucky case!!!
*      The maximum number of characters that can be read is
*      SL_MAXSTRING.
*
*	@param	argin: number N of retries
* @return String read

*/
//+------------------------------------------------------------------
char *Serial::retry_read_string(long nretry)
{
 char 		*argout;
 fd_set		watchset;	// file descriptor set
 fd_set		inset;		// file descriptor set updated by select()
 int		maxfd;		// maximum file descriptor used
 int retrycnt;		// retry reading counter
 struct timeval	timeend;	// current time + timeout
 struct timeval	timeout;	// timeout value
 float		timeout_s;	// timeout value in seconds
 struct timezone tz;		// not used
 int		readyfd;	// number of file descriptors ready to be read
 int		ncharin; 	// number of characters in the receiving buffer
 char   tab[]="Serial::retry_read_string: ";
 int nchar;

 INFO_STREAM << tab << "entering... !" << endl;

 //
 //first "empty" buffer by NULL terminating it
 //
 this->serialdevice.buffer[0] = 0;
 this->serialdevice.ncharread = 0;

 //
 // Initialize the set, no file descriptors contained
 //
 FD_ZERO(&watchset);

 //
 // Add to the set the file descriptor to watch at
 //
 FD_SET(this->serialdevice.serialin, &watchset);

 // set retry counter to 0
 retrycnt = 0;
 nchar = SL_MAXSTRING;
 //
 // Initialize the timeout (calculate when the timeout should expire)
 //
 timeout_s = ((float)this->serialdevice.timeout) / 1000.0; // seconds

 gettimeofday(&timeend,&tz);
 timeend.tv_usec += (int)((timeout_s - (int)timeout_s) * 1000000.0);
 timeend.tv_sec  += (int)(timeout_s);
 if(timeend.tv_usec > 1000000)
 {
  timeend.tv_usec -= 1000000;
  timeend.tv_sec  += 1;
 }

 //
 // Wait until the receiving buffer contains the requested number of characters
 //
 do
 {
  //
  // Set the maximum file descriptor used, this avoid the system to look
  // through all the file descriptors that a Linux process can have (up to 1024)
  //
  maxfd = this->serialdevice.serialin + 1;

  //
  // As select() will update will update the set, make a copy of it
  //
  inset = watchset;

  //
  // Set the timeout value (take into account time consumed by previous
  // wait periods)
  //
  gettimeofday(&timeout,&tz);
  timeout.tv_usec = timeend.tv_usec - timeout.tv_usec;
  timeout.tv_sec  = timeend.tv_sec  - timeout.tv_sec;
  if(timeout.tv_usec < 0)
  {
   timeout.tv_usec += 1000000;
   timeout.tv_sec  -= 1;
  }

	//
  // Check if the timeout occured
  // With reasonnable values this should not happen, but on overloaded
  // systems with a short timeout value it may happens that the timeout
  // expired at this point before the call to select()
  //
  if(timeout.tv_sec < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read (sched)"
              << ends;
   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_timeend",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Block until characters become available on the file descriptor
  // listed in the set.
  //
  readyfd = select(maxfd, &inset, NULL, NULL, &timeout);

	// don't return error if something has been read from serial line
	if ((readyfd <= 0) && (retrycnt > 0))
		break;

	//
  // Check if an error occured
  //
  if (readyfd < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read (select() < 0)"
              << ends;
   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_select",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Check if the timeout occured
  //
  if (readyfd == 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read"
              << ends;
   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_select",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Check if it's our file descriptor which is ready to be read (Should be, as
  // it was the only watched one out).
  //
  if (!FD_ISSET(this->serialdevice.serialin, &inset))
  {
   TangoSys_MemStream out_stream;
   out_stream << "fd_isset() wrong, who whoke me up?"
              << ends;

	 ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_FD_ISSET",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Are there enough characters in the receiving buffer?
  //
  if (ioctl(this->serialdevice.serialin, FIONREAD, &ncharin) < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "error reading no. of char, errno=" << errno
              << ends;
   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_ioctl",
        out_stream.str(),
        (const char *)tab);
  }

  INFO_STREAM << tab << "ncharin=" << ncharin << endl;

  //
  // Empty the receiving buffer (if not done, the next call to select() will
  // immediately return that this serialline has to be read, therefore
  // the waiting loop would be very system consuming)
  //
  ncharin = read(
	this->serialdevice.serialin,
	this->serialdevice.buffer + this->serialdevice.ncharread,
	(ncharin > (nchar - this->serialdevice.ncharread) ?
            (nchar - this->serialdevice.ncharread) : ncharin));


	if (ncharin < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "error reading from device, errno=" << errno
              << ends;
   ERROR_STREAM << tab;
   ERROR_STREAM << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_read",
        out_stream.str(),
        (const char *)tab);
  }
  this->serialdevice.ncharread += ncharin;
	retrycnt++;
 }
 while(nretry >= retrycnt);

 //
 // At this point the number of characters requested have been read.
 //
 this->serialdevice.buffer[this->serialdevice.ncharread] = 0;

 //
 // Prepare return buffer (can not return "buffer" directly
 // because TANGO desallocate the memory return)
 //
 argout = new char[this->serialdevice.ncharread+1];
 if(argout == 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "unable to allocate memory for the return buffer, need "
             << this->serialdevice.ncharread << " bytes" << ends;
  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_alloc",
        out_stream.str(),
        (const char *)tab);
 }

 // Do not use strncpy() as retry_read_string() is used by xxx_read_char()
 int i;
 for(i=0 ; i<this->serialdevice.ncharread ; i++)
  argout[i] = this->serialdevice.buffer[i];

 // Add string ending char, used only by ser_read_string()
 argout[i]=0;

 return argout;}

//+------------------------------------------------------------------
/**
*      method: Serial::raw_read_string
*
*      description:    Read a string of characters from the serial line if
*      there are no characters to be read return an empty string.
*      The maximum number of characters that can be read is
*      SL_MAXSTRING, if there are more characters in the received
*      buffer, they are let there for the next read.
*
*      @return String read
*
*/
//+------------------------------------------------------------------
char *Serial::raw_read_string(void)
{
 char   *argout;
 int    nchar;
 char   tab[]="Serial::raw_read_string(): ";

 INFO_STREAM << tab << "entering... !" << endl;

 //
 //first "empty" buffer by null terminating it
 //
 this->serialdevice.buffer[0] = 0;

 //
 // Read the current contains of the buffer but no more than we can
 // store in our receiving buffer.
 //
 // As we are in non canonical mode (i.e. raw mode) the read won't be blocking.
 //
 nchar = SL_MAXSTRING;
 nchar = read(this->serialdevice.serialin,this->serialdevice.buffer,nchar);
 if (nchar < 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "error reading from device, errno: " << errno;

  if(errno == ETIMEDOUT)
   out_stream << " (timeout)";

  out_stream << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_read",
        out_stream.str(),
        (const char *)tab);
 }
 //
 // At this point everything is ok
 //
 this->serialdevice.buffer[nchar++] = 0;

 //
 // Prepare return buffer (can not return "buffer" directly
 // because TANGO desallocate the memory return)
 //
 argout = new char[nchar+1];
 if(argout == 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "unable to allocate memory for the return buffer, need "
             << nchar << " bytes" << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_alloc",
        out_stream.str(),
        (const char *)tab);
 }

 // Update global resource
 this->serialdevice.ncharread = nchar;

 // Do not use strncpy() as raw_read_string() is used by xxx_read_char()
 int i;
 for(i=0 ; i<this->serialdevice.ncharread ; i++)
  argout[i] = this->serialdevice.buffer[i];

 // Add string ending char, used only by ser_read_string()
 argout[i]=0;

 return argout;
}
//+------------------------------------------------------------------
/**
*	method:	Serial::nchar_read_string
*
*	description:	Read a string of N characters from the serial line if
*	there are no characters to be read return an empty string.
*
*	@param	argin	number N of characters to read
*	@return	string read
*
*/
//+------------------------------------------------------------------
char *Serial::nchar_read_string(long  nchar)
{
 char 		*argout;
 fd_set		watchset;	// file descriptor set
 fd_set		inset;		// file descriptor set updated by select()
 int		maxfd;		// maximum file descriptor used
 struct timeval	timeend;	// current time + timeout
 struct timeval	timeout;	// timeout value
 float		timeout_s;	// timeout value in seconds
 struct timezone tz;		// not used
 int		readyfd;	// number of file descriptors ready to be read
 int		ncharin; 	// number of characters in the receiving buffer
 char           tab[]="Serial::nchar_read_string(): ";


 INFO_STREAM << tab << "entering... !" << endl;

 //
 //first "empty" buffer by NULL terminating it
 //
 this->serialdevice.buffer[0] = 0;
 this->serialdevice.ncharread = 0;


 //
 // Don't read more than we can store
 //
 if (nchar > SL_MAXSTRING)
 {
  TangoSys_MemStream out_stream;
  out_stream << "number of char requested " << nchar << " > " << SL_MAXSTRING
              << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_nchar",
        out_stream.str(),
        (const char *)tab);
 }

 //
 // Initialize the set, no file descriptors contained
 //
 FD_ZERO(&watchset);

 //
 // Add to the set the file descriptor to watch at
 //
 FD_SET(this->serialdevice.serialin, &watchset);

 //
 // Initialize the timeout (calculate when the timeout should expire)
 //
 timeout_s = ((float)this->serialdevice.timeout) / 1000.0; // seconds

 gettimeofday(&timeend,&tz);
 timeend.tv_usec += (int)((timeout_s - (int)timeout_s) * 1000000.0);
 timeend.tv_sec  += (int)(timeout_s);
 if(timeend.tv_usec > 1000000)
 {
  timeend.tv_usec -= 1000000;
  timeend.tv_sec  += 1;
 }

 //
 // Wait until the receiving buffer contains the requested number of characters
 //
 do
 {
  //
  // Set the maximum file descriptor used, this avoid the system to look
  // through all the file descriptors that a Linux process can have (up to 1024)
  //
  maxfd = this->serialdevice.serialin + 1;

  //
  // As select() will update will update the set, make a copy of it
  //
  inset = watchset;

  //
  // Set the timeout value (take into account time consumed by previous
  // wait periods)
  //
  gettimeofday(&timeout,&tz);
  timeout.tv_usec = timeend.tv_usec - timeout.tv_usec;
  timeout.tv_sec  = timeend.tv_sec  - timeout.tv_sec;
  if(timeout.tv_usec < 0)
  {
   timeout.tv_usec += 1000000;
   timeout.tv_sec  -= 1;
  }

  //
  // Check if the timeout occured
  // With reasonnable values this should not happen, but on overloaded
  // systems with a short timeout value it may happens that the timeout
  // expired at this point before the call to select()
  //
  if(timeout.tv_sec < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read (sched)"
              << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_timeend",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Block until characters become available on the file descriptor
  // listed in the set.
  //
  readyfd = select(maxfd, &inset, NULL, NULL, &timeout);

  //
  // Check if an error occured
  //
  if (readyfd < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read (select() < 0)"
              << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_select",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Check if the timeout occured
  //
  if (readyfd == 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read"
              << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_select",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Check if it's our file descriptor which is ready to be read (Should be, as
  // it was the only watched one out).
  //
  if (!FD_ISSET(this->serialdevice.serialin, &inset))
  {
   TangoSys_MemStream out_stream;
   out_stream << "fd_isset() wrong, who whoke me up?"
              << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_FD_ISSET",
        out_stream.str(),
        (const char *)tab);
  }

  //
  // Are there enough characters in the receiving buffer?
  //
  if (ioctl(this->serialdevice.serialin, FIONREAD, &ncharin) < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "error reading no. of char, errno=" << errno
              << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_ioctl",
        out_stream.str(),
        (const char *)tab);
  }

  INFO_STREAM << tab << "ncharin " << ncharin << " ncharread " << this->serialdevice.ncharread << " nchar " << nchar << endl;

  //
  // Empty the receiving buffer (if not done, the next call to select() will
  // immediately return that this serialline has to be read, therefore
  // the waiting loop would be very system consuming)
  //
  ncharin = read(
	this->serialdevice.serialin,
	this->serialdevice.buffer + this->serialdevice.ncharread,
	(ncharin > (nchar - this->serialdevice.ncharread) ?
            (nchar - this->serialdevice.ncharread) : ncharin));
  if (ncharin < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "error reading from device, errno=" << errno
              << ends;

   ERROR_STREAM << tab;
   ERROR_STREAM << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_read",
        out_stream.str(),
        (const char *)tab);
  }
  this->serialdevice.ncharread += ncharin;

 }
 while(this->serialdevice.ncharread < nchar);

 //
 // At this point the number of characters requested have been read.
 //
 this->serialdevice.buffer[this->serialdevice.ncharread] = 0;

 //
 // Prepare return buffer (can not return "buffer" directly
 // because TANGO desallocate the memory return)
 //
 argout = new char[this->serialdevice.ncharread+1];
 if(argout == 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "unable to allocate memory for the return buffer, need "
             << this->serialdevice.ncharread << " bytes" << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_alloc",
        out_stream.str(),
        (const char *)tab);
 }

 // Do not use strncpy() as raw_read_string() is used by xxx_read_char()
 int i;
 for(i=0 ; i<this->serialdevice.ncharread ; i++)
  argout[i] = this->serialdevice.buffer[i];

 // Add string ending char, used only by ser_read_string()
 argout[i]=0;

 return argout;
}

//+------------------------------------------------------------------
/**
*	method:	Serial::line_read_string
*
*	description:	Read a string of characters from the serial line
*      up and including the next newline.
*
*	@return	string read
*
*/
//+------------------------------------------------------------------
char *Serial::line_read_string(void)
{
 char 		*argout;
 fd_set		watchset;	// file descriptor set
 fd_set		inset;		// file descriptor set updated by select()
 int		maxfd;		// maximum file descriptor used
 struct timeval	timeend;	// current time + timeout
 struct timeval	timeout;	// timeout value
 float		timeout_s;	// timeout value in seconds
 struct timezone tz;		// not used
 int		readyfd;	// number of file descriptors ready to be read
 int		nchar;		// number of characters currently read
 int		ncharin; 	// number of characters in the receiving buffer
 int		eolfound;	// flag set when end of line found
 char           tab[]="Serial::line_read_string(): ";

 INFO_STREAM << tab << "entering... !" << endl;

 //
 //first "empty" buffer by null terminating it
 //
 this->serialdevice.buffer[0] = 0;
 this->serialdevice.ncharread = 0;
 eolfound  = 0;

 // Initialize the set, no file descriptors contained
 FD_ZERO(&watchset);

 // Add to the set the file descriptor to watch at
 FD_SET(this->serialdevice.serialin, &watchset);

 // Initialize the timeout (calculate when the timeout should expire)
 timeout_s = ((float)this->serialdevice.timeout) / 1000.0; // seconds

 gettimeofday(&timeend,&tz);
 timeend.tv_usec += (int)((timeout_s - (int)timeout_s) * 1000000.0);
 timeend.tv_sec  += (int)(timeout_s);
 if(timeend.tv_usec > 1000000)
 {
  timeend.tv_usec -= 1000000;
  timeend.tv_sec  += 1;
 }

 // Wait until the receiving buffer contains the end of line character
 do
 {
  // Set the maximum file descriptor used, this avoid the system to look
  // through all the file descriptors that a Linux process can have (up to 1024)
  maxfd = this->serialdevice.serialin + 1;

  // As select() will update will update the set, make a copy of it
  inset = watchset;

  // Set the timeout value (take into account time consumed by previous
  // wait periods)
  gettimeofday(&timeout,&tz);
  timeout.tv_usec = timeend.tv_usec - timeout.tv_usec;
  timeout.tv_sec  = timeend.tv_sec  - timeout.tv_sec;
  if(timeout.tv_usec < 0)
  {
   timeout.tv_usec += 1000000;
   timeout.tv_sec  -= 1;
  }
	int to = (1000000*timeout.tv_sec + timeout.tv_usec/1000);

  // Block until characters become available on the file descriptor
  // listed in the set.
  readyfd = select(maxfd, &inset, NULL, NULL, &timeout);

  // Check if an error occured
  if (readyfd < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read (select() < 0)"
              << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_select",
        out_stream.str(),
        (const char *)tab);
  }

  // Check if the timeout occured
  if (readyfd == 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "timeout waiting for char to be read (>="
              << to <<" ms)" << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_select",
        out_stream.str(),
        (const char *)tab);
  }

  // Check if it's our file descriptor which is ready to be read (Should be, as
  // it was the only watched one out).
  if (!FD_ISSET(this->serialdevice.serialin, &inset))
  {
   TangoSys_MemStream out_stream;
   out_stream << "fd_isset() wrong, who whoke me up?"
              << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_FD_ISSET",
        out_stream.str(),
        (const char *)tab);
  }


  //
  // Are there enough characters in the receiving buffer?
  //
  if (ioctl(this->serialdevice.serialin, FIONREAD, &ncharin) < 0)
  {
   TangoSys_MemStream out_stream;
   out_stream << "ioctl()error reading no. of char., errno="
              << errno << ends;

   ERROR_STREAM << tab << out_stream.str() << endl;
   Tango::Except::throw_exception(
        (const char *)"Serial::error_ioctl",
        out_stream.str(),
        (const char *)tab);
  }

  INFO_STREAM << tab << "ncharin=" << ncharin << endl;

  // Read the receiving buffer, character per character, looking for
  // the end of line predefined.
  //
  // TODO: implement in more efficient way (using double
  // buffering or fgets() ??) to do less system calls.
  do
  {
   // Empty the receiving buffer (if not done, the next call to select() will
   // immediately return that this serialline has to be read, therefore
   // the waiting loop would be very system consuming)
   if (read(this->serialdevice.serialin,
            this->serialdevice.buffer + this->serialdevice.ncharread, 1) < 0)
   {
    TangoSys_MemStream out_stream;
    out_stream << "error reading from devive, errno="
               << errno << ends;

    ERROR_STREAM << tab << out_stream.str() << endl;
    Tango::Except::throw_exception(
        (const char *)"Serial::error_read",
        out_stream.str(),
        (const char *)tab);
   }
   ncharin--;

   if(this->serialdevice.buffer[this->serialdevice.ncharread]==
     (char)(this->serialdevice.newline))
    eolfound = 1;

   this->serialdevice.ncharread++;

  }
  while((ncharin > 0) && (!eolfound));

 }
 while(!eolfound);

 // At this point the newline has been read.
 this->serialdevice.buffer[this->serialdevice.ncharread] = 0;

 // Prepare return buffer (can not return "buffer" directly
 // because TANGO desallocate the memory return)
 argout = new char[this->serialdevice.ncharread+1];
 if(argout == 0)
 {
  TangoSys_MemStream out_stream;
  out_stream << "unable to allocate memory for the return buffer, need "
             << this->serialdevice.ncharread << " bytes" << ends;

  ERROR_STREAM << tab << out_stream.str() << endl;
  Tango::Except::throw_exception(
        (const char *)"Serial::error_alloc",
        out_stream.str(),
        (const char *)tab);
 }

 // Do not use strncpy() as raw_read_string() is used by xxx_read_char()
 int i;
 for(i=0 ; i<this->serialdevice.ncharread ; i++)
 {
  	argout[i] = this->serialdevice.buffer[i];
  	INFO_STREAM << "[" << i << "] " << this->serialdevice.buffer[i] << " hex : " << hex << (unsigned short)this->serialdevice.buffer[i] << endl;
 }
 // Add string ending char, used only by ser_read_string()
 argout[i]=0;

 return argout;
}


//+------------------------------------------------------------------
/**
*      method: Serial::decode_parameters
*
*      description: Return a string with the serial line configuration
*      parameter decoded from the termio structure passed.
*
*      @param  term structure to decode
*      @param  eol end of line character to use
*      @return String with configuration decoded
*/
//+------------------------------------------------------------------
char *Serial::decode_parameters(struct termios term, char eol )
{
	static char            mess[4096];
	static char            append[128];
	const char			*str;
	//
	mess[0] = 0;
	sprintf(append,"c_cc=0%o%c", term.c_cc,    eol);
	strcat(mess,append);
	sprintf(append,"c_lflag=0%o%c", term.c_lflag, eol);
	strcat(mess,append);
	sprintf(append,"c_cflag=0%o%c", term.c_cflag, eol);
	strcat(mess,append);
	sprintf(append,"c_iflag=0%o%c", term.c_iflag, eol);
	strcat(mess,append);
	sprintf(append,"c_oflag=0%o%c", term.c_oflag, eol);
	strcat(mess,append);

	sprintf(append,"%c", eol);
	strcat(mess,append);

	//
	switch(term.c_cflag & CBAUD)
	{
		case B4000000 : str = "4000000"; break;
		case B3500000 : str = "3500000"; break;
		case B3000000 : str = "3000000"; break;
		case B2500000 : str = "2500000"; break;
		case B2000000 : str = "2000000"; break;
		case B1500000 : str = "1500000"; break;
		case B1152000 : str = "1152000"; break;
		case B1000000 : str = "1000000"; break;
		case B921600  : str =  "921600"; break;
		case B576000  : str =  "576000"; break;
		case B500000  : str =  "500000"; break;
		case B460800  : str =  "460800"; break;
		case B230400  : str =  "230400"; break;
		case B115200  : str =  "115200"; break;
		case B57600   : str =   "57600"; break;
		case B38400   : str =   "38400"; break;
		case B19200   : str =   "19200"; break;
		case B9600    : str =    "9600"; break;
		case B4800    : str =    "4800"; break;
		case B2400    : str =    "2400"; break;
		case B1800    : str =    "1800"; break;
		case B1200    : str =    "1200"; break;
		case B600     : str =     "600"; break;
		case B300     : str =     "300"; break;
		case B150     : str =     "150"; break;
		case B134     : str =     "134"; break;
		case B110     : str =     "110"; break;
		case B75      : str =      "75"; break;
		case B50      : str =      "50"; break;

		default       : str = "???????"; break;
	}
	sprintf(append,"baudrate=%s%c",str, eol);
	strcat(mess,append);

	//
	if(!(term.c_cflag & PARENB))
		str = "none";
	else
		if(!(term.c_cflag & PARODD))
			str = "even";
		else
			str = "odd";

	sprintf(append,"parity=%s%c",str, eol);
	strcat(mess,append);
	//
	switch(term.c_cflag  & CSIZE)
	{
		case CS5      : str = "5"; break;
		case CS6      : str = "6"; break;
		case CS7      : str = "7"; break;
		case CS8      : str = "8"; break;

		default       : str = "?"; break;
	}
	sprintf(append,"charlength=%s%c",str, eol);
	strcat(mess,append);
	//
	if((term.c_cflag & CSTOPB))
		str = "2";
	else
		str = "1";
	sprintf(append,"stopbits=%s%c",str, eol);
	strcat(mess,append);

	return mess;
}



