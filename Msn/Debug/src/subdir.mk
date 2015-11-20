################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/main_msn.cpp \
../src/Matrix.cpp \
../src/MSN.cpp \
../src/MSN_Helper.cpp 

OBJS += \
./src/main_msn.o \
./src/Matrix.o \
./src/MSN.o \
./src/MSN_Helper.o 

CPP_DEPS += \
./src/main_msn.d \
./src/Matrix.d \
./src/MSN.d \
./src/MSN_Helper.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	/gpfs/Compile/impi/4.1.1.036/intel64/bin/mpicxx -DMPICH_IGNORE_CXX_SEEK -DUSE_MPI -I/usr/local/include -I../lis/include -O2 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


