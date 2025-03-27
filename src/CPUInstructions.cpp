#include <eternal.hpp>
#include <functional>

#include "CPU.hpp"

namespace gb::cpu {

using InstrFunc = void(*)(Context&);
#define INSTRFUNC static constexpr void

INSTRFUNC nop(Context& cpu) {
	
}

INSTRFUNC ld_r8_r8(Context& cpu) {
	
}

#undef INSTRFUNC
#define INSTR(x) { OpCode::##x, &x }

static constexpr auto instrs = mapbox::eternal::map<OpCode, InstrFunc>(
{
	INSTR(ld_r8_r8)
});

//static constexpr auto cbInstrs = mapbox::eternal::map<OpCode, InstrFunc>(
//{
//
//});

#undef INSTR

constexpr void Context::Exec() {
	// TODO
}

} // namespace gb::cpu
