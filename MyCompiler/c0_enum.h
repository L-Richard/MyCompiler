#pragma once

enum ObjectiveType {
	noty,
	constty,	// defined const, with a name
	varty,
	functy,
	arrayty,

	tmp,		// tmp, optimizing: dag, mem or reg
	label,
	paras,		// use when send parameters, a set of several paras
};
enum kind {
	// used in midcode, and optimizing
	mem_kd,		// store in mem
	reg_kd,		// allocate register
	const_kd,	// compute when complie
	para_kd,	// use $a0-4 registers
	return_kd,	// use $v0 store the var or tmp
};
enum labelType {
	notlb,
	main_lb,
	end_main_lb,
	fun_lb,
	end_fun_lb,
	while_lb,
	end_while_lb,
	end_if_lb,
	end_case_lb,
	end_switch_lb,
};

enum Type {
	notp,
	chartp,
	inttp,
	voidtp,		// only function
	stringtp,	// only print "some string"
	
};
