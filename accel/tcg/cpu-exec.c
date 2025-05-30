/*
 *  emulator main execution loop
 *
 *  Copyright (c) 2003-2005 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu-common.h"
#include "qemu/qemu-print.h"
#include "hw/core/tcg-cpu-ops.h"
#include "trace.h"
#include "disas/disas.h"
#include "exec/exec-all.h"
#include "tcg/tcg.h"
#include "qemu/atomic.h"
#include "qemu/compiler.h"
#include "qemu/timer.h"
#include "qemu/rcu.h"
#include "exec/tb-hash.h"
#include "exec/tb-lookup.h"
#include "exec/log.h"
#include "qemu/main-loop.h"
#if defined(CONFIG_LATX_KZT)
#include "qemu.h"
#include "elfloader_private.h"
#include "box64context.h"
#include "librarian.h"
#include "debug.h"
#include "tcg/tcg.h"
#include "library.h"
#include "fileutils.h"
#include "bridge_private.h"
extern const char *interp_prefix;
extern struct elfheader_s * elf_header;
#endif
#if defined(TARGET_I386) && !defined(CONFIG_USER_ONLY)
#include "hw/i386/apic.h"
#endif
#include "sysemu/cpus.h"
#include "exec/cpu-all.h"
#include "sysemu/cpu-timers.h"
#include "sysemu/replay.h"
#include "internal.h"
#include "qemu/cacheflush.h"
#ifdef CONFIG_LATX_AOT
#include "aot_recover_tb.h"
#endif
#ifdef CONFIG_LATX_PERF
#include "latx-perf.h"
#endif
/* -icount align implementation. */

typedef struct SyncClocks {
    int64_t diff_clk;
    int64_t last_cpu_icount;
    int64_t realtime_clock;
} SyncClocks;

#if !defined(CONFIG_USER_ONLY)
/* Allow the guest to have a max 3ms advance.
 * The difference between the 2 clocks could therefore
 * oscillate around 0.
 */
#define VM_CLOCK_ADVANCE 3000000
#define THRESHOLD_REDUCE 1.5
#define MAX_DELAY_PRINT_RATE 2000000000LL
#define MAX_NB_PRINTS 100
static int64_t max_delay;
static int64_t max_advance;

static void align_clocks(SyncClocks *sc, CPUState *cpu)
{
    int64_t cpu_icount;

    if (!icount_align_option) {
        return;
    }

    cpu_icount = cpu->icount_extra + cpu_neg(cpu)->icount_decr.u16.low;
    sc->diff_clk += icount_to_ns(sc->last_cpu_icount - cpu_icount);
    sc->last_cpu_icount = cpu_icount;

    if (sc->diff_clk > VM_CLOCK_ADVANCE) {
#ifndef _WIN32
        struct timespec sleep_delay, rem_delay;
        sleep_delay.tv_sec = sc->diff_clk / 1000000000LL;
        sleep_delay.tv_nsec = sc->diff_clk % 1000000000LL;
        if (nanosleep(&sleep_delay, &rem_delay) < 0) {
            sc->diff_clk = rem_delay.tv_sec * 1000000000LL + rem_delay.tv_nsec;
        } else {
            sc->diff_clk = 0;
        }
#else
        Sleep(sc->diff_clk / SCALE_MS);
        sc->diff_clk = 0;
#endif
    }
}

static void print_delay(const SyncClocks *sc)
{
    static float threshold_delay;
    static int64_t last_realtime_clock;
    static int nb_prints;

    if (icount_align_option &&
        sc->realtime_clock - last_realtime_clock >= MAX_DELAY_PRINT_RATE &&
        nb_prints < MAX_NB_PRINTS) {
        if ((-sc->diff_clk / (float)1000000000LL > threshold_delay) ||
            (-sc->diff_clk / (float)1000000000LL <
             (threshold_delay - THRESHOLD_REDUCE))) {
            threshold_delay = (-sc->diff_clk / 1000000000LL) + 1;
            qemu_printf("Warning: The guest is now late by %.1f to %.1f seconds\n",
                        threshold_delay - 1,
                        threshold_delay);
            nb_prints++;
            last_realtime_clock = sc->realtime_clock;
        }
    }
}

static void init_delay_params(SyncClocks *sc, CPUState *cpu)
{
    if (!icount_align_option) {
        return;
    }
    sc->realtime_clock = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL_RT);
    sc->diff_clk = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) - sc->realtime_clock;
    sc->last_cpu_icount
        = cpu->icount_extra + cpu_neg(cpu)->icount_decr.u16.low;
    if (sc->diff_clk < max_delay) {
        max_delay = sc->diff_clk;
    }
    if (sc->diff_clk > max_advance) {
        max_advance = sc->diff_clk;
    }

    /* Print every 2s max if the guest is late. We limit the number
       of printed messages to NB_PRINT_MAX(currently 100) */
    print_delay(sc);
}
#else
static void align_clocks(SyncClocks *sc, const CPUState *cpu)
{
}

static void init_delay_params(SyncClocks *sc, const CPUState *cpu)
{
}
#endif /* CONFIG USER ONLY */

#ifdef CONFIG_LATX
#include "latx-config.h"
#include "latx-options.h"
#include "latx-signal.h"
#include "reg-map.h"
#endif

/* Execute a TB, and fix up the CPU state afterwards if necessary */
/*
 * Disable CFI checks.
 * TCG creates binary blobs at runtime, with the transformed code.
 * A TB is a blob of binary code, created at runtime and called with an
 * indirect function call. Since such function did not exist at compile time,
 * the CFI runtime has no way to verify its signature and would fail.
 * TCG is not considered a security-sensitive part of QEMU so this does not
 * affect the impact of CFI in environment with high security requirements
 */
static inline TranslationBlock * QEMU_DISABLE_CFI
cpu_tb_exec(CPUState *cpu, TranslationBlock *itb, int *tb_exit)
{
    if (!itb) {
        fprintf(stderr, "[LATX] ERROR!\n");
    }
    CPUArchState *env = cpu->env_ptr;
    uintptr_t ret;
    TranslationBlock *last_tb;
    const void *tb_ptr = itb->tc.ptr;
#ifdef CONFIG_LATX_KZT
    if (qemu_loglevel_mask(CPU_LOG_EXEC)) {
        Dl_info dl_info;
        if (option_kzt && itb->pc > reserved_va &&
            dladdr ((const void *)((onebridge_t *)itb->pc)->f, &dl_info)) {
            qemu_log_mask_and_addr(CPU_LOG_EXEC, itb->pc,
                   "%d Trace %d: %p ["
                   TARGET_FMT_lx "/" "%016llx" "/" TARGET_FMT_lx
                   "/%#x] KZT:%s\n", getpid(), cpu->cpu_index, itb->tc.ptr,
                   itb->cs_base, (unsigned long long)pthread_self(),
                   itb->pc, itb->flags, dl_info.dli_sname);
        } else {
            qemu_log_mask_and_addr(CPU_LOG_EXEC, itb->pc,
                   "%d Trace %d: %p ["
                   TARGET_FMT_lx "/" "%016llx" "/" TARGET_FMT_lx
                   "/%#x] %s\n", getpid(), cpu->cpu_index, itb->tc.ptr,
                   itb->cs_base, (unsigned long long)pthread_self(),
                   itb->pc, itb->flags, lookup_symbol(itb->pc));
        }
    }
#endif
#ifdef CONFIG_LATX_DEBUG
    if (strlen(lookup_symbol(itb->pc)) > 0) {
        if (env->last_func_index == -1) {
            env->call_func[0] = (char *)lookup_symbol(itb->pc);
            env->last_func_index = 0;
            env->func_index = 1;
        } else if (strcmp(env->call_func[env->last_func_index],
                    lookup_symbol(itb->pc))) {
            env->call_func[env->func_index] = (char *)lookup_symbol(itb->pc);
            env->last_func_index = env->func_index;
            env->func_index = (env->func_index + 1) % FUNC_DEPTH;
        }
    }

    env->tb_exec_count++;
    if (unlikely(latx_unlink_count) && qemu_loglevel_mask(CPU_LOG_TB_NOCHAIN) &&
        latx_unlink_cpu == cpu->cpu_index) {
        printf("Trace %d cpu%d [0x" TARGET_FMT_lx "] "
               "eax 0x%016lx ecx 0x%016lx edx 0x%016lx ebx 0x%016lx "
               "esp 0x%016lx ebp 0x%016lx esi 0x%016lx edi 0x%016lx\n",
               getpid(), cpu->cpu_index, itb->pc,
               env->regs[0], env->regs[1], env->regs[2], env->regs[3],
               env->regs[4], env->regs[5], env->regs[6], env->regs[7]);
    }
#endif

#if defined(DEBUG_DISAS)
    if (qemu_loglevel_mask(CPU_LOG_TB_CPU)
        && qemu_log_in_addr_range(itb->pc)) {
        FILE *logfile = qemu_log_lock();
        int flags = 0;
        if (qemu_loglevel_mask(CPU_LOG_TB_FPU)) {
            flags |= CPU_DUMP_FPU;
        }
#if defined(TARGET_I386)
        flags |= CPU_DUMP_CCOP;
#endif
        log_cpu_state(cpu, flags);
        qemu_log_unlock(logfile);
    }
#endif /* DEBUG_DISAS */

    qemu_thread_jit_execute();
#ifdef CONFIG_LATX_DEBUG
    latx_before_exec_trace_tb(env, itb);
#endif
#ifdef CONFIG_LATX

    env->fpu_clobber = false;
    ret = tcg_qemu_tb_exec(env, tb_ptr);

    if (option_monitor_shared_mem && env->checksum_fail_tb) {
        TranslationBlock * tb_fail = (TranslationBlock *)env->checksum_fail_tb;
        lsassert(tb_fail->checksum && tb_fail->pc == env->eip);
        mmap_lock();
        tb_phys_invalidate(tb_fail, tb_page_addr0(tb_fail));
        mmap_unlock();
        env->checksum_fail_tb = NULL;
        /*qemu_log("latx checksum fail, retranslate pc=%lx\n", tb_fail->pc);*/
    }

    if (env->insn_save[0]) {
        link_indirect_jmp(env);
    }

#else
    ret = tcg_qemu_tb_exec(env, tb_ptr);
#endif
#ifdef CONFIG_LATX_DEBUG
    latx_after_exec_trace_tb(env, itb);
#endif
    cpu->can_do_io = 1;
    /*
     * TODO: Delay swapping back to the read-write region of the TB
     * until we actually need to modify the TB.  The read-only copy,
     * coming from the rx region, shares the same host TLB entry as
     * the code that executed the exit_tb opcode that arrived here.
     * If we insist on touching both the RX and the RW pages, we
     * double the host TLB pressure.
     */
    if (option_split_tb) {
        last_tb = (void *)(ret & ~TB_EXIT_MASK);
    } else {
        last_tb = tcg_splitwx_to_rw((void *)(ret & ~TB_EXIT_MASK));
    }
    *tb_exit = ret & TB_EXIT_MASK;

    trace_exec_tb_exit(last_tb, *tb_exit);
    if (last_tb) {
        if (last_tb->signal_unlink[0]) {
            last_tb->signal_unlink[0] = 2;
        }
        if (last_tb->signal_unlink[1]) {
            last_tb->signal_unlink[1] = 2;
        }
    }

    if (*tb_exit > TB_EXIT_IDX1) {
        /* We didn't start executing this TB (eg because the instruction
         * counter hit zero); we must restore the guest PC to the address
         * of the start of the TB.
         */
        CPUClass *cc = CPU_GET_CLASS(cpu);
        qemu_log_mask_and_addr(CPU_LOG_EXEC, last_tb->pc,
                               "Stopped execution of TB chain before %p ["
                               TARGET_FMT_lx "] %s\n",
                               last_tb->tc.ptr, last_tb->pc,
                               lookup_symbol(last_tb->pc));
        if (cc->tcg_ops->synchronize_from_tb) {
            cc->tcg_ops->synchronize_from_tb(cpu, last_tb);
        } else {
            assert(cc->set_pc);
            cc->set_pc(cpu, last_tb->pc);
        }
    }

#ifdef CONFIG_LATX_DEBUG
    if (unlikely(latx_unlink_count) &&
        latx_unlink_cpu == cpu->cpu_index &&
        env->tb_exec_count >= latx_unlink_count) {
        qemu_loglevel_set(CPU_LOG_TB_NOCHAIN);
        tb_flush(cpu);
    }
#endif

    return last_tb;
}


static void cpu_exec_enter(CPUState *cpu)
{
    CPUClass *cc = CPU_GET_CLASS(cpu);

    if (cc->tcg_ops->cpu_exec_enter) {
        cc->tcg_ops->cpu_exec_enter(cpu);
    }
}

static void cpu_exec_exit(CPUState *cpu)
{
    CPUClass *cc = CPU_GET_CLASS(cpu);

    if (cc->tcg_ops->cpu_exec_exit) {
        cc->tcg_ops->cpu_exec_exit(cpu);
    }
}

void cpu_exec_step_atomic(CPUState *cpu)
{
    CPUArchState *env = (CPUArchState *)cpu->env_ptr;
    TranslationBlock *tb;
    target_ulong cs_base, pc;
    uint32_t flags;
    uint32_t cflags = (curr_cflags(cpu) & ~CF_PARALLEL) | 1;
    int tb_exit;

    if (sigsetjmp(cpu->jmp_env, 0) == 0) {
        start_exclusive();
        g_assert(cpu == current_cpu);
        g_assert(!cpu->running);
        cpu->running = true;

        cpu_get_tb_cpu_state(env, &pc, &cs_base, &flags);
        tb = tb_lookup(cpu, pc, cs_base, flags, cflags);

        if (tb == NULL) {
            mmap_lock();
            tb = tb_gen_code(cpu, pc, cs_base, flags, cflags);
            mmap_unlock();
        }

        cpu_exec_enter(cpu);
        /* execute the generated code */
        trace_exec_tb(tb, pc);
        cpu_tb_exec(cpu, tb, &tb_exit);
        cpu_exec_exit(cpu);
    } else {
        /*
         * The mmap_lock is dropped by tb_gen_code if it runs out of
         * memory.
         */
#ifndef CONFIG_SOFTMMU
        tcg_debug_assert(!have_mmap_lock());
#endif
        if (qemu_mutex_iothread_locked()) {
            qemu_mutex_unlock_iothread();
        }
        assert_no_pages_locked();
        qemu_plugin_disable_mem_helpers(cpu);
    }


    /*
     * As we start the exclusive region before codegen we must still
     * be in the region if we longjump out of either the codegen or
     * the execution.
     */
    g_assert(cpu_in_exclusive_context(cpu));
    cpu->running = false;
    end_exclusive();
}

struct tb_desc {
    target_ulong pc;
    target_ulong cs_base;
    CPUArchState *env;
    tb_page_addr_t page_addr0;
    uint32_t flags;
    uint32_t cflags;
    uint32_t trace_vcpu_dstate;
};

static bool tb_lookup_cmp(const void *p, const void *d)
{
    const TranslationBlock *tb = p;
    const struct tb_desc *desc = d;

    if (tb->pc == desc->pc &&
        tb_page_addr0(tb) == desc->page_addr0 &&
        tb->cs_base == desc->cs_base &&
        tb->flags == desc->flags &&
        tb->trace_vcpu_dstate == desc->trace_vcpu_dstate &&
        tb_cflags(tb) == desc->cflags) {
        /* check next page if needed */
        tb_page_addr_t tb_phys_page1 = tb_page_addr1(tb);
        if (tb_phys_page1 == -1) {
            return true;
        } else {
            tb_page_addr_t phys_page1;
            target_ulong virt_page1;

            virt_page1 = TARGET_PAGE_ALIGN(desc->pc);
            phys_page1 = get_page_addr_code(desc->env, virt_page1);
            if (tb_phys_page1 == phys_page1) {
                return true;
            }
        }
    }
    return false;
}

TranslationBlock *tb_htable_lookup(CPUState *cpu, target_ulong pc,
                                   target_ulong cs_base, uint32_t flags,
                                   uint32_t cflags)
{
    tb_page_addr_t phys_pc;
    struct tb_desc desc;
    uint32_t h;

    desc.env = (CPUArchState *)cpu->env_ptr;
    desc.cs_base = cs_base;
    desc.flags = flags;
    desc.cflags = cflags;
    desc.trace_vcpu_dstate = *cpu->trace_dstate;
    desc.pc = pc;
    phys_pc = get_page_addr_code(desc.env, pc);
    if (phys_pc == -1) {
        return NULL;
    }
    desc.page_addr0 = phys_pc;
    h = tb_hash_func(phys_pc, pc, flags, cflags, *cpu->trace_dstate);
    return qht_lookup_custom(&tb_ctx.htable, &desc, h, tb_lookup_cmp);
}

#ifdef CONFIG_LATX_INSTS_PATTERN
static void update_inst(TranslationBlock *tb, int n, uint32_t insn)
{
    uintptr_t offset = tb->eflags_target_arg[n];
    uintptr_t tc_ptr = (uintptr_t)tb->tc.ptr;
    uintptr_t jmp_rx = tc_ptr + offset;
    uintptr_t jmp_rw = jmp_rx - tcg_splitwx_diff;
    qatomic_set((uint32_t *)jmp_rw, insn);
    flush_idcache_range(jmp_rx, jmp_rw, 4);
}

void tb_eflag_eliminate(TranslationBlock *tb, int n)
{
    if (n && (tb->bool_flags & OPT_BCC)) {
        return;
    }
    /* NOP */
    uint32_t insn = (0xd) << 22;
    update_inst(tb, n, insn);
}

void tb_eflag_recover(TranslationBlock *tb, int n)
{
    if (n && (tb->bool_flags & OPT_BCC)) {
        return;
    }
    uintptr_t offset = tb->eflags_target_arg[EFLAG_BACKUP];
    uintptr_t tc_ptr = (uintptr_t)tb->tc.ptr;
    uintptr_t jmp_rx = tc_ptr + offset;
    uint32_t insn = qatomic_read((uint32_t *)jmp_rx);
    update_inst(tb, n, insn);
}
#endif

#ifdef CONFIG_LATX_XCOMISX_OPT
void tb_stub_bypass(TranslationBlock *tb, int n, uintptr_t addr)
{
    uintptr_t offset = tb->jmp_stub_target_arg[n];
    uintptr_t tc_ptr = (uintptr_t)tb->tc.ptr;
    uintptr_t jmp_rx = tc_ptr + offset;
    uintptr_t jmp_rw = jmp_rx - tcg_splitwx_diff;
    tb_target_set_jmp_target(tc_ptr, jmp_rx, jmp_rw, addr);
}
#endif

void tb_set_jmp_target(TranslationBlock *tb, int n, uintptr_t addr)
{
#if defined(CONFIG_LATX) && defined(CONFIG_LATX_BNE_B)
#define B_SHIFT     26
#define OFF16_BITS  0xfc0003ff
#define MAX_OFFS    0x00020000

    bool is_ptn = false;
#ifdef CONFIG_LATX_INSTS_PATTERN
    if (tb->eflags_target_arg[0] != TB_JMP_RESET_OFFSET_INVALID) {
        is_ptn = true;
        if (n == 1 && !(tb->bool_flags & TARGET1_ELIMINATE)) {
            tb->bool_flags &= ~OPT_BCC;
        }
    }
#endif

    /*
     * The code is used to optimizate condition JMP
     * Condition JMP is translated to BCC + B + B
     *
     *    before                 optimization
     * BCC b1_offset   ---->    BCC tb1_offset
     * B tb0_offset             B tb0_offset
     * B tb1_offset             B tb1_offset
     */
    if (n == 1 && /* taken b */
        ((tb->jmp_reset_offset[0] | tb->jmp_reset_offset[1]) !=
        TB_JMP_RESET_OFFSET_INVALID) && /* direct jmp */
        (tb->bool_flags & OPT_BCC)) {
        uintptr_t bcc_addr;
        uint32_t bcc_insn;

        /* get bcc_insn */
        int ptn_off = is_ptn ? 4 : 0;
        bcc_addr = (uintptr_t)tb->tc.ptr + tb->first_jmp_align - ptn_off - 4;
        bcc_insn = *(uint *)bcc_addr;

        /* get taken b addr */
        uintptr_t taken_b_addr = (uintptr_t)tb->tc.ptr + tb->jmp_target_arg[1];

        uint32_t b_opcode = bcc_insn >> B_SHIFT;
        /* BEQ BNE BLT BGE BLTU BGEU */
        if (b_opcode >= 0x16 && b_opcode <= 0x1b) {
            long offset;
            if (addr - taken_b_addr == B_STUB_SIZE) {
                /* if the function is used to unlink */
                offset = tb->jmp_target_arg[1] - tb->first_jmp_align + 4;
                bcc_insn &= OFF16_BITS;
                bcc_insn |= (offset << 8) & ~OFF16_BITS;
                *(uint *)(bcc_addr) = bcc_insn;
            } else {
                /* if the function is used to patch second B */
                offset = addr - bcc_addr;
                if ((offset < MAX_OFFS) && (offset >= -MAX_OFFS)) {
                    bcc_insn &= OFF16_BITS;
                    bcc_insn |= (offset << 8) & ~OFF16_BITS;
                    *(uint *)(bcc_addr) = bcc_insn;
                }
            }
            flush_idcache_range(bcc_addr, bcc_addr, 4);
        }
    }
#endif
    if (TCG_TARGET_HAS_direct_jump) {
        uintptr_t offset = tb->jmp_target_arg[n];
        uintptr_t tc_ptr = (uintptr_t)tb->tc.ptr;
        uintptr_t jmp_rx = tc_ptr + offset;
        uintptr_t jmp_rw = jmp_rx - tcg_splitwx_diff;
        tb_target_set_jmp_target(tc_ptr, jmp_rx, jmp_rw, addr);
    } else {
        tb->jmp_target_arg[n] = addr;
    }
}

#ifdef CONFIG_LATX_TU
void tu_relink(TranslationBlock *tb) {
    /* fprintf(stderr, "relink\n"); */
    uint32_t *tu_jmp_addr =
        (uint32_t *)(tb->tc.ptr + tb->tu_jmp[TU_TB_INDEX_TARGET]);
    *tu_jmp_addr = tb->tu_link_ins;
    flush_idcache_range((uintptr_t)tu_jmp_addr, (uintptr_t)tu_jmp_addr, 4);
}
#endif

static inline void tb_add_jump(TranslationBlock *tb, int n,
                               TranslationBlock *tb_next)
{
    uintptr_t old;

    qemu_thread_jit_write();
    assert(n < ARRAY_SIZE(tb->jmp_list_next));
    qemu_spin_lock(&tb_next->jmp_lock);

    /* make sure the destination TB is valid */
    if (tb_next->cflags & CF_INVALID) {
        goto out_unlock_next;
    }

#ifdef CONFIG_LATX_TU
    if (tb->tu_jmp[TU_TB_INDEX_TARGET] != TB_JMP_RESET_OFFSET_INVALID
            && tb->tu_unlink_stub_offset != TU_UNLINK_STUB_INVALID) {
        tu_relink(tb);
        goto out_unlock_next;
    }
#endif

    /* Atomically claim the jump destination slot only if it was NULL */
    old = qatomic_cmpxchg(&tb->jmp_dest[n], (uintptr_t)NULL,
                          (uintptr_t)tb_next);
    if (old) {
        goto out_unlock_next;
    } else if (tb->signal_unlink[n] == 2) {
        tb->signal_unlink[n] = 0;
        latx_tb_set_jmp_target(tb, n, tb_next);
        goto out_unlock_next;
    }

#ifdef CONFIG_LATX
    /* check fpu rotate and patch the native jump address */
    latx_tb_set_jmp_target(tb, n, tb_next);
#else
    /* patch the native jump address */
    tb_set_jmp_target(tb, n, (uintptr_t)tb_next->tc.ptr);
#endif

    /* add in TB jmp list */
    tb->jmp_list_next[n] = tb_next->jmp_list_head;
    tb_next->jmp_list_head = (uintptr_t)tb | n;

    qemu_spin_unlock(&tb_next->jmp_lock);

    qemu_log_mask_and_addr(CPU_LOG_EXEC, tb->pc,
                           "Linking TBs %p [" TARGET_FMT_lx
                           "] index %d -> %p [" TARGET_FMT_lx "]\n",
                           tb->tc.ptr, tb->pc, n,
                           tb_next->tc.ptr, tb_next->pc);
    return;

 out_unlock_next:
    qemu_spin_unlock(&tb_next->jmp_lock);
    return;
}

#ifdef CONFIG_LATX_TU
#include "tu.h"
#endif
#ifdef CONFIG_LATX
#include "opt-jmp.h"
#endif

static inline TranslationBlock *tb_find(CPUState *cpu,
                                        TranslationBlock *last_tb,
                                        int tb_exit, uint32_t cflags)
{
    CPUArchState *env = (CPUArchState *)cpu->env_ptr;
    TranslationBlock *tb;
    target_ulong cs_base, pc;
    uint32_t flags;

    cpu_get_tb_cpu_state(env, &pc, &cs_base, &flags);

    tb = tb_lookup(cpu, pc, cs_base, flags, cflags);
#ifdef CONFIG_LATX_AOT
    if (tb == NULL && option_aot) {
        mmap_lock();
        if (load_page_4(pc, cflags)) {
            tb = tb_lookup(cpu, pc, cs_base, flags, cflags);
        }
        mmap_unlock();
    }
#endif
    if (tb == NULL) {
#if (defined CONFIG_LATX_AOT) && (defined CONFIG_LATX_DEBUG)
        if (option_debug_aot && option_load_aot) {
            static long long cnt;
            fprintf(stderr, "NOTE! Translating No.%lld basic block 0x"
                    TARGET_FMT_lx "\n", ++cnt, pc);
        }
#endif
        mmap_lock();

#ifdef CONFIG_LATX_PERF
    latx_timer_start(TIMER_TS);
#endif

#ifdef CONFIG_LATX_TU
        tb = tb_gen_code(cpu, pc, cs_base, flags, cflags);
        jrra_pre_translate((void **)&tb, 1, cpu, cs_base, flags, cflags);
        /* jrra_pre_translate((void **)tu_data->tb_list, tu_data->tb_num, */
        /*                     cpu, cs_base, flags, cflags); */
#else
        tb = tb_gen_code(cpu, pc, cs_base, flags, cflags);
 #ifdef CONFIG_LATX
        jrra_pre_translate((void **)&tb, 1, cpu, cs_base, flags, cflags);
 #endif
#endif
#ifdef CONFIG_LATX_PERF
    latx_timer_stop(TIMER_TS);
#endif

    if (!tb) {
        mmap_unlock();
        return NULL;
    }
        /* We add the TB in the virtual pc hash table for the fast lookup */
        int hash_value = tb_jmp_cache_hash_func(pc);
        qatomic_set(&cpu->tb_jmp_cache[hash_value], tb);
#ifdef CONFIG_LATX
        if (!close_latx_parallel && !(cpu->tcg_cflags & CF_PARALLEL)) {
            latx_fast_jmp_cache_add(hash_value, tb);
        }
#endif
        mmap_unlock();
    }
#ifndef CONFIG_USER_ONLY
    /* We don't take care of direct jumps when address mapping changes in
     * system emulation. So it's not safe to make a direct jump to a TB
     * spanning two pages because the mapping for the second page can change.
     */
    if (tb->page_addr[1] != -1) {
        last_tb = NULL;
    }
#endif
    /* See if we can patch the calling TB. */
    if (last_tb) {
        tb_add_jump(last_tb, tb_exit, tb);
    }
    return tb;
}

static inline bool cpu_handle_halt(CPUState *cpu)
{
    if (cpu->halted) {
#if defined(TARGET_I386) && !defined(CONFIG_USER_ONLY)
        if (cpu->interrupt_request & CPU_INTERRUPT_POLL) {
            X86CPU *x86_cpu = X86_CPU(cpu);
            qemu_mutex_lock_iothread();
            apic_poll_irq(x86_cpu->apic_state);
            cpu_reset_interrupt(cpu, CPU_INTERRUPT_POLL);
            qemu_mutex_unlock_iothread();
        }
#endif
        if (!cpu_has_work(cpu)) {
            return true;
        }

        cpu->halted = 0;
    }

    return false;
}

static inline void cpu_handle_debug_exception(CPUState *cpu)
{
    CPUClass *cc = CPU_GET_CLASS(cpu);
    CPUWatchpoint *wp;

    if (!cpu->watchpoint_hit) {
        QTAILQ_FOREACH(wp, &cpu->watchpoints, entry) {
            wp->flags &= ~BP_WATCHPOINT_HIT;
        }
    }

    if (cc->tcg_ops->debug_excp_handler) {
        cc->tcg_ops->debug_excp_handler(cpu);
    }
}

#if defined(CONFIG_LATX_KZT)
#include "callback.h"
TranslationBlock * kzt_tb_find_exp(
            CPUState *cpu,
            TranslationBlock *last_tb,
            int tb_exit, uint32_t cflags);
TranslationBlock * kzt_tb_find_exp(
            CPUState *cpu,
            TranslationBlock *last_tb,
            int tb_exit, uint32_t cflags)
{
    return tb_find(cpu, last_tb, tb_exit, cflags);
}

#endif

static inline bool cpu_handle_exception(CPUState *cpu, int *ret)
{
#if defined(CONFIG_LATX_KZT)
    if (option_kzt) {
        CPUArchState *env = cpu->env_ptr;
        if(env->eip == (uint64_t)&RunFunctionWithState){
            *ret = 0xCC;
            return true;
        }
    }
#endif

    if (cpu->exception_index < 0) {
#ifndef CONFIG_USER_ONLY
        if (replay_has_exception()
            && cpu_neg(cpu)->icount_decr.u16.low + cpu->icount_extra == 0) {
            /* Execute just one insn to trigger exception pending in the log */
            cpu->cflags_next_tb = (curr_cflags(cpu) & ~CF_USE_ICOUNT) | 1;
        }
#endif
        return false;
    }
    if (cpu->exception_index >= EXCP_INTERRUPT) {
        /* exit request from the cpu execution loop */
        cpu->previous_exception_index = *ret = cpu->exception_index;
        if (*ret == EXCP_DEBUG) {
            cpu_handle_debug_exception(cpu);
        }
        cpu->exception_index = -1;
        return true;
    } else {
#if defined(CONFIG_USER_ONLY)
        /* if user mode only, we simulate a fake exception
           which will be handled outside the cpu execution
           loop */
#if defined(TARGET_I386)
        CPUClass *cc = CPU_GET_CLASS(cpu);
        cc->tcg_ops->do_interrupt(cpu);
#endif
       cpu->previous_exception_index = *ret = cpu->exception_index;
        cpu->exception_index = -1;
        return true;
#else
        if (replay_exception()) {
            CPUClass *cc = CPU_GET_CLASS(cpu);
            qemu_mutex_lock_iothread();
            cc->tcg_ops->do_interrupt(cpu);
            qemu_mutex_unlock_iothread();
            cpu->exception_index = -1;

            if (unlikely(cpu->singlestep_enabled)) {
                /*
                 * After processing the exception, ensure an EXCP_DEBUG is
                 * raised when single-stepping so that GDB doesn't miss the
                 * next instruction.
                 */
                *ret = EXCP_DEBUG;
                cpu_handle_debug_exception(cpu);
                return true;
            }
        } else if (!replay_has_interrupt()) {
            /* give a chance to iothread in replay mode */
            *ret = EXCP_INTERRUPT;
            return true;
        }
#endif
    }

    return false;
}

/*
 * CPU_INTERRUPT_POLL is a virtual event which gets converted into a
 * "real" interrupt event later. It does not need to be recorded for
 * replay purposes.
 */
static inline bool need_replay_interrupt(int interrupt_request)
{
#if defined(TARGET_I386)
    return !(interrupt_request & CPU_INTERRUPT_POLL);
#else
    return true;
#endif
}

static inline bool cpu_handle_interrupt(CPUState *cpu,
                                        TranslationBlock **last_tb)
{
#if defined(CONFIG_LATX_KZT)
    if (option_kzt) {
        CPUArchState *env = cpu->env_ptr;
        if(env->eip == (uint64_t)&RunFunctionWithState){
            *last_tb = NULL;
            return true;
        }
    }
#endif

    CPUClass *cc = CPU_GET_CLASS(cpu);

    /* Clear the interrupt flag now since we're processing
     * cpu->interrupt_request and cpu->exit_request.
     * Ensure zeroing happens before reading cpu->exit_request or
     * cpu->interrupt_request (see also smp_wmb in cpu_exit())
     */
    qatomic_mb_set(&cpu_neg(cpu)->icount_decr.u16.high, 0);

    if (unlikely(qatomic_read(&cpu->interrupt_request))) {
        int interrupt_request;
        qemu_mutex_lock_iothread();
        interrupt_request = cpu->interrupt_request;
        if (unlikely(cpu->singlestep_enabled & SSTEP_NOIRQ)) {
            /* Mask out external interrupts for this step. */
            interrupt_request &= ~CPU_INTERRUPT_SSTEP_MASK;
        }
        if (interrupt_request & CPU_INTERRUPT_DEBUG) {
            cpu->interrupt_request &= ~CPU_INTERRUPT_DEBUG;
            cpu->exception_index = EXCP_DEBUG;
            qemu_mutex_unlock_iothread();
            return true;
        }
        if (replay_mode == REPLAY_MODE_PLAY && !replay_has_interrupt()) {
            /* Do nothing */
        } else if (interrupt_request & CPU_INTERRUPT_HALT) {
            replay_interrupt();
            cpu->interrupt_request &= ~CPU_INTERRUPT_HALT;
            cpu->halted = 1;
            cpu->exception_index = EXCP_HLT;
            qemu_mutex_unlock_iothread();
            return true;
        }
#if defined(TARGET_I386)
        else if (interrupt_request & CPU_INTERRUPT_INIT) {
            X86CPU *x86_cpu = X86_CPU(cpu);
            CPUArchState *env = &x86_cpu->env;
            replay_interrupt();
            cpu_svm_check_intercept_param(env, SVM_EXIT_INIT, 0, 0);
            do_cpu_init(x86_cpu);
            cpu->exception_index = EXCP_HALTED;
            qemu_mutex_unlock_iothread();
            return true;
        }
#else
        else if (interrupt_request & CPU_INTERRUPT_RESET) {
            replay_interrupt();
            cpu_reset(cpu);
            qemu_mutex_unlock_iothread();
            return true;
        }
#endif
        /* The target hook has 3 exit conditions:
           False when the interrupt isn't processed,
           True when it is, and we should restart on a new TB,
           and via longjmp via cpu_loop_exit.  */
        else {
            if (cc->tcg_ops->cpu_exec_interrupt &&
                cc->tcg_ops->cpu_exec_interrupt(cpu, interrupt_request)) {
                if (need_replay_interrupt(interrupt_request)) {
                    replay_interrupt();
                }
                /*
                 * After processing the interrupt, ensure an EXCP_DEBUG is
                 * raised when single-stepping so that GDB doesn't miss the
                 * next instruction.
                 */
                cpu->exception_index =
                    (cpu->singlestep_enabled ? EXCP_DEBUG : -1);
                *last_tb = NULL;
            }
            /* The target hook may have updated the 'cpu->interrupt_request';
             * reload the 'interrupt_request' value */
            interrupt_request = cpu->interrupt_request;
        }
        if (interrupt_request & CPU_INTERRUPT_EXITTB) {
            cpu->interrupt_request &= ~CPU_INTERRUPT_EXITTB;
            /* ensure that no TB jump will be modified as
               the program flow was changed */
            *last_tb = NULL;
        }

        /* If we exit via cpu_loop_exit/longjmp it is reset in cpu_exec */
        qemu_mutex_unlock_iothread();
    }

    /* Finally, check if we need to exit to the main loop.  */
    if (unlikely(qatomic_read(&cpu->exit_request))
        || (icount_enabled()
            && (cpu->cflags_next_tb == -1 || cpu->cflags_next_tb & CF_USE_ICOUNT)
            && cpu_neg(cpu)->icount_decr.u16.low + cpu->icount_extra == 0)) {
        qatomic_set(&cpu->exit_request, 0);
        if (cpu->exception_index == -1) {
            cpu->exception_index = EXCP_INTERRUPT;
        }
        return true;
    }

    return false;
}

static inline void cpu_loop_exec_tb(CPUState *cpu, TranslationBlock *tb,
                                    TranslationBlock **last_tb, int *tb_exit)
{
    int32_t insns_left;

    trace_exec_tb(tb, tb->pc);
    tb = cpu_tb_exec(cpu, tb, tb_exit);
    if (*tb_exit != TB_EXIT_REQUESTED) {
        *last_tb = tb;
        return;
    }

    *last_tb = NULL;
    insns_left = qatomic_read(&cpu_neg(cpu)->icount_decr.u32);
    if (insns_left < 0) {
        /* Something asked us to stop executing chained TBs; just
         * continue round the main loop. Whatever requested the exit
         * will also have set something else (eg exit_request or
         * interrupt_request) which will be handled by
         * cpu_handle_interrupt.  cpu_handle_interrupt will also
         * clear cpu->icount_decr.u16.high.
         */
        return;
    }

    /* Instruction counter expired.  */
    assert(icount_enabled());
#ifndef CONFIG_USER_ONLY
    /* Ensure global icount has gone forward */
    icount_update(cpu);
    /* Refill decrementer and continue execution.  */
    insns_left = MIN(CF_COUNT_MASK, cpu->icount_budget);
    cpu_neg(cpu)->icount_decr.u16.low = insns_left;
    cpu->icount_extra = cpu->icount_budget - insns_left;

    /*
     * If the next tb has more instructions than we have left to
     * execute we need to ensure we find/generate a TB with exactly
     * insns_left instructions in it.
     */
    if (!cpu->icount_extra && insns_left > 0 && insns_left < tb->icount)  {
        cpu->cflags_next_tb = (tb->cflags & ~CF_COUNT_MASK) | insns_left;
    }
#endif
}

/* main execution loop */
int cpu_exec(CPUState *cpu)
{
    CPUClass *cc = CPU_GET_CLASS(cpu);
    int ret;
    SyncClocks sc = { 0 };

    /* replay_interrupt may need current_cpu */
    current_cpu = cpu;

    if (cpu_handle_halt(cpu)) {
        return EXCP_HALTED;
    }

    rcu_read_lock();

    cpu_exec_enter(cpu);

    /* Calculate difference between guest clock and host clock.
     * This delay includes the delay of the last cycle, so
     * what we have to do is sleep until it is 0. As for the
     * advance/delay we gain here, we try to fix it next time.
     */
    init_delay_params(&sc, cpu);

    /* prepare setjmp context for exception handling */
    if (sigsetjmp(cpu->jmp_env, 0) != 0) {
#if defined(__clang__)
        /*
         * Some compilers wrongly smash all local variables after
         * siglongjmp (the spec requires that only non-volatile locals
         * which are changed between the sigsetjmp and siglongjmp are
         * permitted to be trashed). There were bug reports for gcc
         * 4.5.0 and clang.  The bug is fixed in all versions of gcc
         * that we support, but is still unfixed in clang:
         *   https://bugs.llvm.org/show_bug.cgi?id=21183
         *
         * Reload essential local variables here for those compilers.
         * Newer versions of gcc would complain about this code (-Wclobbered),
         * so we only perform the workaround for clang.
         */
        cpu = current_cpu;
        cc = CPU_GET_CLASS(cpu);
#else
        /*
         * Non-buggy compilers preserve these locals; assert that
         * they have the correct value.
         */
        g_assert(cpu == current_cpu);
        g_assert(cc == CPU_GET_CLASS(cpu));
#endif

#ifndef CONFIG_SOFTMMU
        tcg_debug_assert(!have_mmap_lock());
#endif
        if (qemu_mutex_iothread_locked()) {
            qemu_mutex_unlock_iothread();
        }
        qemu_plugin_disable_mem_helpers(cpu);

        assert_no_pages_locked();
    }

    /* if an exception is pending, we execute it here */
    while (!cpu_handle_exception(cpu, &ret)) {
        TranslationBlock *last_tb = NULL;
        int tb_exit = 0;

        while (!cpu_handle_interrupt(cpu, &last_tb)) {
            uint32_t cflags = cpu->cflags_next_tb;
            TranslationBlock *tb;

            /* When requested, use an exact setting for cflags for the next
               execution.  This is used for icount, precise smc, and stop-
               after-access watchpoints.  Since this request should never
               have CF_INVALID set, -1 is a convenient invalid value that
               does not require tcg headers for cpu_common_reset.  */
            if (cflags == -1) {
                cflags = curr_cflags(cpu);
            } else {
                cpu->cflags_next_tb = -1;
            }

            tb = tb_find(cpu, last_tb, tb_exit, cflags);
#ifdef CONFIG_LATX_DEBUG
            trace_tb_execution(tb);
#endif
#ifdef CONFIG_LATX_PROFILER
            ADD_TB_PROFILE(tb, exit_times, 1);
#endif

            cpu_loop_exec_tb(cpu, tb, &last_tb, &tb_exit);
            /* Try to align the host and virtual clocks
               if the guest is in advance */
            align_clocks(&sc, cpu);
        }
    }

    cpu_exec_exit(cpu);
    rcu_read_unlock();

    return ret;
}

void tcg_exec_realizefn(CPUState *cpu, Error **errp)
{
    static bool tcg_target_initialized;
    CPUClass *cc = CPU_GET_CLASS(cpu);

    if (!tcg_target_initialized) {
        cc->tcg_ops->initialize();
        tcg_target_initialized = true;
    }
    tlb_init(cpu);
    qemu_plugin_vcpu_init_hook(cpu);

#ifndef CONFIG_USER_ONLY
    tcg_iommu_init_notifier_list(cpu);
#endif /* !CONFIG_USER_ONLY */
}

/* undo the initializations in reverse order */
void tcg_exec_unrealizefn(CPUState *cpu)
{
#ifndef CONFIG_USER_ONLY
    tcg_iommu_free_notifier_list(cpu);
#endif /* !CONFIG_USER_ONLY */

    qemu_plugin_vcpu_exit_hook(cpu);
    tlb_destroy(cpu);
}

#ifndef CONFIG_USER_ONLY

void dump_drift_info(void)
{
    if (!icount_enabled()) {
        return;
    }

    qemu_printf("Host - Guest clock  %"PRIi64" ms\n",
                (cpu_get_clock() - icount_get()) / SCALE_MS);
    if (icount_align_option) {
        qemu_printf("Max guest delay     %"PRIi64" ms\n",
                    -max_delay / SCALE_MS);
        qemu_printf("Max guest advance   %"PRIi64" ms\n",
                    max_advance / SCALE_MS);
    } else {
        qemu_printf("Max guest delay     NA\n");
        qemu_printf("Max guest advance   NA\n");
    }
}

#endif /* !CONFIG_USER_ONLY */
