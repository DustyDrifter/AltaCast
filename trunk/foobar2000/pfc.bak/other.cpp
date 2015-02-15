#include "pfc.h"

#include <intrin.h>


void order_helper::g_swap(t_size * data,t_size ptr1,t_size ptr2)
{
	t_size temp = data[ptr1];
	data[ptr1] = data[ptr2];
	data[ptr2] = temp;
}


t_size order_helper::g_find_reverse(const t_size * order,t_size val)
{
	t_size prev = val, next = order[val];
	while(next != val)
	{
		prev = next;
		next = order[next];
	}
	return prev;
}


void order_helper::g_reverse(t_size * order,t_size base,t_size count)
{
	t_size max = count>>1;
	t_size n;
	t_size base2 = base+count-1;
	for(n=0;n<max;n++)
		g_swap(order,base+n,base2-n);
}


void pfc::crash() {
#if 0 //def _MSC_VER
	__debugbreak();
#else
	*(char*)NULL = 0;
#endif
}


void pfc::byteswap_raw(void * p_buffer,const t_size p_bytes) {
	t_uint8 * ptr = (t_uint8*)p_buffer;
	t_size n;
	for(n=0;n<p_bytes>>1;n++) swap_t(ptr[n],ptr[p_bytes-n-1]);
}

#if defined(_DEBUG) && defined(_WIN32)
void pfc::myassert(const wchar_t * _Message, const wchar_t *_File, unsigned _Line) { 
	if (IsDebuggerPresent()) pfc::crash();
	_wassert(_Message,_File,_Line);
}
#endif