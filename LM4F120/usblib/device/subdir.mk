################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../usblib/device/usbdaudio.c \
../usblib/device/usbdbulk.c \
../usblib/device/usbdcdc.c \
../usblib/device/usbdcdesc.c \
../usblib/device/usbdcomp.c \
../usblib/device/usbdconfig.c \
../usblib/device/usbddfu-rt.c \
../usblib/device/usbdenum.c \
../usblib/device/usbdhandler.c \
../usblib/device/usbdhid.c \
../usblib/device/usbdhidkeyb.c \
../usblib/device/usbdhidmouse.c \
../usblib/device/usbdmsc.c 

OBJS += \
./usblib/device/usbdaudio.o \
./usblib/device/usbdbulk.o \
./usblib/device/usbdcdc.o \
./usblib/device/usbdcdesc.o \
./usblib/device/usbdcomp.o \
./usblib/device/usbdconfig.o \
./usblib/device/usbddfu-rt.o \
./usblib/device/usbdenum.o \
./usblib/device/usbdhandler.o \
./usblib/device/usbdhid.o \
./usblib/device/usbdhidkeyb.o \
./usblib/device/usbdhidmouse.o \
./usblib/device/usbdmsc.o 

C_DEPS += \
./usblib/device/usbdaudio.d \
./usblib/device/usbdbulk.d \
./usblib/device/usbdcdc.d \
./usblib/device/usbdcdesc.d \
./usblib/device/usbdcomp.d \
./usblib/device/usbdconfig.d \
./usblib/device/usbddfu-rt.d \
./usblib/device/usbdenum.d \
./usblib/device/usbdhandler.d \
./usblib/device/usbdhid.d \
./usblib/device/usbdhidkeyb.d \
./usblib/device/usbdhidmouse.d \
./usblib/device/usbdmsc.d 


# Each subdirectory must supply rules for building sources it contributes
usblib/device/%.o: ../usblib/device/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -Dgcc=1 -DENABLE_LCD -DJOY_INVERT_Y -DTARGET_IS_BLIZZARD_RA1 -DARM_MATH_CM4 -DPART_TM4C1233H6PM -I"/home/uppsalamakerspace/laser/LasaurGrbl_UMS/LasaurGrbl" -Os -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -ffunction-sections -fdata-sections -g3 -pedantic -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


