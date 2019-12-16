################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ioLibrary_Driver/Ethernet/W5200/w5200.c 

OBJS += \
./ioLibrary_Driver/Ethernet/W5200/w5200.o 

C_DEPS += \
./ioLibrary_Driver/Ethernet/W5200/w5200.d 


# Each subdirectory must supply rules for building sources it contributes
ioLibrary_Driver/Ethernet/W5200/w5200.o: ../ioLibrary_Driver/Ethernet/W5200/w5200.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-D__HEAP_SIZE=0x1200' '-DMESH_LIB_NATIVE=1' '-D__STACK_SIZE=0x1000' '-DHAL_CONFIG=1' '-DEFR32BG13P632F512GM48=1' -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\protocol\bluetooth\bt_mesh\src" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\emlib\inc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\Device\SiliconLabs\EFR32BG13P\Include" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\hardware\kit\common\drivers" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\CMSIS\Include" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\radio\rail_lib\common" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\protocol\bluetooth\bt_mesh\inc\common" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\emlib\src" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\emdrv\sleep\src" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\protocol\bluetooth\bt_mesh\inc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\protocol\bluetooth\bt_mesh\inc\soc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\hardware\kit\common\halconfig" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\emdrv\common\inc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\hardware\kit\common\bsp" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\hardware\kit\EFR32BG13_BRD4104A\config" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\halconfig\inc\hal-config" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\bootloader\api" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\emdrv\uartdrv\inc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\Device\SiliconLabs\EFR32BG13P\Source" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\Device\SiliconLabs\EFR32BG13P\Source\GCC" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\radio\rail_lib\chip\efr32" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\platform\emdrv\sleep\inc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\inc" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\lcdGraphics" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//platform/middleware/glib" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//platform/middleware/glib/glib" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//platform/middleware/glib/dmd" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//util/silicon_labs/silabs_core/graphics" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//hardware/kit/common/drivers" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\ioLibrary_Driver" -I"C:\Users\jains\SimplicityStudio\v4_workspace\Bhishma_master\ioLibrary_Driver\Ethernet" -Os -fno-builtin -flto -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"ioLibrary_Driver/Ethernet/W5200/w5200.d" -MT"ioLibrary_Driver/Ethernet/W5200/w5200.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


