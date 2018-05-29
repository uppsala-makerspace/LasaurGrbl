################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../usblib/usbbuffer.c \
../usblib/usbdesc.c \
../usblib/usbdma.c \
../usblib/usbkeyboardmap.c \
../usblib/usbmode.c \
../usblib/usbringbuf.c \
../usblib/usbtick.c 

OBJS += \
./usblib/usbbuffer.o \
./usblib/usbdesc.o \
./usblib/usbdma.o \
./usblib/usbkeyboardmap.o \
./usblib/usbmode.o \
./usblib/usbringbuf.o \
./usblib/usbtick.o 

C_DEPS += \
./usblib/usbbuffer.d \
./usblib/usbdesc.d \
./usblib/usbdma.d \
./usblib/usbkeyboardmap.d \
./usblib/usbmode.d \
./usblib/usbringbuf.d \
./usblib/usbtick.d 


# Each subdirectory must supply rules for building sources it contributes
usblib/%.o: ../usblib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -Dgcc=1 -DENABLE_LCD -DTARGET_IS_BLIZZARD_RA1 -DARM_MATH_CM4 -DPART_TM4C1233H6PM -I"/home/uppsalamakerspace/laser/LasaurGrbl_UMS/LasaurGrbl" -Os -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -ffunction-sections -fdata-sections -g3 -pedantic -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


