#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbcb36fe4, "hugetlb_optimize_vmemmap_key" },
	{ 0x587f22d7, "devmap_managed_key" },
	{ 0x83645119, "__kasan_check_write" },
	{ 0x8e9826ec, "__kasan_check_read" },
	{ 0x5b1f92bb, "mm_trace_folio_put" },
	{ 0x4ac8cfdc, "__put_devmap_managed_folio_refs" },
	{ 0x56c618c1, "__folio_put" },
	{ 0xa2584752, "__asan_report_load4_noabort" },
	{ 0x95534a9f, "__asan_report_load8_noabort" },
	{ 0xeda38c4d, "const_pcpu_hot" },
	{ 0xf6859a7b, "mem_cgroup_from_task" },
	{ 0xf352023f, "memory_cgrp_subsys_enabled_key" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xda43159e, "get_user_pages_fast" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xb91d9784, "__tracepoint_mmap_lock_start_locking" },
	{ 0x60b83cd5, "down_read" },
	{ 0xbd6dbb28, "__tracepoint_mmap_lock_acquire_returned" },
	{ 0x3d2571aa, "find_vma" },
	{ 0xdc96d91a, "_raw_spin_lock_irqsave" },
	{ 0xc7992b1b, "_raw_spin_unlock_irqrestore" },
	{ 0x322aa054, "__tracepoint_mmap_lock_released" },
	{ 0xacfea56c, "up_read" },
	{ 0xf530a627, "get_avail_swap_info_count" },
	{ 0x6c11da18, "__mmap_lock_do_trace_start_locking" },
	{ 0x51310ca8, "__mmap_lock_do_trace_acquire_returned" },
	{ 0xc6f10128, "__mmap_lock_do_trace_released" },
	{ 0x56000d95, "debug_locks" },
	{ 0xe320a9d2, "rcu_read_lock_held" },
	{ 0x4ca1d16a, "cgroup_mutex" },
	{ 0xe34272c9, "lock_is_held_type" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x98ec031b, "__asan_unregister_globals" },
	{ 0xf5869226, "__asan_register_globals" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x79a6876b, "misc_register" },
	{ 0x122c3a7e, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x47b19c2e, "misc_deregister" },
	{ 0x2a1625a7, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "69839B91A619BF79AF9EB46");
