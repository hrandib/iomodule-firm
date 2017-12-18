import qbs
import qbs.FileInfo

Project {
	minimumQbsVersion: "1.6.0"
CppApplication
{
  property string ChibiOS: "ChibiOS/"

	type: ["application", "printsize"]
	consoleApplication: true
  cpp.optimization: "small"
	cpp.debugInformation: false
  cpp.enableExceptions: false
  cpp.enableRtti: false
  cpp.positionIndependentCode: false
	cpp.cLanguageVersion: "c11"
  cpp.cxxLanguageVersion: "gnu++17"
	cpp.executableSuffix: ".elf"
  cpp.defines: [
		"_GLIBCXX_USE_C99",
    "_GLIBCXX_HAVE_BROKEN_VSWPRINTF",
    "SHELL_CONFIG_FILE",
    "STM32F100xB"
	]
  cpp.driverFlags: [
		"-mcpu=cortex-m3",
		"-ggdb3",
    "--specs=nano.specs"
		/*"-Wa,--defsym,THUMB2=1"*/
    //"-nostdlib", "-nodefaultlibs"
	]
  cpp.commonCompilerFlags: [
    "-flto=8",
    "-fdata-sections",
    "-ffunction-sections",
    "-Wno-implicit-fallthrough"
//		"-Wno-unused-function",
//		"-Wno-maybe-uninitialized"
	]
  cpp.cxxFlags: [
    "-Wno-register"
  ]

	Group {	name: "Linker files"
    prefix: "ld/"
		fileTags: ["linkerscript"]
    files: ["STM32F100x6.ld"]
	}
  cpp.linkerFlags: [
		"--gc-sections",
    "--Map=c:/Projects/output.map",
    "--defsym=__process_stack_size__=0x100",
    "--defsym=__main_stack_size__=0x100",
  ]
  cpp.includePaths: [
    "config",
    "board",
    "utils",
    "wake",
    "resources",
    "drivers",
    "source",
    //Startup
    ChibiOS + "os/common/startup/ARMCMx/compilers/GCC",
    ChibiOS + "os/common/startup/ARMCMx/devices/STM32F1xx",
    ChibiOS + "os/common/ext/CMSIS/include",
    ChibiOS + "os/common/ext/CMSIS/ST/STM32F1xx",
    //Port
    ChibiOS + "os/common/ports/ARMCMx",
    ChibiOS + "os/common/ports/ARMCMx/compilers/GCC",
    //RT
    ChibiOS + "os/rt/include",
    ChibiOS + "os/common/oslib/include",
    //OSAL
    ChibiOS + "os/hal/osal/rt",
    //HAL
    ChibiOS + "os/hal/include",
    //Platform
    ChibiOS + "os/hal/ports/common/ARMCMx",
    ChibiOS + "os/hal/ports/STM32/STM32F1xx",
    //License
    ChibiOS + "os/license",
    //Drivers
    ChibiOS + "os/hal/ports/STM32/LLD/DACv1",
    ChibiOS + "os/hal/ports/STM32/LLD/DMAv1",
    ChibiOS + "os/hal/ports/STM32/LLD/EXTIv1",
    ChibiOS + "os/hal/ports/STM32/LLD/GPIOv1",
    ChibiOS + "os/hal/ports/STM32/LLD/TIMv1",
    ChibiOS + "os/hal/ports/STM32/LLD/USARTv1",
    //cpp support
    ChibiOS + "os/various/cpp_wrappers",
    //Various
    ChibiOS + "os/various",
    ChibiOS + "os/various/shell",
    ChibiOS + "test/lib",
    ChibiOS + "test/rt/source/test",
    //Streams
    ChibiOS + "os/hal/lib/streams"
	]
  cpp.libraryPaths: [
    ChibiOS + "os/common/startup/ARMCMx/compilers/GCC/ld"
  ]
  Group { name: "Startup"
    prefix: ChibiOS + "os/common/startup/ARMCMx/compilers/GCC/"
    files: [
      "crt0_v7m.S",
      "crt1.c",
      "vectors.c"
    ]
 	}
  Group { name: "cpp"
    prefix: ChibiOS + "os/various/cpp_wrappers/"
    files: [
      "ch.hpp",
      "ch.cpp",
      "syscalls_cpp.cpp",
    ]
  }
  Group { name: "Board"
    prefix: "board/"
    files: [
      "board.h",
      "board.c"
    ]
  }
  Group { name: "Config"
    prefix: "config/"
    files: [
      "chconf.h",
      "halconf.h",
      "mcuconf.h",
      "shellconf.h",
    ]
  }
  Group { name: "Wake"
    prefix: "wake/"
    files: [
      "led_driver.h",
      "led_driver_strip.cpp",
      "wake_base.cpp",
      "wake_base.h",
      "crc8.h",
      "crc8.cpp",
    ]
  }
  Group { name: "Utils"
    prefix: "utils/"
    files: [
      "ch_extended.h",
      "string_utils.h",
      "string_utils.cpp",
      "type_traits_ex.h"
    ]
  }

  Group { name: "Port"
    prefix: ChibiOS + "os/common/ports/ARMCMx/"
    files: [
      "compilers/GCC/chcoreasm_v7m.S",
      "compilers/GCC/chtypes.h",
      "chcore_v7m.c",
      "chcore.c"
    ]
  }
  Group { name: "Platform"
    prefix: ChibiOS + "os/hal/ports/STM32/STM32F1xx/"
    files: [
      "stm32_isr.h",
      "stm32_rcc.h",
      "stm32_registry.h",
      "hal_lld.h",
      "hal_lld.c",
      "hal_lld_f100.h",
      "hal_ext_lld_isr.c",
      "hal_adc_lld.c"
    ]
  }
  Group { name: "Drivers"
    prefix: "drivers/"
    files: [
      "gpio.h",
      "i2c_fallback.h",
      "ssd1306.h"
    ]
  }
  Group { name: "Drivers ChibiOS"
    prefix: ChibiOS + "os/hal/ports/STM32/"
    files: [
      "LLD/DMAv1/stm32_dma.h",
      "LLD/DMAv1/stm32_dma.c",
      "LLD/GPIOv1/hal_pal_lld.h",
      "LLD/GPIOv1/hal_pal_lld.c",
      "LLD/TIMv1/stm32_tim.h",
      "LLD/TIMv1/hal_pwm_lld.h",
      "LLD/TIMv1/hal_pwm_lld.c",
      "LLD/TIMv1/hal_st_lld.h",
      "LLD/TIMv1/hal_st_lld.c",
      "LLD/USARTv1/hal_uart_lld.h",
      "LLD/USARTv1/hal_uart_lld.c",
    ]
  }
  Group { name: "RT"
    prefix: ChibiOS + "os/"
    files: [
      "various/syscalls.c",
      "rt/src/chsys.c",
      "rt/src/chdebug.c",
      "rt/src/chtrace.c",
      "rt/src/chvt.c",
      "rt/src/chschd.c",
      "rt/src/chthreads.c",
      "rt/src/chtm.c",
      "rt/src/chstats.c",
      "rt/src/chregistry.c",
      "rt/src/chsem.c",
      "rt/src/chmtx.c",
      "rt/src/chcond.c",
      "rt/src/chevents.c",
      "rt/src/chmsg.c",
      "rt/src/chdynamic.c",
      "common/oslib/src/chmboxes.c",
      "common/oslib/src/chmemcore.c",
      "common/oslib/src/chheap.c",
      "common/oslib/src/chmempools.c"
    ]
  }
  Group { name: "OSAL"
		files: [
      ChibiOS + "os/hal/osal/rt/osal.h",
      ChibiOS + "os/hal/osal/rt/osal.c"
    ]
	}
  Group { name: "HAL_PORT"
    prefix: ChibiOS + "/os/hal/ports/common/ARMCMx/"
    files: [
      "nvic.h",
      "nvic.c"
    ]
  }
  Group { name: "HAL"
    prefix: ChibiOS + "os/hal/"
    files: [
      "src/hal*.c",
      "include/hal*.h"
    ]
  }
	Group { name: "No LTO"
		cpp.commonCompilerFlags: [
			"-fno-lto"
		]
		files: [
		]
	}
  Group {	name: "Main"
    files: [
      "main.cpp",
      "source/button_control.h",
      "source/display.cpp",
      "source/display.h",
      "source/measure.cpp",
      "source/measure.h",
    ]
    excludeFiles: [
			"**/*_res.c",
			"**/*_conf_template.c",
			"**/*_conf_template.h",
			"**/*.html",
		]
	}
  Group {	name: "Various"
    condition: false
    prefix: ChibiOS + "os/various/"
    files: [
      "shell/shell.h",
      "shell/shell.c",
      "shell/shell_cmd.h",
      "shell/shell_cmd.c",
    ]
  }
  Group {	name: "Lib"
    prefix: ChibiOS + "os/hal/lib/"
    files: [
      "streams/chprintf.h",
      "streams/chprintf.c",
      "streams/memstreams.h",
      "streams/memstreams.c",
      "streams/nullstreams.h",
      "streams/nullstreams.c"
    ]
  }
  Group { name: "Resources"
    prefix: "resources/"
    files: [
      "fonts.h",
      "fonts.cpp",
    ]
  }
  Group { name: "CMSIS"
    prefix: ChibiOS + "os/common/ext/CMSIS/"
    files: [
      "include/core_cm3.h",
      "include/core_cmInstr.h",
      "include/cmsis_gcc.h"
    ]
  }
  Group { name: "Test"
    condition: false
    prefix: ChibiOS + "test/"
    files: [
      "lib/ch_test.h",
      "lib/ch_test.c",
      "rt/source/test/test_root.h",
      "rt/source/test/test_root.c",
      "rt/source/test/test_sequence_001.h",
      "rt/source/test/test_sequence_002.h",
      "rt/source/test/test_sequence_003.h",
      "rt/source/test/test_sequence_004.h",
      "rt/source/test/test_sequence_005.h",
      "rt/source/test/test_sequence_006.h",
      "rt/source/test/test_sequence_007.h",
      "rt/source/test/test_sequence_008.h",
      "rt/source/test/test_sequence_009.h",
      "rt/source/test/test_sequence_010.h",
      "rt/source/test/test_sequence_011.h",
      "rt/source/test/test_sequence_012.h",
      "rt/source/test/test_sequence_001.c",
      "rt/source/test/test_sequence_002.c",
      "rt/source/test/test_sequence_003.c",
      "rt/source/test/test_sequence_004.c",
      "rt/source/test/test_sequence_005.c",
      "rt/source/test/test_sequence_006.c",
      "rt/source/test/test_sequence_007.c",
      "rt/source/test/test_sequence_008.c",
      "rt/source/test/test_sequence_009.c",
      "rt/source/test/test_sequence_010.c",
      "rt/source/test/test_sequence_011.c",
      "rt/source/test/test_sequence_012.c",
      "rt/source/test/test_sequence_013.c",
    ]
  }
  Rule {
		id: size
		inputs: ["application"]
		Artifact {
			fileTags: ["printsize"]
			filePath: "-"
		}
		prepare: {
      var sizePath = "c:/Tools/gccarm_6.3.1/bin/arm-none-eabi-size.exe";
			var args = [input.filePath];
			var cmd = new Command(sizePath, args);
			cmd.description = "File size: " + FileInfo.fileName(input.filePath);
			cmd.highlight = "linker";
			return cmd;
		}
	}
}
}

