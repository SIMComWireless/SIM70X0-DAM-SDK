#######################################
##$%^&%&^%*****&^*&*&^*&^&*^*&^*&^*&%&^
#EmbeddedAT900 APPlication Project
# brucefan
# 2010-7-13
#Added by brucefan 2010-07-30 merge gcc & rcvt
##$%^&%&^%*****&^*&*&^*&^&*^*&^*&^*&%&^s
#######################################
#############################################################
#Compiler Tools
# You Can Choose :  ARM  or   LLVM
##############################################################
# IF you use RCVT, Please tell me The Compiler ROOT Path
ifeq ($(OS), WINDOWS)
LLVMTOOLCHAIN_PATH:=C:\Users\yanan.sun\Desktop\SIMCom\SIMCom_EAT\SIM7070\Snapdragon-llvm-4.0.11-windows64\bin
LLVMTOOLCHAIN_PATH_STANDARDS:=C:\Users\yanan.sun\Desktop\SIMCom\SIMCom_EAT\SIM7070\Snapdragon-llvm-4.0.11-windows64\armv7m-none-eabi\libc\include
LLVMLIB:=C:\Users\yanan.sun\Desktop\SIMCom\SIMCom_EAT\SIM7070\Snapdragon-llvm-4.0.11-windows64\lib\clang\4.0.11\lib
LLVMLINK_PATH:=C:\Users\yanan.sun\Desktop\SIMCom\SIMCom_EAT\SIM7070\Snapdragon-llvm-4.0.11-windows64\tools\bin
LLVMTOOLCHAIN:=clang.exe
LLVMLINKTOOL:=clang++.exe
PYTHON_PATH := c:/Python27/python.exe
else
LLVMTOOLCHAIN_PATH:=/opt/QualComm/llvm/4.0.11/bin
LLVMTOOLCHAIN_PATH_STANDARDS:=/opt/QualComm/llvm/4.0.11/armv7m-none-eabi/libc/include
LLVMLIB:=/opt/QualComm/llvm/4.0.11/lib/clang/4.0.11/lib
LLVMLINK_PATH:=/opt/QualComm/llvm/4.0.11/tools/bin
LLVMTOOLCHAIN:=clang
LLVMLINKTOOL:=clang++
PYTHON_PATH := python2
endif



##############################################################
# IF you use RCVT, Please tell me The Standard Library Header Files ROOT Path

