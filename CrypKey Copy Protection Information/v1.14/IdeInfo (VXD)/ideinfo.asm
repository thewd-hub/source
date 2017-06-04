    TITLE DSIdeInf.asm - VxD retrieves information from IDEs


       ;  created using a modified version of DSIdeInf.asm from
       ;  http://skyscraper.fortunecity.com/virtuosity/452/


       ;  modified by Scott Kell from DSIdeInf.asm in Jan 2001
       ;  bug fixed by Grzegorz Lapinski in May 2002

    
    .386P
    
    .NOLIST
      Include Vmm.inc
      Include VWin32.inc
      Include Shell.inc
    .LIST
    
    ; *****  EQUATES  *****
    
      ; --- Bits ---
      cBit00 EQU 0000000000000001b
      cBit02 EQU 0000000000000100b
      cBit06 EQU 0000000001000000b
      cBit07 EQU 0000000010000000b
    
      ; --- Tests ---
      cERR   EQU cBit00
      cBusy  EQU cBit07
    
      ; --- Area offsets in buffer ---
      cDisk0_Exists = 4
      cDisk0_RawInfo =12
    
      ; --- GDI commands ---
      cATA_GDIcmd    EQU 0ECh    ; GDI command for ATA
      cATAPI_GDIcmd  EQU 0A1h    ; GDI command for ATAPI
    
    
    ; ****  MACROS  *****
    
    ; wPort0 has to be initialized according to IDE
    WaitWhileBusy MACRO
      LOCAL LoopWhileBusy
      LOCAL timeout
      LOCAL TryAgain
      LOCAL DriveReady

        PUSH  EBX
        TryAgain:
          ;  1000000 is too long !
        MOV   EBX, 100000
    
        MOV   DX, [wPort0]
        ADD   DX, 7           ; DX = Port0 +7
    
      LoopWhileBusy:
        DEC   EBX
        CMP   EBX, 0
        JZ  timeout
        IN    AL, DX
        TEST  AL, cBusy
        JNZ LoopWhileBusy

        POP EBX
        JMP DriveReady

      timeout:
        POP EBX
        JMP Next_Mos

        DriveReady:
    
    ENDM  ; (WaitWhileBusy)

    ; wPort0 has to be initialized according to IDE
    ; bDevSelCmd has to be initialized according to MoS
    SelectDevice MACRO
        MOV   DX, [wPort0]
        ADD   DX, 6                 ; DX = Port0 +6
        MOV   AL, [bDevSelCmd]      ; AL = DevSelCmd
    
        OUT   DX, AL

    ENDM  ; (SelectDevice)
    
    ; wPort0 has to be initialized according to IDE
    ; Parameter 'GDIcmd' is GDI command (ATA or ATAPI)
    SendGDIcmd MACRO GDIcmd
        MOV   DX, [wPort0]
        ADD   DX, 7                 ; DX = Port0 +7
        MOV   AL, GDIcmd
    
        OUT   DX, AL
    ENDM  ; (SendGDIcmd)
    
    
    ; -------------------------------
    ; --- Device Descriptor Block ---
    ; -------------------------------
    
    DECLARE_VIRTUAL_DEVICE \
      IDEINFO, 1, 0, IDEINFO_Control,\
      UNDEFINED_DEVICE_ID, UNDEFINED_INIT_ORDER
    
    
    ; ----------------------------------------
    ; --- Prepare Device Control Procedure ---
    ; ----------------------------------------
    
    Begin_control_dispatch IDEINFO
      Control_Dispatch w32_DeviceIoControl, On_DeviceIoControl
    End_control_dispatch IDEINFO
    
    ; ************
    ; **  CODE  **
    ; ************
    
    VxD_LOCKED_CODE_SEG
    
      ; === DEVICE CONTROL PROCEDURE ===
    
      BeginProc On_DeviceIoControl
        ASSUME ESI:PTR DIOCParams
    
        .if [esi].dwIoControlCode==DIOC_Open
    
          xor eax,eax
    
        .elseif [esi].dwIoControlCode==1
    
          CALL IdesInfo
          xor eax, eax
    
        .endif
    
        RET
      EndProc On_DeviceIoControl
    
      ; Meaning of registers:
      ;   ESI: pointer to output buffer
      ;   EDI: pointer to section of 1 disk info, in output buffer
      ;   EBX: IDE (0..3)
      ;   ECX: MasterOrSlave (0..1)
    
      ;  IDE controllers:
      ;   Range: 0..3
      ;   Address of IdeExists(n) byte in output buffer
      ;     = ESI + Ide = ESI + EBX
    
      ; DISCs:
      ;   Range: 0..7
      ;   Disk = Ide*2 + MoS = EBX*2 + ECX
      ;   Address of DiskExists(n) byte in output buffer
      ;     = ESI + cDisk0_Exists + Disk
      ;   Address of Disc raw info first byte in output buffer
      ;     = ESI + cDisk0_RawInfo + Disk*512
    
      BeginProc IdesInfo
        ASSUME ESI:PTR DIOCParams
    
        ; --- Preserve registers and flags ---
    
          PUSHAD
          PUSHFD

          CLI   ;  clear all interrupts

    
        ; --- Initialize ESI as pointer to output buffer ---
    
          MOV   ESI, [ESI.lpvOutBuffer]
    
        ; --------------------------------------
        ; --- Process to get the information ---
        ; --------------------------------------
    
          ; Initialize FOR_Ide counter
                   ;  test all 4 IDE controllers
          MOV   EBX, 3            ; EBX = Ide no 3
                   ;  test the first 2 IDE controllers
;          MOV   EBX, 1            ; EBX = Ide no 3
                   ;  only test the primary IDE controller
;          MOV   EBX, 0            ; EBX = Ide no 3
    
        FOR_Ide:                  ; FOR EBX=3 DOWNTO 0
    
        ; --- Initialize wPort0 for this IDE ---
    
          MOV   DX, [wPorts_table + EBX*2]
          MOV   wPort0, DX
    
        ; --- Check if IDE exists ---
    
          ;MOV    DX, [wPort0]
          ADD   DX, 7                 ; DX = Port0 +7
    
          IN    AL, DX
    
        ;Test0FFh
          CMP    AL, 0FFh
          JNE    Test07Fh
          JMP   NEXT_Ide              ; If AL = 0FFh, IDE not present
                                      ;   -> NEXT_Ide
    
        Test07Fh:
          CMP   AL, 07Fh
          JNE   IdeExists
          JMP   NEXT_Ide              ; If AL = 07Fh, IDE not present
                                      ;   -> NEXT_Ide
    
        IdeExists:
    
        ; --- Set value at the IdeExists area in output buffer ---
    
          MOV   BYTE PTR [ESI + EBX], 1     ; Ide_Exists = True
    
        ; --- Prepare MoS (Master or Slave) loop ---
    
          ; Initialize FOR_MoS counter
          MOV   ECX, 1                ; ECX = Slave
    
        FOR_MoS:                      ; FOR MoS=1 DOWNTO 0
    
        ; --- Get bDevSelCmd for this disc ---
    
          MOV   DL, [bDevSelCmd_table + ECX]      ; DL = DevSelCmd
          MOV   bDevSelCmd, DL
    
        ; --- Check if Disc exists ---
    
          WaitWhileBusy
          SelectDevice
          WaitWhileBusy
    
          TEST  AL, cBit06          ; check the drive ready bit
          JNZ   DiskExists
          JMP   NEXT_MoS            ; if drive ready bit not set then next MoS
    
        DiskExists:
    
        ; Compute Disk (= Ide*2 + MoS = EBX*2 + ECX)
    
          MOV   EAX, EBX        ; EAX = Ide
          SHL   EAX, 1          ; EAX = Ide*2
          ADD   EAX, ECX        ; EAX = Disk
    
        ; --- Prepare EDI as pointer
        ; to the area where info is to be stored --
    
        ; Compute Disk*512 (knowing that 512 = 2 ^9)
    
          XCHG  AH, AL      ; Was: AL = Disk , Now: AH = Disk
                            ;   -> EAX = Disk* 2^8
          SHL   EAX, 1      ; EAX = Disk* 2^9
    
          MOV   EDI, ESI              ; EDI = @Output buffer
          ADD   EDI, cDisk0_RawInfo   ; EDI = @beginning of raw info area in output buffer
          ADD   EDI, EAX              ; EDI = @raw info section for this Disc
    
        ; ----------------- Retrieve ---------
    
          WaitWhileBusy
          SelectDevice
          SendGDIcmd cATA_GDIcmd
          WaitWhileBusy
    
          ; check error status
          MOV   DX, [wPort0]
          ADD   DX, 7                 ; DX = Port0 +7
    
          IN    AL, DX
    
          TEST  AL, cBit00
          JZ    RetrieveInfo          ; if no error then retrieve info
    
          ; ERR=1 -> try ATAPI GDI command
          WaitWhileBusy
          SelectDevice
          SendGDIcmd cATAPI_GDIcmd
          WaitWhileBusy
    
          ; check error status
          MOV   DX, [wPort0]
          ADD   DX, 7                 ; DX = Port0 +7
    
          IN    AL, DX
    
          TEST  AL, cBit00
          JZ    RetrieveInfo          ; if no error then retrieve info
          JMP   NEXT_Mos
    
        RetrieveInfo:
          PUSH  ECX                   ; keep MoS value
    
          MOV   ECX, 256
          MOV   DX, [wPort0]
          CLD
    
          REP   INSW                  ; Retrieve (finally!)
    
          POP ECX                     ; restore MoS value
    
        ; if we've made it this far then we know we have a valid disk
        ; Compute Disk (= Ide*2 + MoS = EBX*2 + ECX)
    
          MOV   EAX, EBX        ; EAX = Ide
          SHL   EAX, 1          ; EAX = Ide*2
          ADD   EAX, ECX        ; EAX = Disk
    
        ; --- Set value at the DiscExists area in output buffer ---
          MOV   BYTE PTR [ESI + cDisk0_Exists + EAX], 1 ; Disk_Exists = True
    
        NEXT_Mos:
    
          ;LOOP FOR_MoS
          CMP   ECX, 0
          JE    NEXT_Ide
          DEC   ECX
          JMP   FOR_MoS
    
        NEXT_Ide:
          ;LOOP FOR_Ide
          CMP   EBX, 0
          JE    Exit_IdeLoop
          DEC   EBX
          JMP   FOR_Ide
    
        Exit_IdeLoop:
    
        ; --- Restore flags and registers ---

          STI   ; restart the interrupts
    
          POPFD
          POPAD
    
          RET
      EndProc IdesInfo
    
    VxD_LOCKED_CODE_ENDS
    
    ; ************
    ; **  DATA  **
    ; ************
    
    VxD_LOCKED_DATA_SEG
    
      ; --- Tables ---
      wPorts_table      WORD  1F0h, 170h, 1E8h, 168h  ; IDE 1 - 4
      bDevSelCmd_table  BYTE  0A0h, 0B0h              ; Master - Slave
    
      ; --- Variables ---
      wPort0            WORD  ?   ; Port 0
      bDevSelCmd        BYTE  ?
    
    VxD_LOCKED_DATA_ENDS
    
    END
