/******************************************************************************
 * Copyright (C) 2019 Synopsys, Inc.
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 ******************************************************************************/

#include "modeldb.h"

#include <unicorn/arm.h>
#include <unicorn/arm64.h>

#include <set>
#include <cassert>

#include "common.h"

namespace unicorn {

    static const reg g_regdb[] = {
        /* aarch64 core registers */
        { UC_ARM64_REG_X0,  0, 64, "X0"  },
        { UC_ARM64_REG_X1,  0, 64, "X1"  },
        { UC_ARM64_REG_X2,  0, 64, "X2"  },
        { UC_ARM64_REG_X3,  0, 64, "X3"  },
        { UC_ARM64_REG_X4,  0, 64, "X4"  },
        { UC_ARM64_REG_X5,  0, 64, "X5"  },
        { UC_ARM64_REG_X6,  0, 64, "X6"  },
        { UC_ARM64_REG_X7,  0, 64, "X7"  },
        { UC_ARM64_REG_X8,  0, 64, "X8"  },
        { UC_ARM64_REG_X9,  0, 64, "X9"  },
        { UC_ARM64_REG_X10, 0, 64, "X10" },
        { UC_ARM64_REG_X11, 0, 64, "X11" },
        { UC_ARM64_REG_X12, 0, 64, "X12" },
        { UC_ARM64_REG_X13, 0, 64, "X13" },
        { UC_ARM64_REG_X14, 0, 64, "X14" },
        { UC_ARM64_REG_X15, 0, 64, "X15" },
        { UC_ARM64_REG_X16, 0, 64, "X16" },
        { UC_ARM64_REG_X17, 0, 64, "X17" },
        { UC_ARM64_REG_X18, 0, 64, "X18" },
        { UC_ARM64_REG_X19, 0, 64, "X19" },
        { UC_ARM64_REG_X20, 0, 64, "X20" },
        { UC_ARM64_REG_X21, 0, 64, "X21" },
        { UC_ARM64_REG_X22, 0, 64, "X22" },
        { UC_ARM64_REG_X23, 0, 64, "X23" },
        { UC_ARM64_REG_X24, 0, 64, "X24" },
        { UC_ARM64_REG_X25, 0, 64, "X25" },
        { UC_ARM64_REG_X26, 0, 64, "X26" },
        { UC_ARM64_REG_X27, 0, 64, "X27" },
        { UC_ARM64_REG_X28, 0, 64, "X28" },
        { UC_ARM64_REG_X29, 0, 64, "X29" },
        { UC_ARM64_REG_X30, 0, 64, "X30" },
        { UC_ARM64_REG_SP,  0, 64, "SP"  },
        { UC_ARM64_REG_PC,  0, 64, "PC"  },

        /* aarch64 status register and bitfields */
        { UC_ARM64_REG_PSTATE,  0, 32, "CPSR64",    },
        { UC_ARM64_REG_PSTATE,  0,  1, "CPSR64.SP"  },
        { UC_ARM64_REG_PSTATE,  2,  2, "CPSR64.EL"  },
        { UC_ARM64_REG_PSTATE,  4,  1, "CPSR64.nRW" },
        { UC_ARM64_REG_PSTATE,  6,  1, "CPSR64.F"   },
        { UC_ARM64_REG_PSTATE,  7,  1, "CPSR64.I"   },
        { UC_ARM64_REG_PSTATE,  8,  1, "CPSR64.A"   },
        { UC_ARM64_REG_PSTATE,  9,  1, "CPSR64.D"   },
        { UC_ARM64_REG_PSTATE, 20,  1, "CPSR64.IL"  },
        { UC_ARM64_REG_PSTATE, 21,  1, "CPSR64.SS"  },
        { UC_ARM64_REG_PSTATE, 28,  1, "CPSR64.V"   },
        { UC_ARM64_REG_PSTATE, 29,  1, "CPSR64.C"   },
        { UC_ARM64_REG_PSTATE, 30,  1, "CPSR64.Z"   },
        { UC_ARM64_REG_PSTATE, 31,  1, "CPSR64.N"   },

        /* aarch64 banked status register and bitfields */
        { UC_ARM64_REG_SPSR_EL1,  0, 32, "SPSR_EL1",    },
        { UC_ARM64_REG_SPSR_EL1,  0,  1, "SPSR_EL1.SP"  },
        { UC_ARM64_REG_SPSR_EL1,  2,  2, "SPSR_EL1.EL"  },
        { UC_ARM64_REG_SPSR_EL1,  4,  1, "SPSR_EL1.nRW" },
        { UC_ARM64_REG_SPSR_EL1,  6,  1, "SPSR_EL1.F"   },
        { UC_ARM64_REG_SPSR_EL1,  7,  1, "SPSR_EL1.I"   },
        { UC_ARM64_REG_SPSR_EL1,  8,  1, "SPSR_EL1.A"   },
        { UC_ARM64_REG_SPSR_EL1,  9,  1, "SPSR_EL1.D"   },
        { UC_ARM64_REG_SPSR_EL1, 20,  1, "SPSR_EL1.IL"  },
        { UC_ARM64_REG_SPSR_EL1, 21,  1, "SPSR_EL1.SS"  },
        { UC_ARM64_REG_SPSR_EL1, 28,  1, "SPSR_EL1.V"   },
        { UC_ARM64_REG_SPSR_EL1, 29,  1, "SPSR_EL1.C"   },
        { UC_ARM64_REG_SPSR_EL1, 30,  1, "SPSR_EL1.Z"   },
        { UC_ARM64_REG_SPSR_EL1, 31,  1, "SPSR_EL1.N"   },

        { UC_ARM64_REG_SPSR_EL2,  0, 32, "SPSR_EL2",    },
        { UC_ARM64_REG_SPSR_EL2,  0,  1, "SPSR_EL2.SP"  },
        { UC_ARM64_REG_SPSR_EL2,  2,  2, "SPSR_EL2.EL"  },
        { UC_ARM64_REG_SPSR_EL2,  4,  1, "SPSR_EL2.nRW" },
        { UC_ARM64_REG_SPSR_EL2,  6,  1, "SPSR_EL2.F"   },
        { UC_ARM64_REG_SPSR_EL2,  7,  1, "SPSR_EL2.I"   },
        { UC_ARM64_REG_SPSR_EL2,  8,  1, "SPSR_EL2.A"   },
        { UC_ARM64_REG_SPSR_EL2,  9,  1, "SPSR_EL2.D"   },
        { UC_ARM64_REG_SPSR_EL2, 20,  1, "SPSR_EL2.IL"  },
        { UC_ARM64_REG_SPSR_EL2, 21,  1, "SPSR_EL2.SS"  },
        { UC_ARM64_REG_SPSR_EL2, 28,  1, "SPSR_EL2.V"   },
        { UC_ARM64_REG_SPSR_EL2, 29,  1, "SPSR_EL2.C"   },
        { UC_ARM64_REG_SPSR_EL2, 30,  1, "SPSR_EL2.Z"   },
        { UC_ARM64_REG_SPSR_EL2, 31,  1, "SPSR_EL2.N"   },

        { UC_ARM64_REG_SPSR_EL3,  0, 32, "SPSR_EL3",    },
        { UC_ARM64_REG_SPSR_EL3,  0,  1, "SPSR_EL3.SP"  },
        { UC_ARM64_REG_SPSR_EL3,  2,  2, "SPSR_EL3.EL"  },
        { UC_ARM64_REG_SPSR_EL3,  4,  1, "SPSR_EL3.nRW" },
        { UC_ARM64_REG_SPSR_EL3,  6,  1, "SPSR_EL3.F"   },
        { UC_ARM64_REG_SPSR_EL3,  7,  1, "SPSR_EL3.I"   },
        { UC_ARM64_REG_SPSR_EL3,  8,  1, "SPSR_EL3.A"   },
        { UC_ARM64_REG_SPSR_EL3,  9,  1, "SPSR_EL3.D"   },
        { UC_ARM64_REG_SPSR_EL3, 20,  1, "SPSR_EL3.IL"  },
        { UC_ARM64_REG_SPSR_EL3, 21,  1, "SPSR_EL3.SS"  },
        { UC_ARM64_REG_SPSR_EL3, 28,  1, "SPSR_EL3.V"   },
        { UC_ARM64_REG_SPSR_EL3, 29,  1, "SPSR_EL3.C"   },
        { UC_ARM64_REG_SPSR_EL3, 30,  1, "SPSR_EL3.Z"   },
        { UC_ARM64_REG_SPSR_EL3, 31,  1, "SPSR_EL3.N"   },

        /* aarch64 banked registers */
        { UC_ARM64_REG_SP_EL0,    0, 64, "SP_EL0"    },
        { UC_ARM64_REG_SP_EL1,    0, 64, "SP_EL1"    },
        { UC_ARM64_REG_SP_EL2,    0, 64, "SP_EL2"    },
        { UC_ARM64_REG_SP_EL3,    0, 64, "SP_EL3"    },
        { UC_ARM64_REG_ELR_EL0,   0, 64, "ELR_EL0"   },
        { UC_ARM64_REG_ELR_EL1,   0, 64, "ELR_EL1"   },
        { UC_ARM64_REG_ELR_EL2,   0, 64, "ELR_EL2"   },
        { UC_ARM64_REG_ELR_EL3,   0, 64, "ELR_EL3"   },
        { UC_ARM64_REG_SCTLR_EL1, 0, 64, "SCTLR_EL1" },
        { UC_ARM64_REG_SCTLR_EL2, 0, 64, "SCTLR_EL2" },
        { UC_ARM64_REG_SCTLR_EL3, 0, 64, "SCTLR_EL3" },
        { UC_ARM64_REG_VBAR_EL1,  0, 64, "VBAR_EL1"  },
        { UC_ARM64_REG_VBAR_EL2,  0, 64, "VBAR_EL2"  },
        { UC_ARM64_REG_VBAR_EL3,  0, 64, "VBAR_EL3"  },

        /* aarch64 system registers */
        { UC_ARM64_REG_DACR_S,  0, 64, "DACR_S"     },
        { UC_ARM64_REG_DACR_NS, 0, 64, "DACR_NS"    },
        { UC_ARM64_REG_DACR32,  0, 64, "DACR"       },
        { UC_ARM64_REG_HCR_EL2, 0, 64, "HCR_EL2"    },
        { UC_ARM64_REG_SCR_EL3, 0, 64, "SCR_EL3"    },
        { UC_ARM64_REG_MIDR,    0, 64, "MIDR_EL1"   },
        { UC_ARM64_REG_MPIDR,   0, 64, "MPIDR_EL1"  },
        { UC_ARM64_REG_VPIDR,   0, 64, "VPIDR_EL2"  },
        { UC_ARM64_REG_VMPIDR,  0, 64, "VMPIDR_EL1" },

        /* aarch64 floating point registers */
        { UC_ARM64_REG_V0,   0, 64, "V0"  },
        { UC_ARM64_REG_V1,   0, 64, "V1"  },
        { UC_ARM64_REG_V2,   0, 64, "V2"  },
        { UC_ARM64_REG_V3,   0, 64, "V3"  },
        { UC_ARM64_REG_V4,   0, 64, "V4"  },
        { UC_ARM64_REG_V5,   0, 64, "V5"  },
        { UC_ARM64_REG_V6,   0, 64, "V6"  },
        { UC_ARM64_REG_V7,   0, 64, "V7"  },
        { UC_ARM64_REG_V8,   0, 64, "V8"  },
        { UC_ARM64_REG_V9,   0, 64, "V9"  },
        { UC_ARM64_REG_V10,  0, 64, "V10" },
        { UC_ARM64_REG_V11,  0, 64, "V11" },
        { UC_ARM64_REG_V12,  0, 64, "V12" },
        { UC_ARM64_REG_V13,  0, 64, "V13" },
        { UC_ARM64_REG_V14,  0, 64, "V14" },
        { UC_ARM64_REG_V15,  0, 64, "V15" },
        { UC_ARM64_REG_V16,  0, 64, "V16" },
        { UC_ARM64_REG_V17,  0, 64, "V17" },
        { UC_ARM64_REG_V18,  0, 64, "V18" },
        { UC_ARM64_REG_V19,  0, 64, "V19" },
        { UC_ARM64_REG_V20,  0, 64, "V20" },
        { UC_ARM64_REG_V21,  0, 64, "V21" },
        { UC_ARM64_REG_V22,  0, 64, "V22" },
        { UC_ARM64_REG_V23,  0, 64, "V23" },
        { UC_ARM64_REG_V24,  0, 64, "V24" },
        { UC_ARM64_REG_V25,  0, 64, "V25" },
        { UC_ARM64_REG_V26,  0, 64, "V26" },
        { UC_ARM64_REG_V27,  0, 64, "V27" },
        { UC_ARM64_REG_V28,  0, 64, "V28" },
        { UC_ARM64_REG_V29,  0, 64, "V29" },
        { UC_ARM64_REG_V30,  0, 64, "V30" },
        { UC_ARM64_REG_V31,  0, 64, "V31" },

        /* aarch64 floating point status registers */
        { UC_ARM64_REG_FPSR, 0, 32, "FPSR" },
        { UC_ARM64_REG_FPCR, 0, 32, "FPCR" },

        /* aarch32 core registers */
        { UC_ARM_REG_R0,  0, 32, "R0"  },
        { UC_ARM_REG_R1,  0, 32, "R1"  },
        { UC_ARM_REG_R2,  0, 32, "R2"  },
        { UC_ARM_REG_R3,  0, 32, "R3"  },
        { UC_ARM_REG_R4,  0, 32, "R4"  },
        { UC_ARM_REG_R5,  0, 32, "R5"  },
        { UC_ARM_REG_R6,  0, 32, "R6"  },
        { UC_ARM_REG_R7,  0, 32, "R7"  },
        { UC_ARM_REG_R8,  0, 32, "R8"  },
        { UC_ARM_REG_R9,  0, 32, "R9"  },
        { UC_ARM_REG_R10, 0, 32, "R10" },
        { UC_ARM_REG_R11, 0, 32, "R11" },
        { UC_ARM_REG_R12, 0, 32, "R12" },
        { UC_ARM_REG_SP,  0, 32, "R13" },
        { UC_ARM_REG_LR,  0, 32, "R14" },
        { UC_ARM_REG_PC,  0, 32, "R15" },

        /* aarch32 banked core registers */
        { UC_ARM_REG_R8_USR,   0, 32, "R8_USR"  },
        { UC_ARM_REG_R8_FIQ,   0, 32, "R8_FIQ"  },
        { UC_ARM_REG_R9_USR,   0, 32, "R9_USR"  },
        { UC_ARM_REG_R9_FIQ,   0, 32, "R9_FIQ"  },
        { UC_ARM_REG_R10_USR,  0, 32, "R10_USR" },
        { UC_ARM_REG_R10_FIQ,  0, 32, "R10_FIQ" },
        { UC_ARM_REG_R11_USR,  0, 32, "R11_USR" },
        { UC_ARM_REG_R11_FIQ,  0, 32, "R11_FIQ" },
        { UC_ARM_REG_R12_USR,  0, 32, "R12_USR" },
        { UC_ARM_REG_R12_FIQ,  0, 32, "R12_FIQ" },
        { UC_ARM_REG_R13_USR,  0, 32, "R13_USR" },
        { UC_ARM_REG_R13_SVC,  0, 32, "R13_SVC" },
        { UC_ARM_REG_R13_ABT,  0, 32, "R13_ABT" },
        { UC_ARM_REG_R13_UND,  0, 32, "R13_UND" },
        { UC_ARM_REG_R13_IRQ,  0, 32, "R13_IRQ" },
        { UC_ARM_REG_R13_FIQ,  0, 32, "R13_FIQ" },
        { UC_ARM_REG_R13_HYP,  0, 32, "R13_HYP" },
        { UC_ARM_REG_R13_MON,  0, 32, "R13_MON" },
        { UC_ARM_REG_R14_USR,  0, 32, "R14_USR" },
        { UC_ARM_REG_R14_SVC,  0, 32, "R14_SVC" },
        { UC_ARM_REG_R14_ABT,  0, 32, "R14_ABT" },
        { UC_ARM_REG_R14_UND,  0, 32, "R14_UND" },
        { UC_ARM_REG_R14_IRQ,  0, 32, "R14_IRQ" },
        { UC_ARM_REG_R14_FIQ,  0, 32, "R14_FIQ" },
        { UC_ARM_REG_R14_HYP,  0, 32, "R14_HYP" },
        { UC_ARM_REG_R14_MON,  0, 32, "R14_MON" },

        /* aarch32 status register and bitfields */
        { UC_ARM_REG_CPSR,  0, 32, "CPSR32"     },
        { UC_ARM_REG_CPSR,  0,  5, "CPSR32.M"   },
        { UC_ARM_REG_CPSR,  5,  1, "CPSR32.T"   },
        { UC_ARM_REG_CPSR,  6,  1, "CPSR32.F"   },
        { UC_ARM_REG_CPSR,  7,  1, "CPSR32.I"   },
        { UC_ARM_REG_CPSR,  8,  1, "CPSR32.A"   },
        { UC_ARM_REG_CPSR,  9,  1, "CPSR32.E"   },
        { UC_ARM_REG_CPSR, 10,  6, "CPSR32.IT2" },
        { UC_ARM_REG_CPSR, 16,  4, "CPSR32.GE"  },
        { UC_ARM_REG_CPSR, 24,  1, "CPSR32.J"   },
        { UC_ARM_REG_CPSR, 25,  2, "CPSR32.IT1" },
        { UC_ARM_REG_CPSR, 27,  1, "CPSR32.Q"   },
        { UC_ARM_REG_CPSR, 28,  1, "CPSR32.V"   },
        { UC_ARM_REG_CPSR, 29,  1, "CPSR32.C"   },
        { UC_ARM_REG_CPSR, 30,  1, "CPSR32.Z"   },
        { UC_ARM_REG_CPSR, 31,  1, "CPSR32.N"   },

        /* banked aarch32 status registers and bitfields */
        { UC_ARM_REG_SPSR_SVC,  0, 32, "SPSR_SVC32"     },
        { UC_ARM_REG_SPSR_SVC,  0,  5, "SPSR_SVC32.M"   },
        { UC_ARM_REG_SPSR_SVC,  5,  1, "SPSR_SVC32.T"   },
        { UC_ARM_REG_SPSR_SVC,  6,  1, "SPSR_SVC32.F"   },
        { UC_ARM_REG_SPSR_SVC,  7,  1, "SPSR_SVC32.I"   },
        { UC_ARM_REG_SPSR_SVC,  8,  1, "SPSR_SVC32.A"   },
        { UC_ARM_REG_SPSR_SVC,  9,  1, "SPSR_SVC32.E"   },
        { UC_ARM_REG_SPSR_SVC, 10,  6, "SPSR_SVC32.IT2" },
        { UC_ARM_REG_SPSR_SVC, 16,  4, "SPSR_SVC32.GE"  },
        { UC_ARM_REG_SPSR_SVC, 24,  1, "SPSR_SVC32.J"   },
        { UC_ARM_REG_SPSR_SVC, 25,  2, "SPSR_SVC32.IT1" },
        { UC_ARM_REG_SPSR_SVC, 27,  1, "SPSR_SVC32.Q"   },
        { UC_ARM_REG_SPSR_SVC, 28,  1, "SPSR_SVC32.V"   },
        { UC_ARM_REG_SPSR_SVC, 29,  1, "SPSR_SVC32.C"   },
        { UC_ARM_REG_SPSR_SVC, 30,  1, "SPSR_SVC32.Z"   },
        { UC_ARM_REG_SPSR_SVC, 31,  1, "SPSR_SVC32.N"   },

        { UC_ARM_REG_SPSR_ABT,  0, 32, "SPSR_ABT32"     },
        { UC_ARM_REG_SPSR_ABT,  0,  5, "SPSR_ABT32.M"   },
        { UC_ARM_REG_SPSR_ABT,  5,  1, "SPSR_ABT32.T"   },
        { UC_ARM_REG_SPSR_ABT,  6,  1, "SPSR_ABT32.F"   },
        { UC_ARM_REG_SPSR_ABT,  7,  1, "SPSR_ABT32.I"   },
        { UC_ARM_REG_SPSR_ABT,  8,  1, "SPSR_ABT32.A"   },
        { UC_ARM_REG_SPSR_ABT,  9,  1, "SPSR_ABT32.E"   },
        { UC_ARM_REG_SPSR_ABT, 10,  6, "SPSR_ABT32.IT2" },
        { UC_ARM_REG_SPSR_ABT, 16,  4, "SPSR_ABT32.GE"  },
        { UC_ARM_REG_SPSR_ABT, 24,  1, "SPSR_ABT32.J"   },
        { UC_ARM_REG_SPSR_ABT, 25,  2, "SPSR_ABT32.IT1" },
        { UC_ARM_REG_SPSR_ABT, 27,  1, "SPSR_ABT32.Q"   },
        { UC_ARM_REG_SPSR_ABT, 28,  1, "SPSR_ABT32.V"   },
        { UC_ARM_REG_SPSR_ABT, 29,  1, "SPSR_ABT32.C"   },
        { UC_ARM_REG_SPSR_ABT, 30,  1, "SPSR_ABT32.Z"   },
        { UC_ARM_REG_SPSR_ABT, 31,  1, "SPSR_ABT32.N"   },

        { UC_ARM_REG_SPSR_UND,  0, 32, "SPSR_UND32"     },
        { UC_ARM_REG_SPSR_UND,  0,  5, "SPSR_UND32.M"   },
        { UC_ARM_REG_SPSR_UND,  5,  1, "SPSR_UND32.T"   },
        { UC_ARM_REG_SPSR_UND,  6,  1, "SPSR_UND32.F"   },
        { UC_ARM_REG_SPSR_UND,  7,  1, "SPSR_UND32.I"   },
        { UC_ARM_REG_SPSR_UND,  8,  1, "SPSR_UND32.A"   },
        { UC_ARM_REG_SPSR_UND,  9,  1, "SPSR_UND32.E"   },
        { UC_ARM_REG_SPSR_UND, 10,  6, "SPSR_UND32.IT2" },
        { UC_ARM_REG_SPSR_UND, 16,  4, "SPSR_UND32.GE"  },
        { UC_ARM_REG_SPSR_UND, 24,  1, "SPSR_UND32.J"   },
        { UC_ARM_REG_SPSR_UND, 25,  2, "SPSR_UND32.IT1" },
        { UC_ARM_REG_SPSR_UND, 27,  1, "SPSR_UND32.Q"   },
        { UC_ARM_REG_SPSR_UND, 28,  1, "SPSR_UND32.V"   },
        { UC_ARM_REG_SPSR_UND, 29,  1, "SPSR_UND32.C"   },
        { UC_ARM_REG_SPSR_UND, 30,  1, "SPSR_UND32.Z"   },
        { UC_ARM_REG_SPSR_UND, 31,  1, "SPSR_UND32.N"   },

        { UC_ARM_REG_SPSR_IRQ,  0, 32, "SPSR_IRQ32"     },
        { UC_ARM_REG_SPSR_IRQ,  0,  5, "SPSR_IRQ32.M"   },
        { UC_ARM_REG_SPSR_IRQ,  5,  1, "SPSR_IRQ32.T"   },
        { UC_ARM_REG_SPSR_IRQ,  6,  1, "SPSR_IRQ32.F"   },
        { UC_ARM_REG_SPSR_IRQ,  7,  1, "SPSR_IRQ32.I"   },
        { UC_ARM_REG_SPSR_IRQ,  8,  1, "SPSR_IRQ32.A"   },
        { UC_ARM_REG_SPSR_IRQ,  9,  1, "SPSR_IRQ32.E"   },
        { UC_ARM_REG_SPSR_IRQ, 10,  6, "SPSR_IRQ32.IT2" },
        { UC_ARM_REG_SPSR_IRQ, 16,  4, "SPSR_IRQ32.GE"  },
        { UC_ARM_REG_SPSR_IRQ, 24,  1, "SPSR_IRQ32.J"   },
        { UC_ARM_REG_SPSR_IRQ, 25,  2, "SPSR_IRQ32.IT1" },
        { UC_ARM_REG_SPSR_IRQ, 27,  1, "SPSR_IRQ32.Q"   },
        { UC_ARM_REG_SPSR_IRQ, 28,  1, "SPSR_IRQ32.V"   },
        { UC_ARM_REG_SPSR_IRQ, 29,  1, "SPSR_IRQ32.C"   },
        { UC_ARM_REG_SPSR_IRQ, 30,  1, "SPSR_IRQ32.Z"   },
        { UC_ARM_REG_SPSR_IRQ, 31,  1, "SPSR_IRQ32.N"   },

        { UC_ARM_REG_SPSR_FIQ,  0, 32, "SPSR_FIQ32"     },
        { UC_ARM_REG_SPSR_FIQ,  0,  5, "SPSR_FIQ32.M"   },
        { UC_ARM_REG_SPSR_FIQ,  5,  1, "SPSR_FIQ32.T"   },
        { UC_ARM_REG_SPSR_FIQ,  6,  1, "SPSR_FIQ32.F"   },
        { UC_ARM_REG_SPSR_FIQ,  7,  1, "SPSR_FIQ32.I"   },
        { UC_ARM_REG_SPSR_FIQ,  8,  1, "SPSR_FIQ32.A"   },
        { UC_ARM_REG_SPSR_FIQ,  9,  1, "SPSR_FIQ32.E"   },
        { UC_ARM_REG_SPSR_FIQ, 10,  6, "SPSR_FIQ32.IT2" },
        { UC_ARM_REG_SPSR_FIQ, 16,  4, "SPSR_FIQ32.GE"  },
        { UC_ARM_REG_SPSR_FIQ, 24,  1, "SPSR_FIQ32.J"   },
        { UC_ARM_REG_SPSR_FIQ, 25,  2, "SPSR_FIQ32.IT1" },
        { UC_ARM_REG_SPSR_FIQ, 27,  1, "SPSR_FIQ32.Q"   },
        { UC_ARM_REG_SPSR_FIQ, 28,  1, "SPSR_FIQ32.V"   },
        { UC_ARM_REG_SPSR_FIQ, 29,  1, "SPSR_FIQ32.C"   },
        { UC_ARM_REG_SPSR_FIQ, 30,  1, "SPSR_FIQ32.Z"   },
        { UC_ARM_REG_SPSR_FIQ, 31,  1, "SPSR_FIQ32.N"   },

        { UC_ARM_REG_SPSR_HYP,  0, 32, "SPSR_HYP32"     },
        { UC_ARM_REG_SPSR_HYP,  0,  5, "SPSR_HYP32.M"   },
        { UC_ARM_REG_SPSR_HYP,  5,  1, "SPSR_HYP32.T"   },
        { UC_ARM_REG_SPSR_HYP,  6,  1, "SPSR_HYP32.F"   },
        { UC_ARM_REG_SPSR_HYP,  7,  1, "SPSR_HYP32.I"   },
        { UC_ARM_REG_SPSR_HYP,  8,  1, "SPSR_HYP32.A"   },
        { UC_ARM_REG_SPSR_HYP,  9,  1, "SPSR_HYP32.E"   },
        { UC_ARM_REG_SPSR_HYP, 10,  6, "SPSR_HYP32.IT2" },
        { UC_ARM_REG_SPSR_HYP, 16,  4, "SPSR_HYP32.GE"  },
        { UC_ARM_REG_SPSR_HYP, 24,  1, "SPSR_HYP32.J"   },
        { UC_ARM_REG_SPSR_HYP, 25,  2, "SPSR_HYP32.IT1" },
        { UC_ARM_REG_SPSR_HYP, 27,  1, "SPSR_HYP32.Q"   },
        { UC_ARM_REG_SPSR_HYP, 28,  1, "SPSR_HYP32.V"   },
        { UC_ARM_REG_SPSR_HYP, 29,  1, "SPSR_HYP32.C"   },
        { UC_ARM_REG_SPSR_HYP, 30,  1, "SPSR_HYP32.Z"   },
        { UC_ARM_REG_SPSR_HYP, 31,  1, "SPSR_HYP32.N"   },

        { UC_ARM_REG_SPSR_MON,  0, 32, "SPSR_MON32"     },
        { UC_ARM_REG_SPSR_MON,  0,  5, "SPSR_MON32.M"   },
        { UC_ARM_REG_SPSR_MON,  5,  1, "SPSR_MON32.T"   },
        { UC_ARM_REG_SPSR_MON,  6,  1, "SPSR_MON32.F"   },
        { UC_ARM_REG_SPSR_MON,  7,  1, "SPSR_MON32.I"   },
        { UC_ARM_REG_SPSR_MON,  8,  1, "SPSR_MON32.A"   },
        { UC_ARM_REG_SPSR_MON,  9,  1, "SPSR_MON32.E"   },
        { UC_ARM_REG_SPSR_MON, 10,  6, "SPSR_MON32.IT2" },
        { UC_ARM_REG_SPSR_MON, 16,  4, "SPSR_MON32.GE"  },
        { UC_ARM_REG_SPSR_MON, 24,  1, "SPSR_MON32.J"   },
        { UC_ARM_REG_SPSR_MON, 25,  2, "SPSR_MON32.IT1" },
        { UC_ARM_REG_SPSR_MON, 27,  1, "SPSR_MON32.Q"   },
        { UC_ARM_REG_SPSR_MON, 28,  1, "SPSR_MON32.V"   },
        { UC_ARM_REG_SPSR_MON, 29,  1, "SPSR_MON32.C"   },
        { UC_ARM_REG_SPSR_MON, 30,  1, "SPSR_MON32.Z"   },
        { UC_ARM_REG_SPSR_MON, 31,  1, "SPSR_MON32.N"   },

        /* aarch32 system control registers */
        { UC_ARM_REG_SCR,           0, 32, "SCR"           },
        { UC_ARM_REG_VBAR,          0, 32, "VBAR"          },
        { UC_ARM_REG_VBAR_S,        0, 32, "VBAR_S"        },
        { UC_ARM_REG_VBAR_NS,       0, 32, "VBAR_NS"       },
        { UC_ARM_REG_DACR,          0, 32, "DACR32"        },
        { UC_ARM_REG_DACR_S,        0, 32, "DACR32_S"      },
        { UC_ARM_REG_DACR_NS,       0, 32, "DACR32_NS"     },
        { UC_ARM_REG_SCTLR,         0, 32, "SCTLR"         },
        { UC_ARM_REG_SCTLR_S,       0, 32, "SCTLR_S"       },
        { UC_ARM_REG_SCTLR_NS,      0, 32, "SCTLR_NS"      },
        { UC_ARM_REG_FCSEIDR,       0, 32, "FCSEIDR"       },
        { UC_ARM_REG_FCSEIDR_S,     0, 32, "FCSEIDR_S"     },
        { UC_ARM_REG_FCSEIDR_NS,    0, 32, "FCSEIDR_NS"    },
        { UC_ARM_REG_CONTEXTIDR,    0, 32, "CONTEXTIDR"    },
        { UC_ARM_REG_CONTEXTIDR_S,  0, 32, "CONTEXTIDR_S"  },
        { UC_ARM_REG_CONTEXTIDR_NS, 0, 32, "CONTEXTIDR_NS" },
        { UC_ARM_REG_TTBR0,         0, 32, "TTBR0"         },
        { UC_ARM_REG_TTBR0_S,       0, 32, "TTBR0_S"       },
        { UC_ARM_REG_TTBR0_NS,      0, 32, "TTBR0_NS"      },
        { UC_ARM_REG_TTBR1,         0, 32, "TTBR1"         },
        { UC_ARM_REG_TTBR1_S,       0, 32, "TTBR1_S"       },
        { UC_ARM_REG_TTBR1_NS,      0, 32, "TTBR1_NS"      },
        { UC_ARM_REG_TTBCR,         0, 32, "TTBCR"         },
        { UC_ARM_REG_TTBCR_S,       0, 32, "TTBCR_S"       },
        { UC_ARM_REG_TTBCR_NS,      0, 32, "TTBCR_NS"      },
        { UC_ARM_REG_PRRR,          0, 32, "PRRR"          },
        { UC_ARM_REG_PRRR_S,        0, 32, "PRRR_S"        },
        { UC_ARM_REG_PRRR_NS,       0, 32, "PRRR_NS"       },
        { UC_ARM_REG_NMRR,          0, 32, "NMRR"          },
        { UC_ARM_REG_NMRR_S,        0, 32, "NMRR_S"        },
        { UC_ARM_REG_NMRR_NS,       0, 32, "NMRR_NS"       },
        { UC_ARM_REG_DBGDSCREXT,    0, 32, "DBGDSCREXT"    },
        { UC_ARM_REG_NOIMP,         0, 32, "DBGDTRRX"      },
        { UC_ARM_REG_NOIMP,         0, 32, "DBGDTRTX"      },
        { UC_ARM_REG_MPIDR,         0, 32, "MPIDR"         },

        /* aarch32 floating point registers */
        { UC_ARM_REG_D0,   0, 64, "D0"  },
        { UC_ARM_REG_D1,   0, 64, "D1"  },
        { UC_ARM_REG_D2,   0, 64, "D2"  },
        { UC_ARM_REG_D3,   0, 64, "D3"  },
        { UC_ARM_REG_D4,   0, 64, "D4"  },
        { UC_ARM_REG_D5,   0, 64, "D5"  },
        { UC_ARM_REG_D6,   0, 64, "D6"  },
        { UC_ARM_REG_D7,   0, 64, "D7"  },
        { UC_ARM_REG_D8,   0, 64, "D8"  },
        { UC_ARM_REG_D9,   0, 64, "D9"  },
        { UC_ARM_REG_D10,  0, 64, "D10" },
        { UC_ARM_REG_D11,  0, 64, "D11" },
        { UC_ARM_REG_D12,  0, 64, "D12" },
        { UC_ARM_REG_D13,  0, 64, "D13" },
        { UC_ARM_REG_D14,  0, 64, "D14" },
        { UC_ARM_REG_D15,  0, 64, "D15" },
        { UC_ARM_REG_D16,  0, 64, "D16" },
        { UC_ARM_REG_D17,  0, 64, "D17" },
        { UC_ARM_REG_D18,  0, 64, "D18" },
        { UC_ARM_REG_D19,  0, 64, "D19" },
        { UC_ARM_REG_D20,  0, 64, "D20" },
        { UC_ARM_REG_D21,  0, 64, "D21" },
        { UC_ARM_REG_D22,  0, 64, "D22" },
        { UC_ARM_REG_D23,  0, 64, "D23" },
        { UC_ARM_REG_D24,  0, 64, "D24" },
        { UC_ARM_REG_D25,  0, 64, "D25" },
        { UC_ARM_REG_D26,  0, 64, "D26" },
        { UC_ARM_REG_D27,  0, 64, "D27" },
        { UC_ARM_REG_D28,  0, 64, "D28" },
        { UC_ARM_REG_D29,  0, 64, "D29" },
        { UC_ARM_REG_D30,  0, 64, "D30" },
        { UC_ARM_REG_D31,  0, 64, "D31" },

        { UC_ARM_REG_FPSCR, 0, 32, "FPSCR" },
        { UC_ARM_REG_FPEXC, 0, 32, "FPEXC" },
        { UC_ARM_REG_FPSID, 0, 32, "FPSID" },
        { UC_ARM_REG_MVFR0, 0, 32, "MVFR0" },
        { UC_ARM_REG_MVFR1, 0, 32, "MVFR1" },
        { UC_ARM_REG_MVFR2, 0, 32, "MVFR2" },

        /* end marker */
        { ~0, ~0, ~0, nullptr },
    };

    const reg* lookup_reg(int rid) {
        for (const auto& r : g_regdb)
            if (r.id == rid)
                return &r;
        return nullptr;
    }

    static const reg* REGS64 = lookup_reg(UC_ARM64_REG_X0);
    static const reg* REGS32 = lookup_reg(UC_ARM_REG_R0);

    static const unsigned int NREGS64 = lookup_reg(~0) - REGS64;
    static const unsigned int NREGS32 = lookup_reg(~0) - REGS32;

    static const model g_modeldb[] = {
        { "Cortex-M0",  "ARMv7-M", 32, REGS32, NREGS32 },
        { "Cortex-M3",  "ARMv7-M", 32, REGS32, NREGS32 },
        { "Cortex-M4",  "ARMv7-M", 32, REGS32, NREGS32 },
        { "Cortex-M33", "ARMv7-M", 32, REGS32, NREGS32 },

        { "Cortex-R5",  "ARMv7-R", 32, REGS32, NREGS32 },
        { "Cortex-R5F", "ARMv7-R", 32, REGS32, NREGS32 },

        { "Cortex-A7",  "ARMv7-A", 32, REGS32, NREGS32 },
        { "Cortex-A8",  "ARMv7-A", 32, REGS32, NREGS32 },
        { "Cortex-A9",  "ARMv7-A", 32, REGS32, NREGS32 },
        { "Cortex-A15", "ARMv7-A", 32, REGS32, NREGS32 },

        { "Cortex-A53", "ARMv8-A", 64, REGS64, NREGS64 },
        { "Cortex-A57", "ARMv8-A", 64, REGS64, NREGS64 },
        { "Cortex-A72", "ARMv8-A", 64, REGS64, NREGS64 },
        { "Cortex-Max", "ARMv8-A", 64, REGS64, NREGS64 },
    };

    const model* lookup_model(const char* name) {
        for (const auto& m : g_modeldb)
            if (strcmp(name, m.name) == 0)
                return &m;
        return nullptr;
    }

    namespace {
        class reg_db_consistency_checker {
        public:

            struct cmp {
                bool operator()(const char *a, const char *b) const {
                    return strcmp(a,b) < 0;
                }
             };

            reg_db_consistency_checker() {
                std::set<const char *, cmp> known_reg_handles;
                bool found_error = false;
                for (const reg *r = g_regdb; r->name; ++r) {
                    if (!known_reg_handles.insert(r->name).second) {
                        INFO("duplicate register handle %s", r->name);
                        found_error = true;
                    }
                }

                ERROR_ON(found_error, "duplicate register handles found");
            }
        };

        static reg_db_consistency_checker g_check;
    }
};
