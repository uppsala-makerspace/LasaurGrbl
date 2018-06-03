################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../driverlib/adc.c \
../driverlib/can.c \
../driverlib/comp.c \
../driverlib/cpu.c \
../driverlib/eeprom.c \
../driverlib/flash.c \
../driverlib/fpu.c \
../driverlib/gpio.c \
../driverlib/hibernate.c \
../driverlib/i2c.c \
../driverlib/interrupt.c \
../driverlib/mpu.c \
../driverlib/pwm.c \
../driverlib/qei.c \
../driverlib/ssi.c \
../driverlib/sw_crc.c \
../driverlib/sysctl.c \
../driverlib/sysexc.c \
../driverlib/systick.c \
../driverlib/timer.c \
../driverlib/uart.c \
../driverlib/udma.c \
../driverlib/usb.c \
../driverlib/watchdog.c 

OBJS += \
./driverlib/adc.o \
./driverlib/can.o \
./driverlib/comp.o \
./driverlib/cpu.o \
./driverlib/eeprom.o \
./driverlib/flash.o \
./driverlib/fpu.o \
./driverlib/gpio.o \
./driverlib/hibernate.o \
./driverlib/i2c.o \
./driverlib/interrupt.o \
./driverlib/mpu.o \
./driverlib/pwm.o \
./driverlib/qei.o \
./driverlib/ssi.o \
./driverlib/sw_crc.o \
./driverlib/sysctl.o \
./driverlib/sysexc.o \
./driverlib/systick.o \
./driverlib/timer.o \
./driverlib/uart.o \
./driverlib/udma.o \
./driverlib/usb.o \
./driverlib/watchdog.o 

C_DEPS += \
./driverlib/adc.d \
./driverlib/can.d \
./driverlib/comp.d \
./driverlib/cpu.d \
./driverlib/eeprom.d \
./driverlib/flash.d \
./driverlib/fpu.d \
./driverlib/gpio.d \
./driverlib/hibernate.d \
./driverlib/i2c.d \
./driverlib/interrupt.d \
./driverlib/mpu.d \
./driverlib/pwm.d \
./driverlib/qei.d \
./driverlib/ssi.d \
./driverlib/sw_crc.d \
./driverlib/sysctl.d \
./driverlib/sysexc.d \
./driverlib/systick.d \
./driverlib/timer.d \
./driverlib/uart.d \
./driverlib/udma.d \
./driverlib/usb.d \
./driverlib/watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
driverlib/%.o: ../driverlib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -Dgcc=1 -DENABLE_LCD -DJOY_INVERT_Y -DTARGET_IS_BLIZZARD_RA1 -DARM_MATH_CM4 -DPART_TM4C1233H6PM -I"/home/uppsalamakerspace/laser/LasaurGrbl_UMS/LasaurGrbl" -Os -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -ffunction-sections -fdata-sections -g3 -pedantic -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


