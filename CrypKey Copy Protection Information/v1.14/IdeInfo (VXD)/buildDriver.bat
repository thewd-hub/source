ml /coff /c /Cx /Fl /Sn /DMASM6 /DBLD_COFF /DIS_32 /DWIN40COMPAT /IC:\winddk\inc\win_me ideinfo.asm
link /VXD /DEF:ideinfo.def ideinfo.obj

erase /f ideinfo.exp
erase /f ideinfo.lst
erase /f ideinfo.obj
erase /f ideinfo.lib
rename IDEINFO.VXD ideinfo.vxd