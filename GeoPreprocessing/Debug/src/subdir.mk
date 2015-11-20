################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DataCvt_Base.cpp \
../src/DataCvt_Discrete.cpp \
../src/DataCvt_Log.cpp \
../src/DataCvt_Nml.cpp \
../src/GeoPreprocessingMain.cpp 

OBJS += \
./src/DataCvt_Base.o \
./src/DataCvt_Discrete.o \
./src/DataCvt_Log.o \
./src/DataCvt_Nml.o \
./src/GeoPreprocessingMain.o 

CPP_DEPS += \
./src/DataCvt_Base.d \
./src/DataCvt_Discrete.d \
./src/DataCvt_Log.d \
./src/DataCvt_Nml.d \
./src/GeoPreprocessingMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	/gpfs/Compile/impi/4.1.1.036/intel64/bin/mpicxx -DMPICH_IGNORE_CXX_SEEK -I/usr/local/include -O2 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


