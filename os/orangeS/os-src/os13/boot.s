
  org 07c00h

  jmp short LABEL_START   ; start to boot
  nop 

  ; header of FAT12
  BS_OEMName      db  'ForrestY'    ; must be 8 bytes
  BPB_BytsPerSec  dw  512           ; bytes of per sector
  BPB_SecPerClus  db  1             ; sectors of per clustor
  BPB_RsvdSecCnt  dw  1             ; sectors of boot record
  BPB_NumFATs     db  2             ; FAT tables 
  BPB_RootEntCnt  dw  224           ; max file numbers of root directory
  BPB_TotSec16    dw  2880          ; total logic sectors 
  BPB_Media       db  0xf0          ; media descriptor
  BPB_FATSz16     dw  9             ; sectors of per FAT 
  BPB_SecPerTrk   dw  18            ; sectors of per track
  BPB_NumHeads    dw  2             ; number of disk header 
  BPB_HddSec      dd  0             ; hidded sectors 
  BPB_TotSec32    dd  0             ; sector number 
  BS_DrvNum       db  0             ; driver number of interrupt 13
  BS_Reserved1    db  0             ; unused 
  DS_BootSig      db  29h           ; extend boot mark(29h)
  DS_VolID        dd  0             ; volume id 
  DS_VolLab       db  'OrangeS0.02' ; must 11 bytes 
  BS_FileSysType  db  'FAT12'       ; type of file system, must 8 bytes 


LABEL_START:
  mov   ax, cs 
  mov   ds, ax 
  mov   es, ax 
  call  DisplayStr
  jmp   $
DisplayStr:
  mov   ax, BootMsg
  mov   bp, ax      ; ed:bp = address of string 
  mov   cx, 16 
  mov   ax, 01301h
  mov   bx, 000ch
  mov   dl, 0
  int   10h 
  ret 

BootMsg:                db  "Hello, OS world!"
times   510 - ($ - $$)  db  0
                        dw  0xaa55    ; end mark
