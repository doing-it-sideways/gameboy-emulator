namespace gb {

template <typename Self>
auto Memory::operator[](this Self&& self, u16 addr) {
	if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
		const byte val = const_cast<Memory&>(self).Read(addr);
		return val;
	}
	else
		return Accessor{ self, addr };
}

} // namespace gb

