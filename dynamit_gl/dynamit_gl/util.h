#pragma once

template<typename IntegerType> inline bool checkAny(IntegerType check, IntegerType mask) { return check & mask; }
template<typename IntegerType> inline bool checkAll(IntegerType check, IntegerType mask) { return (check & mask) == mask; }

//Scope guard for RAII
template<typename Deleter> class scope_guard
{
	Deleter t;
public:
	scope_guard() {}
	scope_guard(Deleter _t) :t(_t) {}
	~scope_guard() { t(); }
};
