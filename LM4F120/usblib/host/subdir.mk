################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../usblib/host/usbhaudio.c \
../usblib/host/usbhhid.c \
../usblib/host/usbhhidkeyboard.c \
../usblib/host/usbhhidmouse.c \
../usblib/host/usbhhub.c \
../usblib/host/usbhmsc.c \
../usblib/host/usbhostenum.c \
../usblib/host/usbhscsi.c 

OBJS += \
./usblib/host/usbhaudio.o \
./usblib/host/usbhhid.o \
./usblib/host/usbhhidkeyboard.o \
./usblib/host/usbhhidmouse.o \
./usblib/host/usbhhub.o \
./usblib/host/usbhmsc.o \
./usblib/host/usbhostenum.o \
./usblib/host/usbhscsi.o 

C_DEPS += \
./usblib/host/usbhaudio.d \
./usblib/host/usbhhid.d \
./usblib/host/usbhhidkeyboard.d \
./usblib/host/usbhhidmouse.d \
./usblib/host/usbhhub.d \
./usblib/host/usbhmsc.d \
./usblib/host/usbhostenum.d \
./usblib/host/usbhscsi.d 


# Each subdirectory must supply rules for building sources it contributes
usblib/host/%.o: ../usblib/host/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -Dgcc=1 -DENABLE_LCD -DJOY_INVERT_Y -DTARGET_IS_BLIZZARD_RA1 -DARM_MATH_CM4 -DPART_TM4C1233H6PM -I"/home/uppsalamakerspace/laser/LasaurGrbl_UMS/LasaurGrbl" -Os -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -ffunction-sections -fdata-sections -g3 -pedantic -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


