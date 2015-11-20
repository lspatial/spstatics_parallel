################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cxalloc.cpp \
../src/cxarithm.cpp \
../src/cxarray.cpp \
../src/cxcmp.cpp \
../src/cxconvert.cpp \
../src/cxcopy.cpp \
../src/cxdatastructs.cpp \
../src/cxdrawing.cpp \
../src/cxdxt.cpp \
../src/cxerror.cpp \
../src/cxjacobieigens.cpp \
../src/cxlogic.cpp \
../src/cxmathfuncs.cpp \
../src/cxmatmul.cpp \
../src/cxmatrix.cpp \
../src/cxmean.cpp \
../src/cxmeansdv.cpp \
../src/cxminmaxloc.cpp \
../src/cxnorm.cpp \
../src/cxouttext.cpp \
../src/cxpersistence.cpp \
../src/cxprecomp.cpp \
../src/cxrand.cpp \
../src/cxsumpixels.cpp \
../src/cxsvd.cpp \
../src/cxswitcher.cpp \
../src/cxtables.cpp \
../src/cxutils.cpp 

OBJS += \
./src/cxalloc.o \
./src/cxarithm.o \
./src/cxarray.o \
./src/cxcmp.o \
./src/cxconvert.o \
./src/cxcopy.o \
./src/cxdatastructs.o \
./src/cxdrawing.o \
./src/cxdxt.o \
./src/cxerror.o \
./src/cxjacobieigens.o \
./src/cxlogic.o \
./src/cxmathfuncs.o \
./src/cxmatmul.o \
./src/cxmatrix.o \
./src/cxmean.o \
./src/cxmeansdv.o \
./src/cxminmaxloc.o \
./src/cxnorm.o \
./src/cxouttext.o \
./src/cxpersistence.o \
./src/cxprecomp.o \
./src/cxrand.o \
./src/cxsumpixels.o \
./src/cxsvd.o \
./src/cxswitcher.o \
./src/cxtables.o \
./src/cxutils.o 

CPP_DEPS += \
./src/cxalloc.d \
./src/cxarithm.d \
./src/cxarray.d \
./src/cxcmp.d \
./src/cxconvert.d \
./src/cxcopy.d \
./src/cxdatastructs.d \
./src/cxdrawing.d \
./src/cxdxt.d \
./src/cxerror.d \
./src/cxjacobieigens.d \
./src/cxlogic.d \
./src/cxmathfuncs.d \
./src/cxmatmul.d \
./src/cxmatrix.d \
./src/cxmean.d \
./src/cxmeansdv.d \
./src/cxminmaxloc.d \
./src/cxnorm.d \
./src/cxouttext.d \
./src/cxpersistence.d \
./src/cxprecomp.d \
./src/cxrand.d \
./src/cxsumpixels.d \
./src/cxsvd.d \
./src/cxswitcher.d \
./src/cxtables.d \
./src/cxutils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../include -I../src -O0 -g3 -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


