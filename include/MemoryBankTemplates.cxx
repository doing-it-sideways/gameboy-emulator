namespace gb {

template <typename Self>
//auto&& MemoryBank::operator[](this Self&& self, u8 bank, u16 addr) {
auto&& MemoryBank::Access(this Self&& self, u8 bank, u16 addr) {
	if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
		const byte val = const_cast<MemoryBank&>(self)[bank, addr];
		return val;
	}
	else {
		switch (self._type) {
		case MemType::ROM: return self.GetRom(bank, addr);
		case MemType::RAM: return self.GetRam(bank, addr);
		default: std::unreachable();
		}
	}
}

} // namespace gb
