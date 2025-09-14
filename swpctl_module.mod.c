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
	{ 0x25b8b688, "misc_deregister" },
	{ 0xbcb36fe4, "hugetlb_optimize_vmemmap_key" },
	{ 0x587f22d7, "devmap_managed_key" },
	{ 0xd40bdb4e, "mm_trace_folio_put" },
	{ 0x542c6dd1, "__put_devmap_managed_folio_refs" },
	{ 0xe0ba34f0, "__folio_put" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x973b60d3, "const_pcpu_hot" },
	{ 0x7873cb50, "find_vma" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xb5b54b34, "_raw_spin_unlock" },
	{ 0x8427cc7b, "_raw_spin_lock_irq" },
	{ 0x4b750f53, "_raw_spin_unlock_irq" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x70e0e506, "get_user_pages_fast" },
	{ 0xf530a627, "get_avail_swap_info_count" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xf986b46e, "misc_register" },
	{ 0x122c3a7e, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x01e8636b, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "F767203069F636372738261");
