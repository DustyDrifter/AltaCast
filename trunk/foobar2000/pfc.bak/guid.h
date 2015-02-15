#ifndef _PFC_GUID_H_
#define _PFC_GUID_H_

namespace pfc {

	class GUID_from_text : public GUID
	{
		unsigned read_hex(char c);
		unsigned read_byte(const char * ptr);
		unsigned read_word(const char * ptr);
		unsigned read_dword(const char * ptr);
		void read_bytes(unsigned char * out,unsigned num,const char * ptr);

	public:
		GUID_from_text(const char * text);
	};

	class print_guid
	{
	public:
		print_guid(const GUID & p_guid);
		inline operator const char * () const {return m_data;}
		inline const char * get_ptr() {return m_data;}
	private:
		char m_data[64];
	};

	inline int guid_compare(const GUID & g1,const GUID & g2) {return memcmp(&g1,&g2,sizeof(GUID));}

	inline bool guid_equal(const GUID & g1,const GUID & g2) {return (g1 == g2) ? true : false;}
	template<> inline int compare_t<GUID>(const GUID & p_item1,const GUID & p_item2) {return guid_compare(p_item1,p_item2);}

	extern const GUID guid_null;

	PFC_DLL_EXPORT void print_hex_raw(const void * buffer,unsigned bytes,char * p_out);
}


#endif
