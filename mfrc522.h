#ifndef MFRC522_h
#define MFRC522_h

	#include <inttypes.h>

	typedef uint8_t bool;
	typedef uint8_t byte;
	typedef uint16_t word;

	// MFRC522 registers. Described in chapter 9 of the datasheet.
	// When using SPI all addresses are shifted one bit left in the "SPI address byte" (section 8.1.2.3)
	enum PCD_Register {
		// Page 0: Command and status
		//						  0x00			// reserved for future use
		CommandReg				= 0x01 << 1,	// starts and stops command execution
		ComIEnReg				= 0x02 << 1,	// enable and disable interrupt request control bits
		DivIEnReg				= 0x03 << 1,	// enable and disable interrupt request control bits
		ComIrqReg				= 0x04 << 1,	// interrupt request bits
		DivIrqReg				= 0x05 << 1,	// interrupt request bits
		ErrorReg				= 0x06 << 1,	// error bits showing the error status of the last command executed
		Status1Reg				= 0x07 << 1,	// communication status bits
		Status2Reg				= 0x08 << 1,	// receiver and transmitter status bits
		FIFODataReg				= 0x09 << 1,	// input and output of 64 byte FIFO buffer
		FIFOLevelReg			= 0x0A << 1,	// number of bytes stored in the FIFO buffer
		WaterLevelReg			= 0x0B << 1,	// level for FIFO underflow and overflow warning
		ControlReg				= 0x0C << 1,	// miscellaneous control registers
		BitFramingReg			= 0x0D << 1,	// adjustments for bit-oriented frames
		CollReg					= 0x0E << 1,	// bit position of the first bit-collision detected on the RF interface
		//						  0x0F			// reserved for future use

		// Page 1:Command
		// 						  0x10			// reserved for future use
		ModeReg					= 0x11 << 1,	// defines general modes for transmitting and receiving
		TxModeReg				= 0x12 << 1,	// defines transmission data rate and framing
		RxModeReg				= 0x13 << 1,	// defines reception data rate and framing
		TxControlReg			= 0x14 << 1,	// controls the logical behavior of the antenna driver pins TX1 and TX2
		TxASKReg				= 0x15 << 1,	// controls the setting of the transmission modulation
		TxSelReg				= 0x16 << 1,	// selects the internal sources for the antenna driver
		RxSelReg				= 0x17 << 1,	// selects internal receiver settings
		RxThresholdReg			= 0x18 << 1,	// selects thresholds for the bit decoder
		DemodReg				= 0x19 << 1,	// defines demodulator settings
		// 						  0x1A			// reserved for future use
		// 						  0x1B			// reserved for future use
		MfTxReg					= 0x1C << 1,	// controls some MIFARE communication transmit parameters
		MfRxReg					= 0x1D << 1,	// controls some MIFARE communication receive parameters
		// 						  0x1E			// reserved for future use
		SerialSpeedReg			= 0x1F << 1,	// selects the speed of the serial UART interface

		// Page 2: Configuration
		// 						0x20			// reserved for future use
		CRCResultRegH			= 0x21 << 1,	// shows the MSB and LSB values of the CRC calculation
		CRCResultRegL			= 0x22 << 1,
		// 						  0x23			// reserved for future use
		ModWidthReg				= 0x24 << 1,	// controls the ModWidth setting?
		// 						  0x25			// reserved for future use
		RFCfgReg				= 0x26 << 1,	// configures the receiver gain
		GsNReg					= 0x27 << 1,	// selects the conductance of the antenna driver pins TX1 and TX2 for modulation
		CWGsPReg				= 0x28 << 1,	// defines the conductance of the p-driver output during periods of no modulation
		ModGsPReg				= 0x29 << 1,	// defines the conductance of the p-driver output during periods of modulation
		TModeReg				= 0x2A << 1,	// defines settings for the internal timer
		TPrescalerReg			= 0x2B << 1,	// the lower 8 bits of the TPrescaler value. The 4 high bits are in TModeReg.
		TReloadRegH				= 0x2C << 1,	// defines the 16-bit timer reload value
		TReloadRegL				= 0x2D << 1,
		TCounterValueRegH		= 0x2E << 1,	// shows the 16-bit timer value
		TCounterValueRegL		= 0x2F << 1,

		// Page 3:Test Registers
		// 						  0x30			// reserved for future use
		TestSel1Reg				= 0x31 << 1,	// general test signal configuration
		TestSel2Reg				= 0x32 << 1,	// general test signal configuration
		TestPinEnReg			= 0x33 << 1,	// enables pin output driver on pins D1 to D7
		TestPinValueReg			= 0x34 << 1,	// defines the values for D1 to D7 when it is used as an I/O bus
		TestBusReg				= 0x35 << 1,	// shows the status of the internal test bus
		AutoTestReg				= 0x36 << 1,	// controls the digital self test
		VersionReg				= 0x37 << 1,	// shows the software version
		AnalogTestReg			= 0x38 << 1,	// controls the pins AUX1 and AUX2
		TestDAC1Reg				= 0x39 << 1,	// defines the test value for TestDAC1
		TestDAC2Reg				= 0x3A << 1,	// defines the test value for TestDAC2
		TestADCReg				= 0x3B << 1		// shows the value of ADC I and Q channels
		// 						  0x3C			// reserved for production tests
		// 						  0x3D			// reserved for production tests
		// 						  0x3E			// reserved for production tests
		// 						  0x3F			// reserved for production tests
	};

	// MFRC522 comands. Described in chapter 10 of the datasheet.
	enum PCD_Command {
		PCD_Idle				= 0x00,		// no action, cancels current command execution
		PCD_Mem					= 0x01,		// stores 25 bytes into the internal buffer
		PCD_GenerateRandomID	= 0x02,		// generates a 10-byte random ID number
		PCD_CalcCRC				= 0x03,		// activates the CRC coprocessor or performs a self test
		PCD_Transmit			= 0x04,		// transmits data from the FIFO buffer
		PCD_NoCmdChange			= 0x07,		// no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
		PCD_Receive				= 0x08,		// activates the receiver circuits
		PCD_Transceive 			= 0x0C,		// transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
		PCD_MFAuthent 			= 0x0E,		// performs the MIFARE standard authentication as a reader
		PCD_SoftReset			= 0x0F		// resets the MFRC522
	};

	// Commands sent to the PICC.
	enum PICC_Command {
		// The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
		PICC_CMD_REQA			= 0x26,		// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
		PICC_CMD_WUPA			= 0x52,		// Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
		PICC_CMD_CT				= 0x88,		// Cascade Tag. Not really a command, but used during anti collision.
		PICC_CMD_SEL_CL1		= 0x93,		// Anti collision/Select, Cascade Level 1
		PICC_CMD_SEL_CL2		= 0x95,		// Anti collision/Select, Cascade Level 1
		PICC_CMD_SEL_CL3		= 0x97,		// Anti collision/Select, Cascade Level 1
		PICC_CMD_HLTA			= 0x50,		// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
		// The commands used for MIFARE Classic (from http://www.nxp.com/documents/data_sheet/MF1S503x.pdf, Section 9)
		// Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
		// The read/write commands can also be used for MIFARE Ultralight.
		PICC_CMD_MF_AUTH_KEY_A	= 0x60,		// Perform authentication with Key A
		PICC_CMD_MF_AUTH_KEY_B	= 0x61,		// Perform authentication with Key B
		PICC_CMD_MF_READ		= 0x30,		// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
		PICC_CMD_MF_WRITE		= 0xA0,		// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
		PICC_CMD_MF_DECREMENT	= 0xC0,		// Decrements the contents of a block and stores the result in the internal data register.
		PICC_CMD_MF_INCREMENT	= 0xC1,		// Increments the contents of a block and stores the result in the internal data register.
		PICC_CMD_MF_RESTORE		= 0xC2,		// Reads the contents of a block into the internal data register.
		PICC_CMD_MF_TRANSFER	= 0xB0,		// Writes the contents of the internal data register to a block.
		// The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
		// The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
		PICC_CMD_UL_WRITE		= 0xA2		// Writes one 4 byte page to the PICC.
	};

	// MIFARE constants that does not fit anywhere else
	enum MIFARE_Misc {
		MF_ACK					= 0xA,		// The MIFARE Classic uses a 4 bit ACK/NAK. Any other value than 0xA is NAK.
		MF_KEY_SIZE				= 6			// A Mifare Crypto1 key is 6 bytes.
	};

	// PICC types we can detect. Remember to update PICC_GetTypeName() if you add more.
	enum PICC_Type {
		PICC_TYPE_UNKNOWN		= 0,
		PICC_TYPE_ISO_14443_4	= 1,	// PICC compliant with ISO/IEC 14443-4
		PICC_TYPE_ISO_18092		= 2, 	// PICC compliant with ISO/IEC 18092 (NFC)
		PICC_TYPE_MIFARE_MINI	= 3,	// MIFARE Classic protocol, 320 bytes
		PICC_TYPE_MIFARE_1K		= 4,	// MIFARE Classic protocol, 1KB
		PICC_TYPE_MIFARE_4K		= 5,	// MIFARE Classic protocol, 4KB
		PICC_TYPE_MIFARE_UL		= 6,	// MIFARE Ultralight or Ultralight C
		PICC_TYPE_MIFARE_PLUS	= 7,	// MIFARE Plus
		PICC_TYPE_TNP3XXX		= 8,	// Only mentioned in NXP AN 10833 MIFARE Type Identification Procedure
		PICC_TYPE_NOT_COMPLETE	= 255	// SAK indicates UID is not complete.
	};

	// Return codes from the functions in this class. Remember to update GetStatusCodeName() if you add more.
	enum StatusCode {
		STATUS_OK				= 1,	// Success
		STATUS_ERROR			= 2,	// Error in communication
		STATUS_COLLISION		= 3,	// Collission detected
		STATUS_TIMEOUT			= 4,	// Timeout in communication.
		STATUS_NO_ROOM			= 5,	// A buffer is not big enough.
		STATUS_INTERNAL_ERROR	= 6,	// Internal error in the code. Should not happen ;-)
		STATUS_INVALID			= 7,	// Invalid argument.
		STATUS_CRC_WRONG		= 8,	// The CRC_A does not match
		STATUS_MIFARE_NACK		= 9		// A MIFARE PICC responded with NAK.
	};

	// A struct used for passing the UID of a PICC.
	typedef struct {
		byte		size;			// Number of bytes in the UID. 4, 7 or 10.
		byte		uidByte[10];
		byte		sak;			// The SAK (Select acknowledge) byte returned from the PICC after successful selection.
	} Uid;

	// A struct used for passing a MIFARE Crypto1 key
	typedef struct {
		byte		keyByte[MF_KEY_SIZE];
	} MIFARE_Key;

	// Member variables
	//Uid uid;								// Used by PICC_ReadCardSerial().

	// Size of the MFRC522 FIFO
	//static const byte FIFO_SIZE = 64;		// The FIFO is 64 bytes.

	//-----------------------------------------------------------------------------------
	// Functions for setting up the Arduino
	//-----------------------------------------------------------------------------------
	void MFRC522_init(void);
	void setSPIConfig(void);

	//-----------------------------------------------------------------------------------
	// Basic interface functions for communicating with the MFRC522
	//-----------------------------------------------------------------------------------
	void PCD_WriteRegister(byte reg, byte value);
	void PCD_WriteRegister2(byte reg, byte count, byte *values);
	byte PCD_ReadRegister(byte reg);
	void PCD_ReadRegister2(byte reg, byte count, byte *values, byte rxAlign);
	void setBitMask(unsigned char reg, unsigned char mask);
	void PCD_SetRegisterBitMask(byte reg, byte mask);
	void PCD_ClearRegisterBitMask(byte reg, byte mask);
	byte PCD_CalculateCRC(byte *data, byte length, byte *result);

	//-----------------------------------------------------------------------------------
	// Functions for manipulating the MFRC522
	//-----------------------------------------------------------------------------------
	byte PCD_Init(void);
	byte PCD_Reset(void);
	void PCD_AntennaOn(void);

	//-----------------------------------------------------------------------------------
	// Functions for communicating with PICCs
	//-----------------------------------------------------------------------------------
	byte PCD_TransceiveData(byte *sendData, byte sendLen, byte *backData, byte *backLen, byte *validBits, byte rxAlign, bool checkCRC);
	byte PCD_CommunicateWithPICC(byte command, byte waitIRq, byte *sendData, byte sendLen, byte *backData, byte *backLen, byte *validBits, byte rxAlign, bool checkCRC);

	byte PICC_RequestA(byte *bufferATQA, byte *bufferSize);
	byte PICC_WakeupA(byte *bufferATQA, byte *bufferSize);
	byte PICC_REQA_or_WUPA(	byte command, byte *bufferATQA, byte *bufferSize);
	byte PICC_Select(Uid *uid, byte validBits);
	byte PICC_HaltA(void);

	//-----------------------------------------------------------------------------------
	// Functions for communicating with MIFARE PICCs
	//-----------------------------------------------------------------------------------
	byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
	void PCD_StopCrypto1(void);
	byte MIFARE_Read(byte blockAddr, byte *buffer, byte *bufferSize);
	byte MIFARE_Write(byte blockAddr, byte *buffer, byte bufferSize);
 	byte MIFARE_Decrement(byte blockAddr, long delta);
	byte MIFARE_Increment(byte blockAddr, long delta);
 	byte MIFARE_Restore(byte blockAddr);
 	byte MIFARE_Transfer(byte blockAddr);
	byte MIFARE_Ultralight_Write(byte page, byte *buffer, byte bufferSize);

	//-----------------------------------------------------------------------------------
	// Support functions
	//-----------------------------------------------------------------------------------
	byte PCD_MIFARE_Transceive(	byte *sendData, byte sendLen, bool acceptTimeout);
	const char *GetStatusCodeName(byte code);
	byte PICC_GetType(byte sak);
	const char *PICC_GetTypeName(byte type);
	void PICC_DumpToSerial(Uid *uid);
	void PICC_DumpMifareClassicToSerial(Uid *uid, byte piccType, MIFARE_Key *key);
	void PICC_DumpMifareClassicSectorToSerial(Uid *uid, MIFARE_Key *key, byte sector);
	void PICC_DumpMifareUltralightToSerial(void);
	void MIFARE_SetAccessBits(byte *accessBitBuffer, byte g0, byte g1, byte g2, byte g3);

	//-----------------------------------------------------------------------------------
	// Convenience functions - does not add extra functionality
	//-----------------------------------------------------------------------------------
	bool PICC_IsNewCardPresent(void);
	bool PICC_ReadCardSerial(Uid* uid);

/*
private:
	byte _chipSelectPin;		// Arduino pin connected to MFRC522's SPI slave select input (Pin 24, NSS, active low)
	byte _resetPowerDownPin;	// Arduino pin connected to MFRC522's reset and power down input (Pin 6, NRSTPD, active low)
	byte MIFARE_TwoStepHelper(byte command, byte blockAddr,	long data);
*/

#endif
