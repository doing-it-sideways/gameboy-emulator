#ifndef BITFIELD_UNION_BYTE
#define BITFIELD_UNION_BYTE(StructName, BitfieldName, Bitfield) \
	struct StructName { \
		union { \
			struct { \
				Bitfield \
			} BitfieldName; \
			byte asByte; \
		}; \
		constexpr operator byte() const { return asByte; } \
		StructName##() : asByte() {} \
		constexpr StructName##(byte b) : asByte(b) {} \
		constexpr StructName##& operator=(byte b) { asByte = b; return *this; } \
	}
#endif // BITFIELD_UNION_BYTE
