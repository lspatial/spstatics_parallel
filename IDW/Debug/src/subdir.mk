################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/IDW.cpp \
../src/IDWMain.cpp 

OBJS += \
./src/IDW.o \
./src/IDWMain.o 

CPP_DEPS += \
./src/IDW.d \
./src/IDWMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	/gpfs/Compile/impi/4.1.1.036/intel64/bin/mpicxx -DMPICH_IGNORE_CXX_SEEK -I/usr/local/include -O2 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


