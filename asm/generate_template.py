import os,sys

name_list=['test_ABS.S', 
'test_ABS.B.S', 
'test_ABS.H.S', 
'test_ABSDIF.S', 
'test_ABSDIF.B.S', 
'test_ABSDIF.H.S', 
'test_ABSDIFS.S', 
'test_ABSDIFS.H.S', 
'test_ABSS.S', 
'test_ABSS.H.S', 
'test_ADD.S', 
'test_ADD.A.S', 
'test_ADD.B.S', 
'test_ADD.F.S', 
'test_ADD.H.S', 
'test_ADDC.S', 
'test_ADDI.S', 
'test_ADDIH.S', 
'test_ADDIH.A.S', 
'test_ADDS.S', 
'test_ADDS.H.S', 
'test_ADDS.HU.S', 
'test_ADDS.U.S', 
'test_ADDSC.A.S', 
'test_ADDSC.AT.S', 
'test_ADDX.S', 
'test_AND.S', 
'test_AND.AND.T.S', 
'test_AND.ANDN.T.S', 
'test_AND.EQ.S', 
'test_AND.GE.S', 
'test_AND.GE.U.S', 
'test_AND.LT.S', 
'test_AND.LT.U.S', 
'test_AND.NE.S', 
'test_AND.NOR.T.S', 
'test_AND.OR.T.S', 
'test_AND.T.S', 
'test_ANDN.S', 
'test_ANDN.T.S']

TEMP_FILE_NAME = 'Ifx_AbsQ15.S'
fContent = open(TEMP_FILE_NAME, mode='rb')
content = fContent.read()
fContent.close()

for n in name_list:
	f = open(n, mode='wb')
	f.write(content)
	f.close()
	
print("%d files created from template:%s" %(len(name_list), TEMP_FILE_NAME))

