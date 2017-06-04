#define DISKTYPE_UNKNOWN				0
#define DISKTYPE_FIXED					1
#define DISKTYPE_REMOVABLE				2

#define CONTROLLERTYPE_UNKNOWN			0
#define CONTROLLERTYPE_PRIMARY			1
#define CONTROLLERTYPE_SECONDARY		2
#define CONTROLLERTYPE_TERTIARY			3
#define CONTROLLERTYPE_QUATERNARY		4

#define DRIVETYPE_UNKNOWN				0
#define DRIVETYPE_MASTER				1
#define DRIVETYPE_SLAVE					2

#define FILE_DEVICE_CONTROLLER			0x00000004
#define FILE_DEVICE_SCSI				0x0000001B
#define IOCTL_SCSI_BASE					FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_MINIPORT_IDENTIFY	((FILE_DEVICE_SCSI << 16) + 0x0501)
#define IOCTL_SCSI_MINIPORT				CTL_CODE(IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IDENTIFY_BUFFER_SIZE			512
#define IDE_ATA_IDENTIFY				0xEC
#define IDEINFO_IDENTIFY				0x01
#define MAX_IDE_DRIVES					4
#define MAX_SCSI_DRIVES					2

#pragma pack(1)
typedef struct _IDENTIFY_DATA
{
	USHORT GeneralConfiguration;
	USHORT NumberOfCylinders;
	USHORT Reserved1;
	USHORT NumberOfHeads;
	USHORT UnformattedBytesPerTrack;
	USHORT UnformattedBytesPerSector;
	USHORT SectorsPerTrack;
	USHORT VendorUnique1[3];
	USHORT SerialNumber[10];
	USHORT BufferType;
	USHORT BufferSectorSize;
	USHORT NumberOfEccBytes;
	USHORT FirmwareRevision[4];
	USHORT ModelNumber[20];
	UCHAR MaximumBlockTransfer;
	UCHAR VendorUnique2;
	USHORT DoubleWordIo;
	USHORT Capabilities;
	USHORT Reserved2;
	UCHAR VendorUnique3;
	UCHAR PioCycleTimingMode;
	UCHAR VendorUnique4;
	UCHAR DmaCycleTimingMode;
	USHORT TranslationFieldsValid : 1;
	USHORT Reserved3 : 15;
	USHORT NumberOfCurrentCylinders;
	USHORT NumberOfCurrentHeads;
	USHORT CurrentSectorsPerTrack;
	ULONG CurrentSectorCapacity;
	USHORT CurrentMultiSectorSetting;
	ULONG UserAddressableSectors;
	USHORT SingleWordDMASupport : 8;
	USHORT SingleWordDMAActive : 8;
	USHORT MultiWordDMASupport : 8;
	USHORT MultiWordDMAActive : 8;
	USHORT AdvancedPIOModes : 8;
	USHORT Reserved4 : 8;
	USHORT MinimumMWXferCycleTime;
	USHORT RecommendedMWXferCycleTime;
	USHORT MinimumPIOCycleTime;
	USHORT MinimumPIOCycleTimeIORDY;
	USHORT Reserved5[2];
	USHORT ReleaseTimeOverlapped;
	USHORT ReleaseTimeServiceCommand;
	USHORT MajorRevision;
	USHORT MinorRevision;
	USHORT Reserved6[50];
	USHORT SpecialFunctionsEnabled;
	USHORT Reserved7[128];
} IDENTIFY_DATA, *PIDENTIFY_DATA;

typedef struct _SRB_IO_CONTROL
{
	ULONG HeaderLength;
	UCHAR Signature[8];
	ULONG Timeout;
	ULONG ControlCode;
	ULONG ReturnCode;
	ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

#ifndef _WINIOCTL_
typedef struct _DRIVERSTATUS
{
	UCHAR bDriverError;
	UCHAR bIDEError;
	UCHAR bReserved[2];
	ULONG dwReserved[2];
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;
#endif

#ifndef _WINIOCTL_
typedef struct _IDEREGS
{
	UCHAR bFeaturesReg;
	UCHAR bSectorCountReg;
	UCHAR bSectorNumberReg;
	UCHAR bCylLowReg;
	UCHAR bCylHighReg;
	UCHAR bDriveHeadReg;
	UCHAR bCommandReg;
	UCHAR bReserved;
} IDEREGS, *PIDEREGS, *LPIDEREGS;
#endif

#ifndef _WINIOCTL_
typedef struct _SENDCMDINPARAMS
{
	ULONG cBufferSize;
	IDEREGS irDriveRegs;
	UCHAR bDriveNumber;
	UCHAR bReserved[3];
	ULONG dwReserved[4];
	UCHAR bBuffer[1];
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;
#endif

#ifndef _WINIOCTL_
typedef struct _SENDCMDOUTPARAMS
{
	ULONG cBufferSize;
	DRIVERSTATUS DriverStatus;
	UCHAR bBuffer[1];
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;
#endif

typedef struct _IDEINFO
{
	BYTE IDEExists[4];
	BYTE DiskExists[8];
	BYTE DisksRawInfo[8 * IDENTIFY_BUFFER_SIZE];
} IDEINFO, *PIDEINFO;
#pragma pack()

void showHDSNInformation(void);